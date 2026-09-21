#pragma once

#include <co.h>

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

/// @defgroup c_clock_0 Clock
/// @brief Clocks whose time may jump or change rate, and which notify their changes.
///
/// A clock has two times, both in nanoseconds:
///
/// - The base time never jumps and never decreases. Its origin is unspecified; whether it advances while the system is
///   suspended depends on the clock class. Delays (expiry after a duration) are measured in base time.
/// - The time is what the clock shows: signed nanoseconds since its origin, which is clock specific (for example the
///   Unix epoch for a wall clock, or the creation of the clock); it is negative before the origin. It may
///   jump, run at another rate than the base time, or be paused, for example when the clock is set, scaled or driven
///   manually. Deadlines (expiry at a point in time) are given in time.
///
/// The mapping (co_clock_0_mapping_0) describes how the time follows the base time. Every change of the mapping, other
/// than gradual adjustments of a clock that follows an external source (for example the system slewing its wall clock),
/// is a change of the clock:
///
/// - The clock counts its changes since its creation: their count, modulo SIZE_MAX + 1, is the change generation,
///   starting at 0. co_clock_0_get_mapping_0() gets the mapping together with the change generation it belongs to.
/// - co_clock_0_wait_change_0() waits for a change after a given change generation, which must be one the clock had: it
///   completes once the change generation differs. If it already does, the wait completes right away; otherwise with
///   the next change. Generations are compared modulo SIZE_MAX + 1 (with !=), so they stay valid when they wrap around.
/// - Consumers that only depend on the current mapping, such as a timer service waiting for a deadline, read the
///   mapping, act on it, and wait after its generation: several changes before they get to read the mapping again are
///   seen as one, and none is missed between reading and waiting.
/// - Any number of waits may be pending, for one consumer or for several consumers sharing the clock; each consumer
///   keeps the generation it saw last.
/// - After a successful wait, its completion callback is posted to the given executor exactly once, with
///   #CO_RESULT_0_SUCCESS when the clock changed or #CO_RESULT_0_CANCELLED when the wait was cancelled first. It is never
///   invoked from within co_clock_0_wait_change_0(). If the executor rejects a completion callback after the wait call
///   returned, the process is terminated.
/// - A wait is pending from its successful wait call until its completion callback is invoked. The clock may be
///   destroyed when no wait is pending; in particular from within the completion callback of its last wait.
/// - The functions of a clock other than co_clock_0_destroy() may be called from any thread, concurrently.
///
/// A clock may have a calendar origin: the calendar time (co_calendar_time_0) at which its time is 0, a fixed value
/// chosen by the clock class (for example the creation of the clock). Its time `t` is then the calendar time `t`
/// nanoseconds after the origin, without leap seconds. co_clock_0_get_calendar_origin_0() gets it, and
/// co_clock_0_time_from_calendar_0() and co_clock_0_time_to_calendar_0() convert between calendar times and times of the
/// clock, so that a deadline given as a calendar time can be set on a timer without knowing the origin of its clock.
/// The calendar origin only labels the times of the clock; it does not constrain how the clock runs:
///
/// - For a wall clock, the calendar time is the real UTC time. Setting the system time is a change of the clock; it
///   does not move the origin.
/// - A scaled or paused clock (for example game time) may have a calendar origin too: its calendar time then runs at
///   its own rate. A deadline set as a calendar time is reached when the clock reaches it.
/// - Clocks without a meaningful calendar (for example a stopwatch or a manual test clock) have no calendar origin.
///
/// @code
/// // Re-arms a deadline whenever the clock changes.
/// static void on_change(void* data, co_result_0 status, size_t generation) CO_NOEXCEPT
/// {
///     waiter* w = (waiter*)data;
///     if (status == CO_RESULT_0_CANCELLED) {
///         // ... the waiter is stopping ...
///         return;
///     }
///     co_clock_0_mapping_0 mapping;
///     co_clock_0_get_mapping_0(w->clock, &mapping, &generation);
///     // ... re-arm the deadline using mapping ...
///     co_clock_0_wait_change_0(w->clock, generation, w->cancellation, w->executor, &on_change, w);
/// }
/// @endcode
/// @{

// Defined in the Clock implementation section: clock classes start their objects with it.
typedef struct co_clock_0_t co_clock_0_t;
/// @brief Handle of a clock.
typedef struct co_clock_0_t* co_clock_0;

/// @brief Casts a handle to co_clock_0, unchecked; `NULL` stays `NULL`. See "Handles and casts" in Concepts.
#define CO_CLOCK_0_CAST(handle) ((co_clock_0)(handle))

/// @brief How the time of a clock follows its base time, from a base time on.
///
/// At every base time `b` not before #base_time, until the next change of the clock, the time is
/// `time + floor((b - base_time) * rate_numerator / rate_denominator)`, computed exactly (as with 128-bit intermediate
/// values). Conversely, the time reaches a deadline `t` after #time at the base time
/// `base_time + ceil((t - time) * rate_denominator / rate_numerator)`, or never while #rate_numerator is 0.
///
/// The mapping is exact for clocks that compute their time from the base time (for example scaled or manual clocks).
/// For clocks that follow an external source (for example the system wall clock, which the system may slew), it is the
/// best approximation at the time it was read.
typedef struct co_clock_0_mapping_0 CO_FINAL
{
	/// @brief Base time from which the mapping applies; not after the base time at which the mapping was read.
	uint64_t base_time;
	/// @brief Time at #base_time.
	int64_t time;
	/// @brief Numerator of the rate of the time relative to the base time; 0 while the clock is paused.
	uint64_t rate_numerator;
	/// @brief Denominator of the rate of the time relative to the base time; never 0.
	uint64_t rate_denominator;
} co_clock_0_mapping_0;

/// @brief A calendar time: a date of the proleptic Gregorian calendar and a time of day, as in UTC without leap
/// seconds and without time zone.
/// @see co_clock_0_get_calendar_origin_0()
typedef struct co_calendar_time_0 CO_FINAL
{
	/// @brief Year; 0 is 1 BC, negative years are before it.
	int32_t year;
	/// @brief Month of the year, 1 to 12.
	uint8_t month;
	/// @brief Day of the month, 1 to the number of days of the month.
	uint8_t day;
	/// @brief Hour of the day, 0 to 23.
	uint8_t hour;
	/// @brief Minute of the hour, 0 to 59.
	uint8_t minute;
	/// @brief Second of the minute, 0 to 59; leap seconds (60) are not represented.
	uint8_t second;
	/// @brief Nanosecond of the second, 0 to 999999999.
	uint32_t nanosecond;
} co_calendar_time_0;

/// @brief Completion callback of co_clock_0_wait_change_0(), invoked exactly once per successful wait.
/// @param data User data passed to co_clock_0_wait_change_0().
/// @param status #CO_RESULT_0_SUCCESS when the clock changed, or #CO_RESULT_0_CANCELLED when the wait was cancelled
/// first.
/// @param generation The change generation of the clock at that point: the generation of the change that completed the
/// wait, or the unchanged generation the wait was given when cancelled.
typedef void (* co_clock_0_wait_change_0_completion_fn)(void* data, co_result_0 status, size_t generation) CO_NOEXCEPT;

/// @brief Destroys a clock; does nothing when clock is `NULL`.
/// @pre No wait of the clock is pending, and no other call on it is in progress.
CO_API
void co_clock_0_destroy(co_clock_0 clock) CO_NOEXCEPT;

/// @brief Gets the calendar origin of the clock: the calendar time at which its time is 0.
/// @param origin Receives the origin; unchanged on failure.
/// @retval CO_RESULT_0_SUCCESS origin received the calendar origin of the clock.
/// @retval CO_RESULT_0_ERROR_NOT_SUPPORTED The clock has no calendar origin.
CO_API
co_result_0 co_clock_0_get_calendar_origin_0(co_clock_0 clock, co_calendar_time_0* origin) CO_NOEXCEPT;

/// @brief Gets the time of the clock, in signed nanoseconds since its origin.
CO_API
void co_clock_0_get_time_0(co_clock_0 clock, int64_t* time) CO_NOEXCEPT;

/// @brief Gets the base time of the clock, in nanoseconds; it never decreases.
CO_API
void co_clock_0_get_base_time_0(co_clock_0 clock, uint64_t* base_time) CO_NOEXCEPT;

/// @brief Gets the mapping of the clock and the change generation it belongs to, read together.
/// @param mapping Receives the mapping, which applies until the change generation differs from the one received.
/// @param generation Receives the change generation: the number of changes since the creation of the clock, modulo
/// SIZE_MAX + 1.
CO_API
void co_clock_0_get_mapping_0(co_clock_0 clock, co_clock_0_mapping_0* mapping, size_t* generation) CO_NOEXCEPT;

/// @brief Waits for a change of the clock after the given change generation.
///
/// On success, completion_fn is posted to executor exactly once. On failure, completion_fn is never invoked.
///
/// @param after_generation The wait completes once the change generation differs: right away if it already does,
/// otherwise with the next change. Usually 0, or the generation received from co_clock_0_get_mapping_0() or passed to
/// the completion callback of the previous wait.
/// @param cancellation Cancellation whose request cancels the wait, or `NULL`. When it is already requested, the wait
/// completes with #CO_RESULT_0_CANCELLED. Must stay valid until completion_fn has been invoked.
/// @param executor Executor to which completion_fn is posted. Must stay valid until completion_fn has been invoked.
/// @param completion_fn Callback invoked when the clock changed or the wait was cancelled.
/// @param data User data passed to completion_fn.
/// @retval CO_RESULT_0_SUCCESS The wait is pending.
/// @retval CO_RESULT_0_ERROR_OUT_OF_MEMORY The clock cannot allocate its bookkeeping.
/// @return The error of co_executor_0_post_0() when the wait completes right away and executor rejects completion_fn.
/// @pre after_generation is a change generation the clock had (it is never ahead of the change generation).
/// @pre executor and completion_fn are not `NULL`.
CO_API
co_result_0 co_clock_0_wait_change_0(co_clock_0 clock, size_t after_generation, co_cancellation_0 cancellation, co_executor_0 executor, co_clock_0_wait_change_0_completion_fn completion_fn, void* data) CO_NOEXCEPT;

/// @brief Cancels every pending wait of the clock, of every consumer sharing it: their completion callbacks are invoked
/// with #CO_RESULT_0_CANCELLED. To cancel the waits of one consumer only, use the cancellation passed to its waits. The
/// change generation is not affected. Does nothing when no wait is pending.
/// @retval CO_RESULT_0_SUCCESS The pending waits were cancelled, or there was nothing to cancel.
CO_API
co_result_0 co_clock_0_cancel_0(co_clock_0 clock) CO_NOEXCEPT;

/// @brief Converts a calendar time to a time of the clock, for example the deadline of a timer bound to it.
///
/// Uses the calendar origin of the clock only: the result does not depend on the current time of the clock.
/// @param calendar_time Calendar time to convert.
/// @param time Receives the time of the clock at calendar_time, in signed nanoseconds since its origin; unchanged on
/// failure.
/// @retval CO_RESULT_0_SUCCESS time received the time.
/// @retval CO_RESULT_0_ERROR_NOT_SUPPORTED The clock has no calendar origin.
/// @retval CO_RESULT_0_ERROR_INVALID_ARGUMENT A field of calendar_time is out of its range.
/// @retval CO_RESULT_0_ERROR_OUT_OF_RANGE calendar_time is too far from the origin of the clock (about 292 years).
CO_API
co_result_0 co_clock_0_time_from_calendar_0(co_clock_0 clock, const co_calendar_time_0* calendar_time, int64_t* time) CO_NOEXCEPT;

/// @brief Converts a time of the clock to a calendar time.
///
/// Uses the calendar origin of the clock only: the result does not depend on the current time of the clock.
/// @param time Time of the clock, in signed nanoseconds since its origin.
/// @param calendar_time Receives the calendar time; unchanged on failure.
/// @retval CO_RESULT_0_SUCCESS calendar_time received the calendar time.
/// @retval CO_RESULT_0_ERROR_NOT_SUPPORTED The clock has no calendar origin.
/// @retval CO_RESULT_0_ERROR_OUT_OF_RANGE The year of the calendar time does not fit in co_calendar_time_0::year.
CO_API
co_result_0 co_clock_0_time_to_calendar_0(co_clock_0 clock, int64_t time, co_calendar_time_0* calendar_time) CO_NOEXCEPT;

/// @defgroup c_clock_0_impl Clock implementation
/// @ingroup c_clock_0
/// @brief Declarations used to implement clock classes.
///
/// A clock object starts with a co_clock_0_t, which the class initializes with co_clock_0_init() before the object is
/// used. Methods are only ever appended to co_clock_0_vtable, as described for co_executor_0_vtable.
/// @{

/// @brief Type of co_clock_0_vtable::destroy.
typedef void (* co_clock_0_destroy_fn)(co_clock_0 self) CO_NOEXCEPT;

/// @brief Type of co_clock_0_vtable::get_calendar_origin_0.
typedef co_result_0 (* co_clock_0_get_calendar_origin_0_fn)(co_clock_0 self, co_calendar_time_0* origin) CO_NOEXCEPT;

/// @brief Type of co_clock_0_vtable::get_time_0.
typedef void (* co_clock_0_get_time_0_fn)(co_clock_0 self, int64_t* time) CO_NOEXCEPT;

/// @brief Type of co_clock_0_vtable::get_base_time_0.
typedef void (* co_clock_0_get_base_time_0_fn)(co_clock_0 self, uint64_t* base_time) CO_NOEXCEPT;

/// @brief Type of co_clock_0_vtable::get_mapping_0.
typedef void (* co_clock_0_get_mapping_0_fn)(co_clock_0 self, co_clock_0_mapping_0* mapping, size_t* generation) CO_NOEXCEPT;

/// @brief Type of co_clock_0_vtable::wait_change_0.
typedef co_result_0 (* co_clock_0_wait_change_0_fn)(co_clock_0 self, size_t after_generation, co_cancellation_0 cancellation, co_executor_0 executor, co_clock_0_wait_change_0_completion_fn completion_fn, void* data) CO_NOEXCEPT;

/// @brief Type of co_clock_0_vtable::cancel_0.
typedef co_result_0 (* co_clock_0_cancel_0_fn)(co_clock_0 self) CO_NOEXCEPT;

typedef struct co_clock_0_vtable co_clock_0_vtable;

/// @brief Start of every clock object.
struct co_clock_0_t
{
	/// @brief Methods of the class of the object.
	const co_clock_0_vtable* vtable;
};

/// @brief Methods of a clock class. Must stay valid while objects of the class exist.
struct co_clock_0_vtable CO_FINAL
{
	/// @brief API version of the headers the class is compiled against; usually CO_API_VERSION_0.
	co_version_0 api_version;
	/// @brief Required. Implements co_clock_0_destroy(): destroys the object and deallocates its memory.
	co_clock_0_destroy_fn destroy;
	/// @brief Required. Implements co_clock_0_get_calendar_origin_0(); returns #CO_RESULT_0_ERROR_NOT_SUPPORTED when the
	/// class has no calendar origin. The origin of an object never changes, and is a valid co_calendar_time_0.
	co_clock_0_get_calendar_origin_0_fn get_calendar_origin_0;
	/// @brief Required. Implements co_clock_0_get_time_0().
	co_clock_0_get_time_0_fn get_time_0;
	/// @brief Required. Implements co_clock_0_get_base_time_0().
	co_clock_0_get_base_time_0_fn get_base_time_0;
	/// @brief Required. Implements co_clock_0_get_mapping_0().
	co_clock_0_get_mapping_0_fn get_mapping_0;
	/// @brief Required. Implements co_clock_0_wait_change_0().
	co_clock_0_wait_change_0_fn wait_change_0;
	/// @brief Required. Implements co_clock_0_cancel_0().
	co_clock_0_cancel_0_fn cancel_0;
};

/// @brief Initializes the start of a clock object: stores its vtable.
/// @pre clock_vtable has every required method and stays valid while the object exists.
static inline void co_clock_0_init(co_clock_0 clock, const co_clock_0_vtable* clock_vtable) CO_NOEXCEPT
{
	clock->vtable = clock_vtable;
}

/// @}

/// @}

#ifdef __cplusplus
}
#endif
