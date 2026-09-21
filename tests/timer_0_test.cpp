#include <co/timer_0.h>
#include <co/timer_0.hpp>

#include <gtest/gtest.h>

#include <cstdint>
#include <optional>

namespace
{

// Clock with a fixed calendar origin (or none); only get_calendar_origin_0 is used by the conversion.
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

// Timer that records the expiry set through expiry_at_0, which returns expiry_at_result.
struct fake_timer final : co_timer_0_t
{
	co_clock_0 clock;
	co_result_0 expiry_at_result;
	std::optional<std::int64_t> expiry;
};

void fake_timer_get_clock_0(co_timer_0 self, co_clock_0* clock) noexcept
{
	*clock = static_cast<fake_timer*>(self)->clock;
}

co_result_0 fake_timer_expiry_at_0(co_timer_0 self, int64_t time_ns) noexcept
{
	fake_timer* timer = static_cast<fake_timer*>(self);
	if (timer->expiry_at_result == CO_RESULT_0_SUCCESS) {
		timer->expiry = time_ns;
	}
	return timer->expiry_at_result;
}

const co_timer_0_vtable fake_timer_vtable = {
	.api_version = CO_API_VERSION_0,
	.destroy = nullptr,
	.get_clock_0 = &fake_timer_get_clock_0,
	.expiry_at_0 = &fake_timer_expiry_at_0,
	.expiry_after_0 = nullptr,
	.wait_0 = nullptr,
	.cancel_0 = nullptr,
};

struct fixture
{
	fake_clock clock{};
	fake_timer timer{};

	explicit fixture(co_calendar_time_0 origin)
	{
		co_clock_0_init(&clock, &fake_clock_vtable);
		clock.has_calendar_origin = true;
		clock.origin = origin;
		co_timer_0_init(&timer, &fake_timer_vtable);
		timer.clock = &clock;
		timer.expiry_at_result = CO_RESULT_0_SUCCESS;
	}
};

constexpr std::int64_t second_ns = 1'000'000'000;
constexpr std::int64_t day_ns = 86'400 * second_ns;

} // namespace

TEST(c_timer_0, expiry_at_calendar_0_sets_expiry_on_clock_of_timer)
{
	fixture f({2025, 1, 1, 0, 0, 0, 0});
	const co_calendar_time_0 at = {2025, 1, 2, 0, 0, 1, 0};
	ASSERT_EQ(co_timer_0_expiry_at_calendar_0(&f.timer, &at), CO_RESULT_0_SUCCESS);
	EXPECT_EQ(f.timer.expiry, day_ns + second_ns);
}

TEST(c_timer_0, expiry_at_calendar_0_is_not_supported_without_calendar_origin)
{
	fixture f({2025, 1, 1, 0, 0, 0, 0});
	f.clock.has_calendar_origin = false;
	const co_calendar_time_0 at = {2025, 1, 2, 0, 0, 0, 0};
	EXPECT_EQ(co_timer_0_expiry_at_calendar_0(&f.timer, &at), CO_RESULT_0_ERROR_NOT_SUPPORTED);
	EXPECT_FALSE(f.timer.expiry.has_value());
}

TEST(c_timer_0, expiry_at_calendar_0_rejects_invalid_calendar_time)
{
	fixture f({2025, 1, 1, 0, 0, 0, 0});
	const co_calendar_time_0 at = {2025, 2, 29, 0, 0, 0, 0};
	EXPECT_EQ(co_timer_0_expiry_at_calendar_0(&f.timer, &at), CO_RESULT_0_ERROR_INVALID_ARGUMENT);
	EXPECT_FALSE(f.timer.expiry.has_value());
}

TEST(c_timer_0, expiry_at_calendar_0_reports_out_of_range_instead_of_clamping)
{
	fixture f({2025, 1, 1, 0, 0, 0, 0});
	const co_calendar_time_0 far_future = {2400, 1, 1, 0, 0, 0, 0};
	EXPECT_EQ(co_timer_0_expiry_at_calendar_0(&f.timer, &far_future), CO_RESULT_0_ERROR_OUT_OF_RANGE);
	const co_calendar_time_0 far_past = {1700, 1, 1, 0, 0, 0, 0};
	EXPECT_EQ(co_timer_0_expiry_at_calendar_0(&f.timer, &far_past), CO_RESULT_0_ERROR_OUT_OF_RANGE);
	EXPECT_FALSE(f.timer.expiry.has_value());
}

TEST(c_timer_0, expiry_at_calendar_0_returns_error_of_expiry_at_0)
{
	fixture f({2025, 1, 1, 0, 0, 0, 0});
	f.timer.expiry_at_result = CO_RESULT_0_ERROR_OUT_OF_RANGE;
	const co_calendar_time_0 at = {2025, 1, 2, 0, 0, 0, 0};
	EXPECT_EQ(co_timer_0_expiry_at_calendar_0(&f.timer, &at), CO_RESULT_0_ERROR_OUT_OF_RANGE);
	EXPECT_FALSE(f.timer.expiry.has_value());
}

TEST(cpp_timer_0, expiry_at_calendar_0_forwards)
{
	fixture f({2000, 1, 1, 0, 0, 0, 0});
	co::timer_0 timer(&f.timer);
	const co::calendar_time_0 at = {.year = 2000, .month = 1, .day = 1, .hour = 0, .minute = 0, .second = 2, .nanosecond = 0};
	ASSERT_EQ(timer.expiry_at_calendar_0(at), co::result_0::success);
	EXPECT_EQ(f.timer.expiry, 2 * second_ns);
}
