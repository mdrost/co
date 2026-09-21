#pragma once

#include <co.h>

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

/// @defgroup c_timer_0 Timer
/// @brief One-shot timers whose completions are posted to an executor.
///
/// Timers are created by a timer service with co_timer_0_service_0_create_timer_0(); the timer service is found in an
/// event domain that provides #CO_SERVICE_TYPE_0_TIMER_0_SERVICE_0. A timer is reusable: it runs one start at a time.
///
/// - co_timer_0_start_0() starts the timer. After a successful start, its completion callback is posted to the given
///   executor exactly once, with #CO_RESULT_0_SUCCESS when the timer elapsed or #CO_RESULT_0_CANCELLED when it was
///   cancelled first. It is never invoked from within co_timer_0_start_0().
/// - A start is pending from its successful co_timer_0_start_0() until its completion callback is invoked. The timer
///   is idle when no start is pending. It may only be started when idle, for example again from within the completion
///   callback of its previous start.
/// - co_timer_0_cancel_0() requests cancellation of the pending start. It does nothing when no start is pending or the
///   timer has already elapsed; the outcome is reported only by the status passed to the completion callback.
/// - The timer may be destroyed when idle; in particular from within the completion callback of its last start.
/// - If the executor rejects a completion callback, the process is terminated.
/// - Starting and cancelling may be done from any thread, but the caller must ensure that the preconditions above hold,
///   for example by calling them only from handlers of one strand.
///
/// To stop using a timer while a start may be pending, cancel the start and destroy the timer from its completion
/// callback, which is invoked (with #CO_RESULT_0_SUCCESS or #CO_RESULT_0_CANCELLED) once the timer no longer uses it.
/// The owner must not start the timer again from that callback, for example by setting a flag that the callback checks.
///
/// @code
/// // A timer that ticks every 100 ms until it is stopped from a handler of t->strand_executor.
/// static void on_tick(void* data, co_result_0 status) CO_NOEXCEPT
/// {
///     ticker* t = (ticker*)data;
///     if (t->stopping) {
///         co_timer_0_destroy(t->timer); // idle: this was the last start
///         free(t);
///         return;
///     }
///     // ... do the periodic work ...
///     co_timer_0_start_0(t->timer, 100000000, t->strand_executor, &on_tick, t);
/// }
///
/// co_service_0 service;
/// co_result_0 result = co_event_facility_0_get_service_0(event_facility, CO_SERVICE_TYPE_0_TIMER_0_SERVICE_0, &service);
/// // ...
/// result = co_timer_0_service_0_create_timer_0(CO_TIMER_0_SERVICE_0_CAST(service), &t->timer);
/// // ...
/// result = co_timer_0_start_0(t->timer, 100000000, t->strand_executor, &on_tick, t);
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

/// @brief Completion callback of co_timer_0_start_0(), invoked exactly once per successful start.
/// @param data User data passed to co_timer_0_start_0().
/// @param status #CO_RESULT_0_SUCCESS when the timer elapsed, or #CO_RESULT_0_CANCELLED when co_timer_0_cancel_0() took
/// effect first.
typedef void (* co_timer_0_start_0_completion_fn)(void* data, co_result_0 status) CO_NOEXCEPT;

/// @brief Destroys a timer; does nothing when timer is `NULL`.
/// @pre The timer is idle: it was never started, or the completion callback of its last start has been invoked or is
/// being invoked. Destroying it from within that callback is allowed.
/// @pre No other call on the timer is in progress.
CO_API
void co_timer_0_destroy(co_timer_0 timer) CO_NOEXCEPT;

/// @brief Starts the timer.
///
/// On success, completion_fn is posted to executor exactly once, after the timer elapsed or was cancelled. On failure,
/// completion_fn is never invoked and the timer stays idle.
///
/// @param timer Timer to start.
/// @param due_ns Delay in nanoseconds after which the timer elapses, rounded up to the resolution of the timer class.
/// Zero makes the timer elapse as soon as possible.
/// @param executor Executor to which completion_fn is posted. Must stay valid until completion_fn has been invoked.
/// @param completion_fn Callback invoked when the timer elapsed or was cancelled.
/// @param data User data passed to completion_fn.
/// @retval CO_RESULT_0_SUCCESS The start is pending.
/// @retval CO_RESULT_0_ERROR_OUT_OF_MEMORY The timer cannot allocate its bookkeeping.
/// @pre The timer is idle (see co_timer_0_destroy()). Starting it again from within the completion callback of its
/// previous start is allowed.
/// @pre executor and completion_fn are not `NULL`.
CO_API
co_result_0 co_timer_0_start_0(co_timer_0 timer, uint64_t due_ns, co_executor_0 executor, co_timer_0_start_0_completion_fn completion_fn, void* data) CO_NOEXCEPT;

/// @brief Requests cancellation of the pending start.
///
/// If the request takes effect before the timer elapses, the completion callback of the start is invoked with
/// #CO_RESULT_0_CANCELLED. Does nothing and returns #CO_RESULT_0_SUCCESS when no start is pending or the timer has
/// already elapsed, for example while its completion callback is queued on the executor.
///
/// @param timer Timer whose pending start should be cancelled.
/// @retval CO_RESULT_0_SUCCESS The request was made, or there was nothing to cancel.
/// @return An error when the request could not be made; the start then stays pending and the request may be retried.
CO_API
co_result_0 co_timer_0_cancel_0(co_timer_0 timer) CO_NOEXCEPT;

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

/// @brief Type of co_timer_0_vtable::start_0.
typedef co_result_0 (* co_timer_0_start_0_fn)(co_timer_0 self, uint64_t due_ns, co_executor_0 executor, co_timer_0_start_0_completion_fn completion_fn, void* data) CO_NOEXCEPT;

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
	/// @brief Required. Implements co_timer_0_start_0().
	co_timer_0_start_0_fn start_0;
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
