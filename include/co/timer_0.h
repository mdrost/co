#pragma once

#include <co.h>
#include <co/clock_0.h>

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

/// @defgroup c_timer_0 Timer
/// @brief Timers with a settable expiry whose waits complete on an executor.
///
/// Timers are created by a timer service with co_timer_0_service_0_create_timer_0(); the timer service is found in an
/// event domain that provides #CO_SERVICE_TYPE_0_TIMER_0_SERVICE_0.
///
/// - A timer is bound to the clock given at its creation (co_timer_0_create_info_0::clock): expiries at a point in time
///   are in the time of the clock, expiries after a duration in its base time.
/// - A timer has an expiry:
///   duration from now. A new timer has no expiry: it does not expire until an expiry is set.
/// - The timer has expired once its expiry has been reached. It stays expired until its expiry is set again.
/// - co_timer_0_wait_0() waits until the timer has expired: right away if it already has, otherwise once its expiry is
///   reached. Any number of waits may be pending.
/// - Setting the expiry applies to the pending waits too: they complete once the new expiry is reached, or right away
///   if it already has been. Setting the expiry never completes or cancels a wait by itself.
/// - After a successful wait, its completion callback is posted to the given executor exactly once, with
///   #CO_RESULT_0_SUCCESS when the timer expired or #CO_RESULT_0_CANCELLED when the wait was cancelled first, through
///   its cancellation or co_timer_0_cancel_0(). It is never invoked from within co_timer_0_wait_0(). If the executor
///   rejects a completion callback after the wait call returned, the process is terminated.
/// - A wait is pending from its successful co_timer_0_wait_0() until its completion callback is invoked. The timer may
///   be destroyed when no wait is pending; in particular from within the completion callback of its last wait.
/// - The functions of a timer other than co_timer_0_destroy() may be called from any thread, concurrently; concurrent
///   calls take effect in some order.
///
/// To stop using a timer while a wait may be pending, cancel the wait and destroy the timer from its completion
/// callback, which is invoked (with #CO_RESULT_0_SUCCESS or #CO_RESULT_0_CANCELLED) once the timer no longer uses it.
/// The owner must not wait again from that callback, for example by setting a flag that the callback checks.
///
/// @code
/// // A timer that ticks every 100 ms until it is stopped from a handler of t->strand_executor.
/// static void on_tick(void* data, co_result_0 status) CO_NOEXCEPT
/// {
///     ticker* t = (ticker*)data;
///     if (t->stopping) {
///         co_timer_0_destroy(t->timer); // no wait is pending: this was the last one
///         free(t);
///         return;
///     }
///     // ... do the periodic work ...
///     co_timer_0_expiry_after_0(t->timer, 100000000);
///     co_timer_0_wait_0(t->timer, NULL, t->strand_executor, &on_tick, t);
/// }
///
/// co_service_0 service;
/// co_result_0 result = co_event_facility_0_get_service_0(event_facility, CO_SERVICE_TYPE_0_TIMER_0_SERVICE_0, &service);
/// // ...
/// co_timer_0_create_info_0 timer_create_info = {
///     .structure_type = CO_STRUCTURE_TYPE_0_TIMER_0_CREATE_INFO_0,
///     .clock = clock,
/// };
/// result = co_timer_0_service_0_create_timer_0(CO_TIMER_0_SERVICE_0_CAST(service), &timer_create_info, &t->timer);
/// // ...
/// result = co_timer_0_expiry_after_0(t->timer, 100000000);
/// // ...
/// result = co_timer_0_wait_0(t->timer, NULL, t->strand_executor, &on_tick, t);
///
/// // Stop, in a handler of t->strand_executor: on_tick finishes it.
/// t->stopping = true;
/// co_timer_0_cancel_0(t->timer);
/// @endcode
/// @{

// Defined in the Timer implementation section: timer classes start their objects with it.
typedef struct co_timer_0_t co_timer_0_t;
/// @brief Handle of a timer.
typedef struct co_timer_0_t* co_timer_0;

/// @brief Casts a handle to co_timer_0, unchecked; `NULL` stays `NULL`. See "Handles and casts" in Concepts.
#define CO_TIMER_0_CAST(handle) ((co_timer_0)(handle))

/// @brief Structure type of co_timer_0_create_info_0.
extern CO_API
const co_structure_type_0 CO_STRUCTURE_TYPE_0_TIMER_0_CREATE_INFO_0;

/// @brief Create info of a timer, passed to co_timer_0_service_0_create_timer_0().
typedef struct co_timer_0_create_info_0 CO_FINAL
{
	co_structure_type_0 structure_type;
	const co_in_structure_0* next_structure;

	/// @brief Clock the timer is bound to: co_timer_0_expiry_at_0() takes a time of this clock, and
	/// co_timer_0_expiry_after_0() a duration of its base time. The timer does not own the clock.
	/// @pre Not `NULL`.
	/// @pre The clock outlives the timer.
	co_clock_0 clock;
} co_timer_0_create_info_0;

/// @brief Completion callback of co_timer_0_wait_0(), invoked exactly once per successful wait.
/// @param data User data passed to co_timer_0_wait_0().
/// @param status #CO_RESULT_0_SUCCESS when the timer expired, or #CO_RESULT_0_CANCELLED when the wait was cancelled
/// first.
typedef void (* co_timer_0_wait_0_completion_fn)(void* data, co_result_0 status) CO_NOEXCEPT;

/// @brief Destroys a timer; does nothing when timer is `NULL`.
/// @pre No wait of the timer is pending: the completion callback of its last wait has been invoked or is being invoked.
/// Destroying it from within that callback is allowed.
/// @pre No other call on the timer is in progress.
CO_API
void co_timer_0_destroy(co_timer_0 timer) CO_NOEXCEPT;

/// @brief Gets the clock the timer is bound to: co_timer_0_create_info_0::clock of its creation.
///
/// Lets code that is given a timer compute expiries on its clock, for example from co_clock_0_get_time_0().
/// @param timer Timer whose clock is got.
/// @param clock Receives the clock; never `NULL`. The timer does not own it.
CO_API
void co_timer_0_get_clock_0(co_timer_0 timer, co_clock_0* clock) CO_NOEXCEPT;

/// @brief Sets the expiry of the timer to a point in time.
///
/// The pending waits complete once the new expiry is reached, or right away if it already has been.
///
/// @param timer Timer whose expiry is set.
/// @param time_ns Point in time at which the timer expires, as a time of the clock of the timer (see co_clock_0): in
/// signed nanoseconds since the origin of the clock. The expiry follows changes of the clock: it is reached when the
/// time of the clock reaches it. A point in time that has already been reached makes the timer expire as soon as
/// possible.
/// @retval CO_RESULT_0_SUCCESS The expiry is set.
/// @retval CO_RESULT_0_ERROR_OUT_OF_RANGE time_ns is later than the latest expiry the timer class can represent; it is
/// never clamped.
/// @return An error when the expiry could not be set; the expiry is then unchanged.
CO_API
co_result_0 co_timer_0_expiry_at_0(co_timer_0 timer, int64_t time_ns) CO_NOEXCEPT;

/// @brief Sets the expiry of the timer to a duration from now.
///
/// The pending waits complete once the new expiry is reached, or right away if it already has been.
///
/// @param timer Timer whose expiry is set.
/// @param duration_ns Duration in nanoseconds after which the timer expires, rounded up to the resolution of the timer
/// class. The duration is measured in the base time of the clock of the timer (see co_clock_0), which does not jump;
/// whether it counts time the system is suspended depends on the clock class. Zero makes the timer expire as soon as
/// possible.
/// @retval CO_RESULT_0_SUCCESS The expiry is set.
/// @return An error when the expiry could not be set; the expiry is then unchanged.
CO_API
co_result_0 co_timer_0_expiry_after_0(co_timer_0 timer, uint64_t duration_ns) CO_NOEXCEPT;

/// @brief Waits until the timer has expired.
///
/// The wait completes right away if the timer has already expired, otherwise once its expiry, as set at that point, is
/// reached. On success, completion_fn is posted to executor exactly once. On failure, completion_fn is never invoked.
///
/// @param timer Timer to wait for.
/// @param cancellation Cancellation whose request cancels the wait, or `NULL`. When it is already requested, the wait
/// completes with #CO_RESULT_0_CANCELLED. Must stay valid until completion_fn has been invoked.
/// @param executor Executor to which completion_fn is posted. Must stay valid until completion_fn has been invoked.
/// @param completion_fn Callback invoked when the timer expired or the wait was cancelled.
/// @param data User data passed to completion_fn.
/// @retval CO_RESULT_0_SUCCESS The wait is pending.
/// @retval CO_RESULT_0_ERROR_OUT_OF_MEMORY The timer cannot allocate its bookkeeping.
/// @return The error of co_executor_0_post_0() when the wait completes right away and executor rejects completion_fn.
/// @pre executor and completion_fn are not `NULL`.
CO_API
co_result_0 co_timer_0_wait_0(co_timer_0 timer, co_cancellation_0 cancellation, co_executor_0 executor, co_timer_0_wait_0_completion_fn completion_fn, void* data) CO_NOEXCEPT;

/// @brief Cancels every pending wait of the timer.
///
/// The completion callbacks of the waits pending at that point are invoked with #CO_RESULT_0_CANCELLED, unless the
/// timer expired first. To cancel one wait only, use the cancellation passed to it. The expiry is not affected. Does
/// nothing and returns #CO_RESULT_0_SUCCESS when no wait is pending.
///
/// @param timer Timer whose pending waits are cancelled.
/// @retval CO_RESULT_0_SUCCESS The pending waits were cancelled, or there was nothing to cancel.
/// @return An error when the waits could not be cancelled; they then stay pending and the call may be retried.
CO_API
co_result_0 co_timer_0_cancel_0(co_timer_0 timer) CO_NOEXCEPT;

/// @brief Sets the expiry of the timer to a calendar time.
///
/// Converts calendar_time with co_clock_0_time_from_calendar_0() on the clock of the timer, then sets the expiry as
/// co_timer_0_expiry_at_0() does. The calendar origin of a clock never changes, so the expiry is the same as if it
/// were converted later; it follows changes of the clock like any expiry at a point in time.
///
/// @param timer Timer whose expiry is set.
/// @param calendar_time Calendar time at which the timer expires.
/// @retval CO_RESULT_0_SUCCESS The expiry is set.
/// @retval CO_RESULT_0_ERROR_NOT_SUPPORTED The clock of the timer has no calendar origin.
/// @retval CO_RESULT_0_ERROR_INVALID_ARGUMENT A field of calendar_time is out of its range.
/// @retval CO_RESULT_0_ERROR_OUT_OF_RANGE calendar_time is too far from the origin of the clock, or later than the
/// latest expiry the timer class can represent; it is never clamped.
/// @return Another error of co_timer_0_expiry_at_0().
/// On failure, the expiry is unchanged.
CO_API
co_result_0 co_timer_0_expiry_at_calendar_0(co_timer_0 timer, const co_calendar_time_0* calendar_time) CO_NOEXCEPT;

/// @defgroup c_timer_0_impl Timer implementation
/// @ingroup c_timer_0
/// @brief Declarations used to implement timer classes.
///
/// Timer classes are provided by timer service classes, which create their objects in
/// co_timer_0_service_0_create_timer_0(). A timer object starts with a co_timer_0_t, which the class initializes with
/// co_timer_0_init() before the object is used. Methods are only ever appended to co_timer_0_vtable, as described for
/// co_executor_0_vtable.
/// @{

/// @brief Type of co_timer_0_vtable::destroy.
typedef void (* co_timer_0_destroy_fn)(co_timer_0 self) CO_NOEXCEPT;

/// @brief Type of co_timer_0_vtable::get_clock_0.
typedef void (* co_timer_0_get_clock_0_fn)(co_timer_0 self, co_clock_0* clock) CO_NOEXCEPT;

/// @brief Type of co_timer_0_vtable::expiry_at_0.
typedef co_result_0 (* co_timer_0_expiry_at_0_fn)(co_timer_0 self, int64_t time_ns) CO_NOEXCEPT;

/// @brief Type of co_timer_0_vtable::expiry_after_0.
typedef co_result_0 (* co_timer_0_expiry_after_0_fn)(co_timer_0 self, uint64_t duration_ns) CO_NOEXCEPT;

/// @brief Type of co_timer_0_vtable::wait_0.
typedef co_result_0 (* co_timer_0_wait_0_fn)(co_timer_0 self, co_cancellation_0 cancellation, co_executor_0 executor, co_timer_0_wait_0_completion_fn completion_fn, void* data) CO_NOEXCEPT;

/// @brief Type of co_timer_0_vtable::cancel_0.
typedef co_result_0 (* co_timer_0_cancel_0_fn)(co_timer_0 self) CO_NOEXCEPT;

typedef struct co_timer_0_vtable co_timer_0_vtable;

/// @brief Start of every timer object.
struct co_timer_0_t
{
	/// @brief Methods of the class of the object.
	const co_timer_0_vtable* vtable;
};

/// @brief Methods of a timer class. Must stay valid while objects of the class exist.
struct co_timer_0_vtable CO_FINAL
{
	/// @brief API version of the headers the class is compiled against; usually CO_API_VERSION_0.
	co_version_0 api_version;
	/// @brief Required. Implements co_timer_0_destroy(): destroys the object and deallocates its memory.
	co_timer_0_destroy_fn destroy;
	/// @brief Required. Implements co_timer_0_get_clock_0().
	co_timer_0_get_clock_0_fn get_clock_0;
	/// @brief Required. Implements co_timer_0_expiry_at_0().
	co_timer_0_expiry_at_0_fn expiry_at_0;
	/// @brief Required. Implements co_timer_0_expiry_after_0().
	co_timer_0_expiry_after_0_fn expiry_after_0;
	/// @brief Required. Implements co_timer_0_wait_0().
	co_timer_0_wait_0_fn wait_0;
	/// @brief Required. Implements co_timer_0_cancel_0().
	co_timer_0_cancel_0_fn cancel_0;
};

/// @brief Initializes the start of a timer object: stores its vtable.
/// @pre timer_vtable has every required method and stays valid while the object exists.
static inline void co_timer_0_init(co_timer_0 timer, const co_timer_0_vtable* timer_vtable) CO_NOEXCEPT
{
	timer->vtable = timer_vtable;
}

/// @}

/// @}

#ifdef __cplusplus
}
#endif
