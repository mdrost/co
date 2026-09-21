#pragma once

#include <co.h>

#ifdef __cplusplus
extern "C"
{
#endif

/// @defgroup c_time_watcher_0 Time watcher
/// @brief Watchers of changes of the system time, for example when the user or a time synchronization service sets it.
///
/// A time watcher does not measure time; it reports that the system time was set. Gradual adjustments of the system time
/// (slewing) are not changes. Changes of the time zone are not changes either: the system time is UTC.
///
/// Time watchers are created by a time watcher service with co_time_watcher_0_service_0_create_time_watcher_0(); the
/// service is found in an event domain that provides CO_SERVICE_TYPE_0_TIME_WATCHER_0_SERVICE_0.
///
/// - The time watcher counts the changes of the system time since its creation: their count, modulo SIZE_MAX + 1, is the
///   change generation, starting at 0. co_time_watcher_0_get_generation_0() gets it.
/// - co_time_watcher_0_wait_change_0() waits for a change after a given change generation, which must be one the time
///   watcher had: it completes once the change generation differs. If it already does, the wait completes right away;
///   otherwise with the next change. Generations are compared modulo SIZE_MAX + 1 (with !=), so they stay valid when
///   they wrap around.
/// - Consumers that depend on the system time, such as a clock following it, read the change generation, then the system
///   time, act on it, and wait after that generation: several changes before they get to read the system time again
///   are seen as one, and none is missed between reading and waiting.
/// - Any number of waits may be pending, for one consumer or for several consumers sharing the time watcher; each
///   consumer keeps the generation it saw last.
/// - After a successful wait, its completion callback is posted to the given executor exactly once, with
///   #CO_RESULT_0_SUCCESS when the system time changed or #CO_RESULT_0_CANCELLED when the wait was cancelled first. It
///   is never invoked from within co_time_watcher_0_wait_change_0(). If the executor rejects a completion callback
///   after the wait call returned, the process is terminated.
/// - A wait is pending from its successful wait call until its completion callback is invoked. The time watcher may be
///   destroyed when no wait is pending; in particular from within the completion callback of its last wait.
/// - The functions of a time watcher other than co_time_watcher_0_destroy() may be called from any thread,
///   concurrently.
///
/// @code
/// // Re-reads the system time whenever it is set.
/// static void on_change(void* data, co_result_0 status, size_t generation) CO_NOEXCEPT
/// {
///     consumer* c = (consumer*)data;
///     if (status == CO_RESULT_0_CANCELLED) {
///         // ... the consumer is stopping ...
///         return;
///     }
///     // ... read the system time and act on it ...
///     co_time_watcher_0_wait_change_0(c->time_watcher, generation, c->cancellation, c->executor, &on_change, c);
/// }
/// @endcode
/// @{

// Defined in the Time watcher implementation section: time watcher classes start their objects with it.
typedef struct co_time_watcher_0_t co_time_watcher_0_t;
/// @brief Handle of a time watcher.
typedef struct co_time_watcher_0_t* co_time_watcher_0;

/// @brief Casts a handle to co_time_watcher_0, unchecked; `NULL` stays `NULL`. See "Handles and casts" in Concepts.
#define CO_TIME_WATCHER_0_CAST(handle) ((co_time_watcher_0)(handle))

/// @brief Completion callback of co_time_watcher_0_wait_change_0(), invoked exactly once per successful wait.
/// @param data User data passed to co_time_watcher_0_wait_change_0().
/// @param status #CO_RESULT_0_SUCCESS when the system time changed, or #CO_RESULT_0_CANCELLED when the wait was
/// cancelled first.
/// @param generation The change generation of the time watcher at that point: the generation of the change that
/// completed the wait, or the unchanged generation the wait was given when cancelled.
typedef void (* co_time_watcher_0_wait_change_0_completion_fn)(void* data, co_result_0 status, size_t generation) CO_NOEXCEPT;

/// @brief Destroys a time watcher; does nothing when time_watcher is `NULL`.
/// @pre No wait of the time watcher is pending, and no other call on it is in progress.
CO_API
void co_time_watcher_0_destroy(co_time_watcher_0 time_watcher) CO_NOEXCEPT;

/// @brief Gets the change generation: the number of changes of the system time since the creation of the time watcher,
/// modulo SIZE_MAX + 1.
CO_API
void co_time_watcher_0_get_generation_0(co_time_watcher_0 time_watcher, size_t* generation) CO_NOEXCEPT;

/// @brief Waits for a change of the system time after the given change generation.
///
/// On success, completion_fn is posted to executor exactly once. On failure, completion_fn is never invoked.
///
/// @param after_generation The wait completes once the change generation differs: right away if it already does,
/// otherwise with the next change. Usually 0, the result of co_time_watcher_0_get_generation_0(), or the generation
/// passed to the completion callback of the previous wait.
/// @param cancellation Cancellation whose request cancels the wait, or `NULL`. When it is already requested, the wait
/// completes with #CO_RESULT_0_CANCELLED. Must stay valid until completion_fn has been invoked.
/// @param executor Executor to which completion_fn is posted. Must stay valid until completion_fn has been invoked.
/// @param completion_fn Callback invoked when the system time changed or the wait was cancelled.
/// @param data User data passed to completion_fn.
/// @retval CO_RESULT_0_SUCCESS The wait is pending.
/// @retval CO_RESULT_0_ERROR_OUT_OF_MEMORY The time watcher cannot allocate its bookkeeping.
/// @return The error of co_executor_0_post_0() when the wait completes right away and executor rejects completion_fn.
/// @pre after_generation is a change generation the time watcher had (it is never ahead of the change generation).
/// @pre executor and completion_fn are not `NULL`.
CO_API
co_result_0 co_time_watcher_0_wait_change_0(co_time_watcher_0 time_watcher, size_t after_generation, co_cancellation_0 cancellation, co_executor_0 executor, co_time_watcher_0_wait_change_0_completion_fn completion_fn, void* data) CO_NOEXCEPT;

/// @brief Cancels every pending wait of the time watcher, of every consumer sharing it: their completion callbacks are
/// invoked with #CO_RESULT_0_CANCELLED. To cancel the waits of one consumer only, use the cancellation passed to its
/// waits. The change generation is not affected. Does nothing when no wait is pending.
/// @retval CO_RESULT_0_SUCCESS The pending waits were cancelled, or there was nothing to cancel.
CO_API
co_result_0 co_time_watcher_0_cancel_0(co_time_watcher_0 time_watcher) CO_NOEXCEPT;

/// @defgroup c_time_watcher_0_impl Time watcher implementation
/// @ingroup c_time_watcher_0
/// @brief Declarations used to implement time watcher classes.
///
/// A time watcher object starts with a co_time_watcher_0_t, which the class initializes with co_time_watcher_0_init()
/// before the object is used. Methods are only ever appended to co_time_watcher_0_vtable, as described for
/// co_executor_0_vtable.
/// @{

/// @brief Type of co_time_watcher_0_vtable::destroy.
typedef void (* co_time_watcher_0_destroy_fn)(co_time_watcher_0 self) CO_NOEXCEPT;

/// @brief Type of co_time_watcher_0_vtable::get_generation_0.
typedef void (* co_time_watcher_0_get_generation_0_fn)(co_time_watcher_0 self, size_t* generation) CO_NOEXCEPT;

/// @brief Type of co_time_watcher_0_vtable::wait_change_0.
typedef co_result_0 (* co_time_watcher_0_wait_change_0_fn)(co_time_watcher_0 self, size_t after_generation, co_cancellation_0 cancellation, co_executor_0 executor, co_time_watcher_0_wait_change_0_completion_fn completion_fn, void* data) CO_NOEXCEPT;

/// @brief Type of co_time_watcher_0_vtable::cancel_0.
typedef co_result_0 (* co_time_watcher_0_cancel_0_fn)(co_time_watcher_0 self) CO_NOEXCEPT;

typedef struct co_time_watcher_0_vtable co_time_watcher_0_vtable;

/// @brief Start of every time watcher object.
struct co_time_watcher_0_t
{
	/// @brief Methods of the class of the object.
	const co_time_watcher_0_vtable* vtable;
};

/// @brief Methods of a time watcher class. Must stay valid while objects of the class exist.
struct co_time_watcher_0_vtable CO_FINAL
{
	/// @brief API version of the headers the class is compiled against; usually CO_API_VERSION_0.
	co_version_0 api_version;
	/// @brief Required. Implements co_time_watcher_0_destroy(): destroys the object and deallocates its memory.
	co_time_watcher_0_destroy_fn destroy;
	/// @brief Required. Implements co_time_watcher_0_get_generation_0().
	co_time_watcher_0_get_generation_0_fn get_generation_0;
	/// @brief Required. Implements co_time_watcher_0_wait_change_0().
	co_time_watcher_0_wait_change_0_fn wait_change_0;
	/// @brief Required. Implements co_time_watcher_0_cancel_0().
	co_time_watcher_0_cancel_0_fn cancel_0;
};

/// @brief Initializes the start of a time watcher object: stores its vtable.
/// @pre time_watcher_vtable has every required method and stays valid while the object exists.
static inline void co_time_watcher_0_init(co_time_watcher_0 time_watcher, const co_time_watcher_0_vtable* time_watcher_vtable) CO_NOEXCEPT
{
	time_watcher->vtable = time_watcher_vtable;
}

/// @}

/// @}

#ifdef __cplusplus
}
#endif
