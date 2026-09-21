#pragma once

#include <co.h>

#ifdef __cplusplus
extern "C"
{
#endif

/// @defgroup c_strand_0_executor_0 Strand executor
/// @brief Executors that invoke posted handlers one at a time, in posting order, on top of another executor.
///
/// A strand is a built-in executor class. Create it with co_strand_0_executor_0_create_0(), post to it with
/// co_strand_0_executor_0_post_0() and destroy it with co_strand_0_executor_0_destroy(). As a strand is an executor, a
/// handle cast with CO_EXECUTOR_0_CAST may also be used wherever a co_executor_0 is expected.
///
/// - Handlers are invoked one at a time and in the order in which they were posted. A handler has finished its
///   invocation, which also releases what it owns, before the next one starts, so state accessed only from handlers
///   of the same strand needs no locking.
/// - Each handler is invoked by its own handler (job) of the inner executor, so it runs exactly in the context the inner
///   executor provides. Consecutive handlers may run on different threads of the inner executor.
/// - While it has handlers, the strand has exactly one job posted to, or running on, the inner executor. The job posts
///   the job for the next handler once its own handler has been invoked.
/// - co_strand_0_executor_0_post_0() never invokes the handler inline; it may be called from any thread, including from
///   a handler of the strand.
/// - If the strand has no job and the inner executor rejects the new one, co_strand_0_executor_0_post_0() returns that
///   error and ownership of the handler data stays with the caller.
/// - If the inner executor rejects the job for the next queued handler, the process is terminated.
/// - The inner executor must not invoke handlers inline from within co_executor_0_post_0().
///
/// The strand is idle when it has no queued handler and the inner executor holds or runs no job of the strand, for
/// example after the inner executor has invoked all of its handlers. The strand may be destroyed when it is idle, or
/// from within the invocation of one of its handlers when no other handler is queued, for example by the last handler
/// of an object owning the strand. The job then returns without accessing the strand. No co_strand_0_executor_0_post_0()
/// call on the strand may be in progress when it is destroyed.
///
/// @code
/// // The last handler of a connection destroys the strand of the connection.
/// static void close_connection(void* data) CO_NOEXCEPT
/// {
///     connection* c = (connection*)data;
///     co_strand_0_executor_0_destroy(c->strand);
///     free(c);
/// }
///
/// co_strand_0_executor_0_create_info_0 create_info = {
///     .structure_type = CO_STRUCTURE_TYPE_0_STRAND_0_EXECUTOR_0_CREATE_INFO_0,
///     .next_structure = NULL,
///     .inner_executor = thread_pool_executor,
/// };
/// co_result_0 result = co_strand_0_executor_0_create_0(instance, &create_info, &c->strand);
/// // ...
/// co_strand_0_executor_0_post_0(c->strand, (co_executor_handler_0){.invoke_fn = &close_connection, .data = c});
/// @endcode
/// @{

/// @brief Structure type of co_strand_0_executor_0_create_info_0.
extern CO_API
const co_structure_type_0 CO_STRUCTURE_TYPE_0_STRAND_0_EXECUTOR_0_CREATE_INFO_0;

/// @brief Handle of a strand executor. A strand is an executor: a co_strand_0_executor_0 may be cast to co_executor_0
/// with CO_EXECUTOR_0_CAST, and a co_executor_0 of a strand back with CO_STRAND_0_EXECUTOR_0_CAST.
typedef struct co_strand_0_executor_0_t co_strand_0_executor_0_t; // private; do not use
typedef struct co_strand_0_executor_0_t* co_strand_0_executor_0;

/// @brief Casts a handle to co_strand_0_executor_0, unchecked; `NULL` stays `NULL`. A downcast requires the object to be
/// a strand. See "Handles and casts" in Concepts.
#define CO_STRAND_0_EXECUTOR_0_CAST(handle) ((co_strand_0_executor_0)(handle))

/// @brief Parameters of a strand executor.
typedef struct co_strand_0_executor_0_create_info_0 CO_FINAL
{
	/// @brief Identifies the structure; set to #CO_STRUCTURE_TYPE_0_STRAND_0_EXECUTOR_0_CREATE_INFO_0.
	co_structure_type_0 structure_type;

	/// @brief Extension chain; may hold a co_allocation_callbacks_0.
	const co_in_structure_0* next_structure;

	/// @brief Executor on which the handlers are invoked. Must stay valid while the strand is not idle.
	/// @pre Not `NULL`.
	co_executor_0 inner_executor;
} co_strand_0_executor_0_create_info_0;

/// @brief Creates a strand executor; *strand is written only on success.
/// Returns #CO_RESULT_0_ERROR_NOT_SUPPORTED for an unsupported next chain entry and #CO_RESULT_0_ERROR_OUT_OF_MEMORY when
/// the strand cannot be allocated.
/// @pre create_info is not `NULL`.
/// @pre strand is not `NULL`.
CO_API
co_result_0 co_strand_0_executor_0_create_0(co_instance_0 instance, const co_strand_0_executor_0_create_info_0* create_info, co_strand_0_executor_0* strand) CO_NOEXCEPT;

/// @brief Destroys a strand; does nothing when strand is `NULL`. Same as co_executor_0_destroy() on the strand, which it
/// calls inline.
/// @pre The strand is idle, or this is called from within the invocation of its last handler (see above).
static inline void co_strand_0_executor_0_destroy(co_strand_0_executor_0 strand) CO_NOEXCEPT
{
	co_executor_0_destroy(CO_EXECUTOR_0_CAST(strand));
}

/// @brief Posts a handler to a strand. Same as co_executor_0_post_0() on the strand, which it calls inline.
/// On success the strand invokes handler exactly once, after the handlers posted before it. On failure handler is
/// never invoked and ownership of its data stays with the caller.
static inline co_result_0 co_strand_0_executor_0_post_0(co_strand_0_executor_0 strand, co_executor_handler_0 handler) CO_NOEXCEPT
{
	return co_executor_0_post_0(CO_EXECUTOR_0_CAST(strand), handler);
}

/// @}

#ifdef __cplusplus
}
#endif
