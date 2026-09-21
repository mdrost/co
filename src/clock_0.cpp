#include "clock_0.hpp"

#include <cstddef>
#include <cstdint>
#include <limits>
#include <type_traits>

// Rule: Make sure functions in this file are defined in the same order as in clock_0.h file.

static_assert(std::is_standard_layout_v<co_clock_0_t>);
static_assert(offsetof(co_clock_0_vtable, api_version) == 0);

namespace
{

constexpr std::int64_t nanoseconds_per_second = 1'000'000'000;
constexpr std::int64_t seconds_per_day = 86'400;

constexpr std::int64_t floor_div(std::int64_t a, std::int64_t b) noexcept
{
	std::int64_t q = a / b;
	return (a % b != 0 && (a < 0) != (b < 0)) ? q - 1 : q;
}

constexpr std::int64_t floor_mod(std::int64_t a, std::int64_t b) noexcept
{
	std::int64_t r = a % b;
	return (r != 0 && (r < 0) != (b < 0)) ? r + b : r;
}

constexpr bool is_leap_year(std::int64_t year) noexcept
{
	return year % 4 == 0 && (year % 100 != 0 || year % 400 == 0);
}

constexpr unsigned days_in_month(std::int64_t year, unsigned month) noexcept
{
	constexpr unsigned days[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
	return month == 2 && is_leap_year(year) ? 29 : days[month - 1];
}

bool is_valid(const co_calendar_time_0& calendar_time) noexcept
{
	return calendar_time.month >= 1 && calendar_time.month <= 12
		&& calendar_time.day >= 1 && calendar_time.day <= days_in_month(calendar_time.year, calendar_time.month)
		&& calendar_time.hour < 24
		&& calendar_time.minute < 60
		&& calendar_time.second < 60
		&& calendar_time.nanosecond < nanoseconds_per_second;
}

// Days from 1970-01-01 of the proleptic Gregorian calendar (H. Hinnant's days_from_civil). The anchor is internal: only
// differences of the results are used.
constexpr std::int64_t days_from_civil(std::int64_t year, unsigned month, unsigned day) noexcept
{
	year -= month <= 2;
	const std::int64_t era = floor_div(year, 400);
	const std::int64_t year_of_era = year - era * 400;
	const std::int64_t day_of_year = (153 * (month > 2 ? month - 3 : month + 9) + 2) / 5 + day - 1;
	const std::int64_t day_of_era = year_of_era * 365 + year_of_era / 4 - year_of_era / 100 + day_of_year;
	return era * 146097 + day_of_era - 719468;
}

// Inverse of days_from_civil.
constexpr void civil_from_days(std::int64_t days, std::int64_t* year, unsigned* month, unsigned* day) noexcept
{
	days += 719468;
	const std::int64_t era = floor_div(days, 146097);
	const std::int64_t day_of_era = days - era * 146097;
	const std::int64_t year_of_era = (day_of_era - day_of_era / 1460 + day_of_era / 36524 - day_of_era / 146096) / 365;
	const std::int64_t day_of_year = day_of_era - (365 * year_of_era + year_of_era / 4 - year_of_era / 100);
	const std::int64_t month_from_march = (5 * day_of_year + 2) / 153;
	*day = static_cast<unsigned>(day_of_year - (153 * month_from_march + 2) / 5 + 1);
	*month = static_cast<unsigned>(month_from_march < 10 ? month_from_march + 3 : month_from_march - 9);
	*year = year_of_era + era * 400 + (*month <= 2);
}

static_assert(days_from_civil(1970, 1, 1) == 0);
static_assert(days_from_civil(2000, 3, 1) == 11017);

// Seconds of a valid calendar time from the internal anchor; fits in int64_t for every int32_t year.
constexpr std::int64_t seconds_from_civil(const co_calendar_time_0& calendar_time) noexcept
{
	return days_from_civil(calendar_time.year, calendar_time.month, calendar_time.day) * seconds_per_day + calendar_time.hour * 3600 + calendar_time.minute * 60 + calendar_time.second;
}

} // namespace

void co_clock_0_destroy(co_clock_0 clock) noexcept
{
	if (clock == nullptr) {
		return;
	}
	clock->vtable->destroy(clock);
}

co_result_0 co_clock_0_get_calendar_origin_0(co_clock_0 clock, co_calendar_time_0* origin) noexcept
{
	return clock->vtable->get_calendar_origin_0(clock, origin);
}

void co_clock_0_get_time_0(co_clock_0 clock, int64_t* time) noexcept
{
	clock->vtable->get_time_0(clock, time);
}

void co_clock_0_get_base_time_0(co_clock_0 clock, uint64_t* base_time) noexcept
{
	clock->vtable->get_base_time_0(clock, base_time);
}

void co_clock_0_get_mapping_0(co_clock_0 clock, co_clock_0_mapping_0* mapping, size_t* generation) noexcept
{
	clock->vtable->get_mapping_0(clock, mapping, generation);
}

co_result_0 co_clock_0_wait_change_0(co_clock_0 clock, size_t after_generation, co_cancellation_0 cancellation, co_executor_0 executor, co_clock_0_wait_change_0_completion_fn completion_fn, void* data) noexcept
{
	return clock->vtable->wait_change_0(clock, after_generation, cancellation, executor, completion_fn, data);
}

co_result_0 co_clock_0_cancel_0(co_clock_0 clock) noexcept
{
	return clock->vtable->cancel_0(clock);
}

co_result_0 co_clock_0_time_from_calendar_0(co_clock_0 clock, const co_calendar_time_0* calendar_time, int64_t* time) noexcept
{
	co_calendar_time_0 origin;
	const co_result_0 result = clock->vtable->get_calendar_origin_0(clock, &origin);
	if (result != CO_RESULT_0_SUCCESS) {
		return result;
	}
	if (!is_valid(*calendar_time)) {
		return CO_RESULT_0_ERROR_INVALID_ARGUMENT;
	}
	// time = seconds * 10^9 + nanoseconds, with 0 <= nanoseconds < 10^9.
	std::int64_t seconds = seconds_from_civil(*calendar_time) - seconds_from_civil(origin);
	std::int64_t nanoseconds = static_cast<std::int64_t>(calendar_time->nanosecond) - static_cast<std::int64_t>(origin.nanosecond);
	if (nanoseconds < 0) {
		nanoseconds += nanoseconds_per_second;
		seconds -= 1;
	}
	constexpr std::int64_t min = std::numeric_limits<std::int64_t>::min();
	constexpr std::int64_t max = std::numeric_limits<std::int64_t>::max();
	if (seconds >= 0) {
		if (seconds > (max - nanoseconds) / nanoseconds_per_second) {
			return CO_RESULT_0_ERROR_OUT_OF_RANGE;
		}
		*time = seconds * nanoseconds_per_second + nanoseconds;
	} else {
		// (seconds + 1) * 10^9 - (10^9 - nanoseconds), so that no intermediate value is below min.
		const std::int64_t remainder = nanoseconds_per_second - nanoseconds;
		if (seconds + 1 < (min + remainder) / nanoseconds_per_second) {
			return CO_RESULT_0_ERROR_OUT_OF_RANGE;
		}
		*time = (seconds + 1) * nanoseconds_per_second - remainder;
	}
	return CO_RESULT_0_SUCCESS;
}

co_result_0 co_clock_0_time_to_calendar_0(co_clock_0 clock, int64_t time, co_calendar_time_0* calendar_time) noexcept
{
	co_calendar_time_0 origin;
	const co_result_0 result = clock->vtable->get_calendar_origin_0(clock, &origin);
	if (result != CO_RESULT_0_SUCCESS) {
		return result;
	}
	std::int64_t nanoseconds = static_cast<std::int64_t>(origin.nanosecond) + floor_mod(time, nanoseconds_per_second);
	std::int64_t seconds = seconds_from_civil(origin) + floor_div(time, nanoseconds_per_second);
	if (nanoseconds >= nanoseconds_per_second) {
		nanoseconds -= nanoseconds_per_second;
		seconds += 1;
	}
	const std::int64_t second_of_day = floor_mod(seconds, seconds_per_day);
	std::int64_t year;
	unsigned month;
	unsigned day;
	civil_from_days(floor_div(seconds, seconds_per_day), &year, &month, &day);
	if (year < std::numeric_limits<std::int32_t>::min() || year > std::numeric_limits<std::int32_t>::max()) {
		return CO_RESULT_0_ERROR_OUT_OF_RANGE;
	}
	*calendar_time = co_calendar_time_0{
		.year = static_cast<std::int32_t>(year),
		.month = static_cast<std::uint8_t>(month),
		.day = static_cast<std::uint8_t>(day),
		.hour = static_cast<std::uint8_t>(second_of_day / 3600),
		.minute = static_cast<std::uint8_t>(second_of_day / 60 % 60),
		.second = static_cast<std::uint8_t>(second_of_day % 60),
		.nanosecond = static_cast<std::uint32_t>(nanoseconds),
	};
	return CO_RESULT_0_SUCCESS;
}
