#include <co/clock_0.h>
#include <co/clock_0.hpp>

#include <gtest/gtest.h>

#include <cstdint>
#include <limits>

namespace
{

// Clock with a fixed calendar origin (or none); only get_calendar_origin_0 is used by the conversions.
struct fake_clock final : co_clock_0_t
{
	bool has_calendar_origin;
	co_calendar_time_0 origin;
};

co_result_0 fake_clock_get_calendar_origin_0(co_clock_0 self, co_calendar_time_0* origin) noexcept
{
	fake_clock* clock = static_cast<fake_clock*>(self);
	if (!clock->has_calendar_origin) {
		return CO_RESULT_0_ERROR_NOT_SUPPORTED;
	}
	*origin = clock->origin;
	return CO_RESULT_0_SUCCESS;
}

const co_clock_0_vtable fake_clock_vtable = {
	.api_version = CO_API_VERSION_0,
	.destroy = nullptr,
	.get_calendar_origin_0 = &fake_clock_get_calendar_origin_0,
	.get_time_0 = nullptr,
	.get_base_time_0 = nullptr,
	.get_mapping_0 = nullptr,
	.wait_change_0 = nullptr,
	.cancel_0 = nullptr,
};

fake_clock make_clock(co_calendar_time_0 origin)
{
	fake_clock clock{};
	co_clock_0_init(&clock, &fake_clock_vtable);
	clock.has_calendar_origin = true;
	clock.origin = origin;
	return clock;
}

constexpr std::int64_t second_ns = 1'000'000'000;
constexpr std::int64_t day_ns = 86'400 * second_ns;

} // namespace

TEST(c_clock_0, time_from_calendar_0_is_relative_to_origin)
{
	fake_clock clock = make_clock({2025, 1, 1, 0, 0, 0, 0});
	const co_calendar_time_0 at = {2025, 1, 2, 0, 0, 1, 500};
	int64_t time = 0;
	ASSERT_EQ(co_clock_0_time_from_calendar_0(&clock, &at, &time), CO_RESULT_0_SUCCESS);
	EXPECT_EQ(time, day_ns + second_ns + 500);
}

TEST(c_clock_0, time_from_calendar_0_before_origin_is_negative)
{
	fake_clock clock = make_clock({2025, 1, 1, 0, 0, 0, 250});
	const co_calendar_time_0 at = {2024, 12, 31, 23, 59, 59, 0};
	int64_t time = 0;
	ASSERT_EQ(co_clock_0_time_from_calendar_0(&clock, &at, &time), CO_RESULT_0_SUCCESS);
	EXPECT_EQ(time, -second_ns - 250);
}

TEST(c_clock_0, time_from_calendar_0_handles_leap_days)
{
	fake_clock clock = make_clock({2024, 2, 28, 0, 0, 0, 0});
	const co_calendar_time_0 at = {2024, 3, 1, 0, 0, 0, 0};
	int64_t time = 0;
	ASSERT_EQ(co_clock_0_time_from_calendar_0(&clock, &at, &time), CO_RESULT_0_SUCCESS);
	EXPECT_EQ(time, 2 * day_ns);
}

TEST(c_clock_0, time_from_calendar_0_rejects_invalid_calendar_time)
{
	fake_clock clock = make_clock({2025, 1, 1, 0, 0, 0, 0});
	int64_t time = 42;
	const co_calendar_time_0 not_leap = {2025, 2, 29, 0, 0, 0, 0};
	EXPECT_EQ(co_clock_0_time_from_calendar_0(&clock, &not_leap, &time), CO_RESULT_0_ERROR_INVALID_ARGUMENT);
	const co_calendar_time_0 leap_second = {2025, 6, 30, 23, 59, 60, 0};
	EXPECT_EQ(co_clock_0_time_from_calendar_0(&clock, &leap_second, &time), CO_RESULT_0_ERROR_INVALID_ARGUMENT);
	const co_calendar_time_0 nanosecond = {2025, 1, 1, 0, 0, 0, 1'000'000'000};
	EXPECT_EQ(co_clock_0_time_from_calendar_0(&clock, &nanosecond, &time), CO_RESULT_0_ERROR_INVALID_ARGUMENT);
	EXPECT_EQ(time, 42);
}

TEST(c_clock_0, time_from_calendar_0_reports_out_of_range)
{
	fake_clock clock = make_clock({2025, 1, 1, 0, 0, 0, 0});
	int64_t time = 42;
	const co_calendar_time_0 far_future = {2400, 1, 1, 0, 0, 0, 0};
	EXPECT_EQ(co_clock_0_time_from_calendar_0(&clock, &far_future, &time), CO_RESULT_0_ERROR_OUT_OF_RANGE);
	const co_calendar_time_0 far_past = {1700, 1, 1, 0, 0, 0, 0};
	EXPECT_EQ(co_clock_0_time_from_calendar_0(&clock, &far_past, &time), CO_RESULT_0_ERROR_OUT_OF_RANGE);
	EXPECT_EQ(time, 42);
}

TEST(c_clock_0, time_to_calendar_0_round_trips)
{
	fake_clock clock = make_clock({2025, 6, 1, 12, 30, 15, 999'999'999});
	const std::int64_t times[] = {
		0,
		1,
		-1,
		day_ns * 365 + 123,
		-day_ns * 400 - 7,
		std::numeric_limits<std::int64_t>::max(),
		std::numeric_limits<std::int64_t>::min(),
	};
	for (std::int64_t time : times) {
		co_calendar_time_0 calendar_time{};
		ASSERT_EQ(co_clock_0_time_to_calendar_0(&clock, time, &calendar_time), CO_RESULT_0_SUCCESS) << time;
		int64_t back = 0;
		ASSERT_EQ(co_clock_0_time_from_calendar_0(&clock, &calendar_time, &back), CO_RESULT_0_SUCCESS) << time;
		EXPECT_EQ(back, time);
	}
}

TEST(c_clock_0, time_to_calendar_0_carries_into_next_day)
{
	fake_clock clock = make_clock({2024, 12, 31, 23, 59, 59, 999'999'999});
	co_calendar_time_0 calendar_time{};
	ASSERT_EQ(co_clock_0_time_to_calendar_0(&clock, 1, &calendar_time), CO_RESULT_0_SUCCESS);
	EXPECT_EQ(calendar_time.year, 2025);
	EXPECT_EQ(calendar_time.month, 1);
	EXPECT_EQ(calendar_time.day, 1);
	EXPECT_EQ(calendar_time.hour, 0);
	EXPECT_EQ(calendar_time.minute, 0);
	EXPECT_EQ(calendar_time.second, 0);
	EXPECT_EQ(calendar_time.nanosecond, 0u);
}

TEST(c_clock_0, conversions_are_not_supported_without_calendar_origin)
{
	fake_clock clock = make_clock({2025, 1, 1, 0, 0, 0, 0});
	clock.has_calendar_origin = false;
	co_calendar_time_0 origin{};
	EXPECT_EQ(co_clock_0_get_calendar_origin_0(&clock, &origin), CO_RESULT_0_ERROR_NOT_SUPPORTED);
	int64_t time = 0;
	const co_calendar_time_0 at = {2025, 1, 1, 0, 0, 0, 0};
	EXPECT_EQ(co_clock_0_time_from_calendar_0(&clock, &at, &time), CO_RESULT_0_ERROR_NOT_SUPPORTED);
	co_calendar_time_0 calendar_time{};
	EXPECT_EQ(co_clock_0_time_to_calendar_0(&clock, 0, &calendar_time), CO_RESULT_0_ERROR_NOT_SUPPORTED);
}

TEST(cpp_clock_0, time_from_calendar_0_forwards)
{
	fake_clock clock = make_clock({2000, 1, 1, 0, 0, 0, 0});
	co::clock_0 handle(&clock);
	co::calendar_time_0 origin{};
	ASSERT_EQ(handle.get_calendar_origin_0(&origin), co::result_0::success);
	EXPECT_EQ(origin.year, 2000);
	std::int64_t time = 0;
	ASSERT_EQ(handle.time_from_calendar_0(co::calendar_time_0{.year = 2000, .month = 1, .day = 1, .hour = 0, .minute = 0, .second = 1, .nanosecond = 0}, &time), co::result_0::success);
	EXPECT_EQ(time, second_ns);
	co::calendar_time_0 calendar_time{};
	ASSERT_EQ(handle.time_to_calendar_0(-second_ns, &calendar_time), co::result_0::success);
	EXPECT_EQ(calendar_time.year, 1999);
	EXPECT_EQ(calendar_time.second, 59);
}
