#pragma once

#include <co.h>

#ifdef __cplusplus
extern "C"
{
#endif

// Suspend watchers are created by a suspend watcher service with co_suspend_watcher_0_service_0_create_suspend_watcher_0();
// the service is found in an event domain that provides CO_SERVICE_TYPE_0_SUSPEND_WATCHER_0_SERVICE_0.
//
// - co_suspend_watcher_0_wait_suspend_0() waits for a suspend of the system, co_suspend_watcher_0_wait_resume_0() for a
//   resume. Both kinds of waits are independent: suspends and resumes are not reported in pairs, and either may be
//   missed by the system.
// - The suspend watcher counts the suspends and the resumes that happened since its creation: their counts, modulo
//   SIZE_MAX + 1, are the suspend generation and the resume generation, both starting at 0.
// - A wait is given the generation after which it waits, which must be one the generation of its kind had: it completes
//   once the generation of its kind differs. If it already does, the wait completes right away; otherwise with the next
//   event of its kind. The generation of the event is passed to the completion callback, so that a consumer waits for the
//   following events by passing it to its next wait, missing none. The number of events between two generations is
//   their difference modulo SIZE_MAX + 1 (the unsigned difference), so generations stay valid when they wrap around;
//   only a consumer SIZE_MAX + 1 or more events behind would miss events.
// - Any number of waits of each kind may be pending, for one consumer or for several consumers sharing the suspend
//   watcher; each consumer keeps the generation it saw last. A consumer that only cares about events from now on starts
//   from co_suspend_watcher_0_get_suspend_generation_0() or co_suspend_watcher_0_get_resume_generation_0(); one passing 0
//   also sees the events since the creation of the suspend watcher.
// - After a successful wait, its completion callback is posted to the given executor exactly once, with
//   CO_RESULT_0_SUCCESS when the event happened or CO_RESULT_0_CANCELLED when the wait was cancelled first. It is never
//   invoked from within the wait call. If the executor rejects a completion callback after the wait call returned, the
//   process is terminated.
// - A wait is pending from its successful wait call until its completion callback is invoked. The suspend watcher may be
//   destroyed when no wait is pending; in particular from within the completion callback of its last wait.
// - The completion callbacks of the waits completed by an event are posted before those completed by any later event.
//   So when a suspend wait and a resume wait are pending at the same time and post to the same strand (see strand_0.h),
//   which invokes handlers in posting order, the resume completion callback is invoked after the suspend completion
//   callback whenever the suspend is announced. This does not hold for a wait completing right away because its
//   generation is behind: its completion callback is posted by the wait call.

typedef struct co_suspend_watcher_0_t co_suspend_watcher_0_t;
typedef struct co_suspend_watcher_0_t* co_suspend_watcher_0;

#define CO_SUSPEND_WATCHER_0_CAST(handle) ((co_suspend_watcher_0)(handle))

// Completion callback of co_suspend_watcher_0_wait_suspend_0(), invoked exactly once per successful wait with
// CO_RESULT_0_SUCCESS or CO_RESULT_0_CANCELLED, and the suspend generation of the suspend watcher at that point: the
// generation of the suspend that completed the wait, or the unchanged generation the wait was given when cancelled.
typedef void (*co_suspend_watcher_0_wait_suspend_0_completion_fn)(void* data, co_result_0 status, size_t generation) CO_NOEXCEPT;

// Completion callback of co_suspend_watcher_0_wait_resume_0(); same as co_suspend_watcher_0_wait_suspend_0_completion_fn
// with the resume generation.
typedef void (*co_suspend_watcher_0_wait_resume_0_completion_fn)(void* data, co_result_0 status, size_t generation) CO_NOEXCEPT;

// Destroys the suspend watcher; does nothing when suspend_watcher is NULL.
// @pre No wait of the suspend watcher is pending, and no other call on it is in progress.
CO_API
void co_suspend_watcher_0_destroy(co_suspend_watcher_0 suspend_watcher) CO_NOEXCEPT;

// Gets the suspend generation: the number of suspends since the creation of the suspend watcher, modulo SIZE_MAX + 1.
CO_API
void co_suspend_watcher_0_get_suspend_generation_0(co_suspend_watcher_0 suspend_watcher, size_t* generation) CO_NOEXCEPT;

// Gets the resume generation: the number of resumes since the creation of the suspend watcher, modulo SIZE_MAX + 1.
CO_API
void co_suspend_watcher_0_get_resume_generation_0(co_suspend_watcher_0 suspend_watcher, size_t* generation) CO_NOEXCEPT;

// Waits for a suspend of the system after the given suspend generation.
//
// On success, completion_fn is posted to executor exactly once. On failure, completion_fn is never invoked.
//
// @param after_generation The wait completes once the suspend generation differs: right away if it already does,
// otherwise with the next suspend. Usually 0, the result of co_suspend_watcher_0_get_suspend_generation_0(), or the
// generation passed to the completion callback of the previous wait.
// @param cancellation Cancellation whose request cancels the wait, or NULL. When it is already requested, the wait
// completes with CO_RESULT_0_CANCELLED. Must stay valid until completion_fn has been invoked.
// @param executor Executor to which completion_fn is posted. Must stay valid until completion_fn has been invoked.
// @retval CO_RESULT_0_SUCCESS The wait is pending.
// @retval CO_RESULT_0_ERROR_OUT_OF_MEMORY The suspend watcher cannot allocate its bookkeeping.
// @return The error of co_executor_0_post_0() when the wait completes right away and executor rejects completion_fn.
// @pre after_generation is a suspend generation the suspend watcher had (it is never ahead of the suspend generation).
// @pre executor and completion_fn are not NULL.
CO_API
co_result_0 co_suspend_watcher_0_wait_suspend_0(co_suspend_watcher_0 suspend_watcher, size_t after_generation, co_cancellation_0 cancellation, co_executor_0 executor, co_suspend_watcher_0_wait_suspend_0_completion_fn completion_fn, void* data) CO_NOEXCEPT;

// Waits for a resume of the system after the given resume generation. Same as co_suspend_watcher_0_wait_suspend_0()
// otherwise.
CO_API
co_result_0 co_suspend_watcher_0_wait_resume_0(co_suspend_watcher_0 suspend_watcher, size_t after_generation, co_cancellation_0 cancellation, co_executor_0 executor, co_suspend_watcher_0_wait_resume_0_completion_fn completion_fn, void* data) CO_NOEXCEPT;

// Cancels every pending wait of the suspend watcher, of both kinds and of every consumer sharing it: their completion
// callbacks are invoked with CO_RESULT_0_CANCELLED. To cancel the waits of one consumer only, use the cancellation passed
// to its waits. Waits of other suspend watchers and the generations are not affected. Does nothing when no wait is
// pending.
// @retval CO_RESULT_0_SUCCESS The pending waits were cancelled, or there was nothing to cancel.
CO_API
co_result_0 co_suspend_watcher_0_cancel_0(co_suspend_watcher_0 suspend_watcher) CO_NOEXCEPT;

typedef void (* co_suspend_watcher_0_destroy_fn)(co_suspend_watcher_0 self) CO_NOEXCEPT;

typedef void (* co_suspend_watcher_0_get_suspend_generation_0_fn)(co_suspend_watcher_0 self, size_t* generation) CO_NOEXCEPT;

typedef void (* co_suspend_watcher_0_get_resume_generation_0_fn)(co_suspend_watcher_0 self, size_t* generation) CO_NOEXCEPT;

typedef co_result_0 (* co_suspend_watcher_0_wait_suspend_0_fn)(co_suspend_watcher_0 self, size_t after_generation, co_cancellation_0 cancellation, co_executor_0 executor, co_suspend_watcher_0_wait_suspend_0_completion_fn completion_fn, void* data) CO_NOEXCEPT;

typedef co_result_0 (* co_suspend_watcher_0_wait_resume_0_fn)(co_suspend_watcher_0 self, size_t after_generation, co_cancellation_0 cancellation, co_executor_0 executor, co_suspend_watcher_0_wait_resume_0_completion_fn completion_fn, void* data) CO_NOEXCEPT;

typedef co_result_0 (* co_suspend_watcher_0_cancel_0_fn)(co_suspend_watcher_0 self) CO_NOEXCEPT;

typedef struct co_suspend_watcher_0_vtable co_suspend_watcher_0_vtable;

struct co_suspend_watcher_0_t
{
	const co_suspend_watcher_0_vtable* vtable;
};

struct co_suspend_watcher_0_vtable CO_FINAL
{
	co_version_0 api_version;
	co_suspend_watcher_0_destroy_fn destroy;
	co_suspend_watcher_0_get_suspend_generation_0_fn get_suspend_generation_0;
	co_suspend_watcher_0_get_resume_generation_0_fn get_resume_generation_0;
	co_suspend_watcher_0_wait_suspend_0_fn wait_suspend_0;
	co_suspend_watcher_0_wait_resume_0_fn wait_resume_0;
	co_suspend_watcher_0_cancel_0_fn cancel_0;
};

static inline void co_suspend_watcher_0_init(co_suspend_watcher_0 suspend_watcher, const co_suspend_watcher_0_vtable* suspend_watcher_vtable) CO_NOEXCEPT
{
	suspend_watcher->vtable = suspend_watcher_vtable;
}

#ifdef __cplusplus
}
#endif