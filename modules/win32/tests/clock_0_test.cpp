#include <co/win32/clock_0.hpp>

#include <gtest/gtest.h>

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <new>

namespace
{

struct counting_allocator final
{
	std::size_t allocations = 0;
	std::size_t deallocations = 0;

	static void* allocate(void* data, std::size_t size, std::size_t alignment) noexcept
	{
		++static_cast<counting_allocator*>(data)->allocations;
		return ::operator new(size, std::align_val_t(alignment), std::nothrow);
	}

	static void deallocate(void* data, void* memory, std::size_t size, std::size_t alignment) noexcept
	{
		++static_cast<counting_allocator*>(data)->deallocations;
		::operator delete(memory, size, std::align_val_t(alignment));
	}
};

co_win32_clock_0 create_clock(bool include_suspend_time, const co_in_structure_0* next_structure = nullptr)
{
	const co_win32_clock_0_create_info_0 create_info = {
		.structure_type = CO_STRUCTURE_TYPE_0_WIN32_CLOCK_0_CREATE_INFO_0,
		.next_structure = next_structure,
		.include_suspend_time = include_suspend_time,
	};
	co_win32_clock_0 clock = nullptr;
	EXPECT_EQ(co_win32_clock_0_create_0(&create_info, &clock), CO_RESULT_0_SUCCESS);
	return clock;
}

std::int64_t system_time_ns()
{
	return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

// Nanoseconds since the Unix epoch of a calendar time.
std::int64_t unix_ns(const co_calendar_time_0& calendar_time)
{
	const std::chrono::sys_days date = std::chrono::year{calendar_time.year} / std::chrono::month{calendar_time.month} / std::chrono::day{calendar_time.day};
	const auto time = date + std::chrono::hours{calendar_time.hour} + std::chrono::minutes{calendar_time.minute} + std::chrono::seconds{calendar_time.second} + std::chrono::nanoseconds{calendar_time.nanosecond};
	return std::chrono::duration_cast<std::chrono::nanoseconds>(time.time_since_epoch()).count();
}

} // namespace

class win32_clock_0_test : public testing::TestWithParam<bool>
{
};

INSTANTIATE_TEST_SUITE_P(include_suspend_time, win32_clock_0_test, testing::Bool());

TEST_P(win32_clock_0_test, calendar_origin_is_creation)
{
	const std::int64_t before = system_time_ns();
	co_win32_clock_0 clock = create_clock(GetParam());
	const std::int64_t after = system_time_ns();
	ASSERT_NE(clock, nullptr);
	co_calendar_time_0 origin = {};
	ASSERT_EQ(co_clock_0_get_calendar_origin_0(CO_CLOCK_0_CAST(clock), &origin), CO_RESULT_0_SUCCESS);
	// std::chrono::system_clock reads the same source with 100 ns resolution.
	EXPECT_GE(unix_ns(origin), before - 100);
	EXPECT_LE(unix_ns(origin), after + 100);
	EXPECT_EQ(origin.nanosecond % 100, 0u);
	std::int64_t time = -1;
	ASSERT_EQ(co_clock_0_time_from_calendar_0(CO_CLOCK_0_CAST(clock), &origin, &time), CO_RESULT_0_SUCCESS);
	EXPECT_EQ(time, 0);
	co_win32_clock_0_destroy(clock);
}

TEST_P(win32_clock_0_test, time_follows_system_time)
{
	co_win32_clock_0 clock = create_clock(GetParam());
	ASSERT_NE(clock, nullptr);
	co_calendar_time_0 origin = {};
	ASSERT_EQ(co_clock_0_get_calendar_origin_0(CO_CLOCK_0_CAST(clock), &origin), CO_RESULT_0_SUCCESS);
	const std::int64_t before = system_time_ns();
	std::int64_t time = 0;
	co_clock_0_get_time_0(CO_CLOCK_0_CAST(clock), &time);
	const std::int64_t after = system_time_ns();
	EXPECT_GE(time, 0);
	EXPECT_GE(unix_ns(origin) + time, before - 100);
	EXPECT_LE(unix_ns(origin) + time, after + 100);
	co_calendar_time_0 calendar_time = {};
	ASSERT_EQ(co_clock_0_time_to_calendar_0(CO_CLOCK_0_CAST(clock), time, &calendar_time), CO_RESULT_0_SUCCESS);
	EXPECT_EQ(unix_ns(calendar_time), unix_ns(origin) + time);
	co_win32_clock_0_destroy(clock);
}

TEST_P(win32_clock_0_test, base_time_never_decreases)
{
	co_win32_clock_0 clock = create_clock(GetParam());
	ASSERT_NE(clock, nullptr);
	std::uint64_t previous = 0;
	co_clock_0_get_base_time_0(CO_CLOCK_0_CAST(clock), &previous);
	for (int i = 0; i < 1000; ++i) {
		std::uint64_t base_time = 0;
		co_clock_0_get_base_time_0(CO_CLOCK_0_CAST(clock), &base_time);
		EXPECT_GE(base_time, previous);
		previous = base_time;
	}
	co_win32_clock_0_destroy(clock);
}

TEST_P(win32_clock_0_test, mapping_matches_time)
{
	co_win32_clock_0 clock = create_clock(GetParam());
	ASSERT_NE(clock, nullptr);
	std::uint64_t base_before = 0;
	co_clock_0_get_base_time_0(CO_CLOCK_0_CAST(clock), &base_before);
	co_clock_0_mapping_0 mapping = {};
	std::size_t generation = 1;
	co_clock_0_get_mapping_0(CO_CLOCK_0_CAST(clock), &mapping, &generation);
	std::uint64_t base_after = 0;
	co_clock_0_get_base_time_0(CO_CLOCK_0_CAST(clock), &base_after);
	EXPECT_EQ(generation, 0u);
	EXPECT_EQ(mapping.rate_numerator, 1u);
	EXPECT_EQ(mapping.rate_denominator, 1u);
	EXPECT_GE(mapping.base_time, base_before);
	EXPECT_LE(mapping.base_time, base_after);
	std::int64_t time = 0;
	co_clock_0_get_time_0(CO_CLOCK_0_CAST(clock), &time);
	std::uint64_t base_time = 0;
	co_clock_0_get_base_time_0(CO_CLOCK_0_CAST(clock), &base_time);
	// Unless the system time was set meanwhile, the mapping predicts the time within a millisecond.
	const std::int64_t predicted = mapping.time + static_cast<std::int64_t>(base_time - mapping.base_time);
	EXPECT_NEAR(static_cast<double>(time), static_cast<double>(predicted), 1e6);
	co_win32_clock_0_destroy(clock);
}

TEST_P(win32_clock_0_test, wait_change_is_not_supported_and_cancel_succeeds)
{
	co_win32_clock_0 clock = create_clock(GetParam());
	ASSERT_NE(clock, nullptr);
	co_executor_0 executor = reinterpret_cast<co_executor_0>(static_cast<std::uintptr_t>(1));
	co_clock_0_wait_change_0_completion_fn completion_fn = [](void*, co_result_0, std::size_t) noexcept {};
	EXPECT_EQ(co_clock_0_wait_change_0(CO_CLOCK_0_CAST(clock), 0, nullptr, executor, completion_fn, nullptr), CO_RESULT_0_ERROR_NOT_SUPPORTED);
	EXPECT_EQ(co_clock_0_cancel_0(CO_CLOCK_0_CAST(clock)), CO_RESULT_0_SUCCESS);
	co_win32_clock_0_destroy(clock);
}

TEST(win32_clock_0, uses_allocation_callbacks)
{
	counting_allocator allocator;
	const co_allocation_callbacks_0 allocation_callbacks = {
		.structure_type = CO_STRUCTURE_TYPE_0_ALLOCATION_CALLBACKS_0,
		.next_structure = nullptr,
		.allocate_fn = &counting_allocator::allocate,
		.deallocate_fn = &counting_allocator::deallocate,
		.data = &allocator,
	};
	co_win32_clock_0 clock = create_clock(true, CO_IN_STRUCTURE_0_CAST(&allocation_callbacks));
	ASSERT_NE(clock, nullptr);
	EXPECT_EQ(allocator.allocations, 1u);
	EXPECT_EQ(allocator.deallocations, 0u);
	co_win32_clock_0_destroy(clock);
	EXPECT_EQ(allocator.deallocations, 1u);
}

TEST(win32_clock_0, rejects_unsupported_next_chain_entry)
{
	const co_win32_clock_0_create_info_0 other = {
		.structure_type = CO_STRUCTURE_TYPE_0_WIN32_CLOCK_0_CREATE_INFO_0,
		.next_structure = nullptr,
		.include_suspend_time = false,
	};
	const co_win32_clock_0_create_info_0 create_info = {
		.structure_type = CO_STRUCTURE_TYPE_0_WIN32_CLOCK_0_CREATE_INFO_0,
		.next_structure = CO_IN_STRUCTURE_0_CAST(&other),
		.include_suspend_time = false,
	};
	co_win32_clock_0 clock = nullptr;
	EXPECT_EQ(co_win32_clock_0_create_0(&create_info, &clock), CO_RESULT_0_ERROR_NOT_SUPPORTED);
	EXPECT_EQ(clock, nullptr);
}

TEST(win32_clock_0, cpp_wrapper)
{
	const co::win32::clock_0_create_info_0 create_info = {
		.structure_type = co::structure_type_0_win32_clock_0_create_info_0,
		.next_structure = nullptr,
		.include_suspend_time = false,
	};
	co::win32::clock_0 clock;
	ASSERT_EQ(co::win32::clock_0_create_0(create_info, &clock), co::result_0::success);
	ASSERT_TRUE(clock);
	co::clock_0 generic = clock;
	co::calendar_time_0 origin = {};
	EXPECT_EQ(generic.get_calendar_origin_0(&origin), co::result_0::success);
	EXPECT_GE(origin.year, 2024);
	clock.destroy();
}
