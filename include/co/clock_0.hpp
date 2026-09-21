#pragma once

#include <co/clock_0.h>

#include <co.hpp>

#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace co
{

/// @defgroup cpp_clock_0 Clock
/// @brief Clocks whose time may jump or change rate, and which notify their changes.
///
/// A clock has a base time, which never jumps and never decreases, and a time, which is what the clock shows and may
/// jump, run at another rate or be paused. Delays are measured in base time, deadlines are given in time. The mapping
/// (clock_0_mapping_0) describes how the time follows the base time; each change of it increments the change
/// generation, and clock_0::wait_change_0() waits for a change after a given generation.
///
/// See @ref c_clock_0 for the full contract and the C API.
/// @{

/// @brief How the time of a clock follows its base time, from a base time on. See co_clock_0_mapping_0.
struct clock_0_mapping_0 final
{
	/// @brief Base time from which the mapping applies.
	std::uint64_t base_time;
	/// @brief Time at base_time.
	std::int64_t time;
	/// @brief Numerator of the rate of the time relative to the base time; 0 while the clock is paused.
	std::uint64_t rate_numerator;
	/// @brief Denominator of the rate of the time relative to the base time; never 0.
	std::uint64_t rate_denominator;
};

static_assert(std::is_standard_layout_v<clock_0_mapping_0>);
static_assert(sizeof(clock_0_mapping_0) == sizeof(co_clock_0_mapping_0));
static_assert(offsetof(clock_0_mapping_0, base_time) == offsetof(co_clock_0_mapping_0, base_time));
static_assert(offsetof(clock_0_mapping_0, time) == offsetof(co_clock_0_mapping_0, time));
static_assert(offsetof(clock_0_mapping_0, rate_numerator) == offsetof(co_clock_0_mapping_0, rate_numerator));
static_assert(offsetof(clock_0_mapping_0, rate_denominator) == offsetof(co_clock_0_mapping_0, rate_denominator));

/// @brief A calendar time: a date of the proleptic Gregorian calendar and a time of day, as in UTC without leap seconds. See
/// co_calendar_time_0.
struct calendar_time_0 final
{
	/// @brief Year; 0 is 1 BC.
	std::int32_t year;
	/// @brief Month of the year, 1 to 12.
	std::uint8_t month;
	/// @brief Day of the month, 1 to the number of days of the month.
	std::uint8_t day;
	/// @brief Hour of the day, 0 to 23.
	std::uint8_t hour;
	/// @brief Minute of the hour, 0 to 59.
	std::uint8_t minute;
	/// @brief Second of the minute, 0 to 59.
	std::uint8_t second;
	/// @brief Nanosecond of the second, 0 to 999999999.
	std::uint32_t nanosecond;
};

static_assert(std::is_standard_layout_v<calendar_time_0>);
static_assert(sizeof(calendar_time_0) == sizeof(co_calendar_time_0));
static_assert(offsetof(calendar_time_0, year) == offsetof(co_calendar_time_0, year));
static_assert(offsetof(calendar_time_0, month) == offsetof(co_calendar_time_0, month));
static_assert(offsetof(calendar_time_0, day) == offsetof(co_calendar_time_0, day));
static_assert(offsetof(calendar_time_0, hour) == offsetof(co_calendar_time_0, hour));
static_assert(offsetof(calendar_time_0, minute) == offsetof(co_calendar_time_0, minute));
static_assert(offsetof(calendar_time_0, second) == offsetof(co_calendar_time_0, second));
static_assert(offsetof(calendar_time_0, nanosecond) == offsetof(co_calendar_time_0, nanosecond));

/// @brief Completion callback of clock_0::wait_change_0(), invoked exactly once per successful wait with
/// `result_0::success` or `result_0::cancelled` and the change generation. See co_clock_0_wait_change_0_completion_fn.
using clock_0_wait_change_0_completion_fn = void (*)(void* data, result_0 status, std::size_t generation) noexcept;

/// @brief Handle of a clock; destroyed with destroy(). See @ref c_clock_0 for its contract.
class clock_0 final
{
public:

	/// @brief Completion callback of wait_change_0().
	using wait_change_0_completion_fn = clock_0_wait_change_0_completion_fn;

	clock_0() noexcept = default;

	clock_0(std::nullptr_t) noexcept
	{
	}

	explicit clock_0(co_clock_0 handle) noexcept
		: m_handle(handle)
	{
	}

	explicit operator co_clock_0() const noexcept
	{
		return m_handle;
	}

	explicit operator bool() const noexcept
	{
		return m_handle != nullptr;
	}

	friend bool operator==(const clock_0&, const clock_0&) noexcept = default;

	/// @brief Destroys the clock; does nothing when the handle is null. See co_clock_0_destroy().
	/// @pre No wait of the clock is pending, and no other call on it is in progress.
	void destroy() const noexcept
	{
		co_clock_0_destroy(m_handle);
	}

	/// @brief Gets the calendar origin of the clock; `result_0::error_not_supported` when it has none. See
	/// co_clock_0_get_calendar_origin_0().
	result_0 get_calendar_origin_0(calendar_time_0* origin) const noexcept
	{
		return static_cast<result_0>(co_clock_0_get_calendar_origin_0(m_handle, reinterpret_cast<co_calendar_time_0*>(origin)));
	}

	/// @brief Gets the time of the clock, in signed nanoseconds since its origin. See co_clock_0_get_time_0().
	void get_time_0(std::int64_t* time) const noexcept
	{
		co_clock_0_get_time_0(m_handle, time);
	}

	/// @brief Gets the base time of the clock, in nanoseconds. See co_clock_0_get_base_time_0().
	void get_base_time_0(std::uint64_t* base_time) const noexcept
	{
		co_clock_0_get_base_time_0(m_handle, base_time);
	}

	/// @brief Gets the mapping of the clock and the change generation it belongs to. See co_clock_0_get_mapping_0().
	void get_mapping_0(clock_0_mapping_0* mapping, std::size_t* generation) const noexcept
	{
		co_clock_0_get_mapping_0(m_handle, reinterpret_cast<co_clock_0_mapping_0*>(mapping), generation);
	}

	/// @brief Waits
	/// @param cancellation Cancellation whose request cancels the wait, or null.
	/// @pre after_generation is a change generation the clock had.
	/// @pre executor and completion_fn are not null.
	result_0 wait_change_0(std::size_t after_generation, cancellation_0 cancellation, executor_0 executor, wait_change_0_completion_fn completion_fn, void* data) const noexcept
	{
		return static_cast<result_0>(co_clock_0_wait_change_0(m_handle, after_generation, static_cast<co_cancellation_0>(cancellation), static_cast<co_executor_0>(executor), reinterpret_cast<co_clock_0_wait_change_0_completion_fn>(completion_fn), data));
	}

	/// @brief Cancels every pending wait of the clock. See co_clock_0_cancel_0().
	result_0 cancel_0() const noexcept
	{
		return static_cast<result_0>(co_clock_0_cancel_0(m_handle));
	}

	/// @brief Converts a calendar time to a time of the clock. See co_clock_0_time_from_calendar_0().
	result_0 time_from_calendar_0(const calendar_time_0& calendar_time, std::int64_t* time) const noexcept
	{
		return static_cast<result_0>(co_clock_0_time_from_calendar_0(m_handle, reinterpret_cast<const co_calendar_time_0*>(&calendar_time), time));
	}

	/// @brief Converts a time of the clock to a calendar time. See co_clock_0_time_to_calendar_0().
	result_0 time_to_calendar_0(std::int64_t time, calendar_time_0* calendar_time) const noexcept
	{
		return static_cast<result_0>(co_clock_0_time_to_calendar_0(m_handle, time, reinterpret_cast<co_calendar_time_0*>(calendar_time)));
	}

private:
	co_clock_0 m_handle = nullptr;
};

static_assert(detail::is_handle_layout_compatible<clock_0, co_clock_0>);

/// @}

} // namespace co
