#pragma once

#include <co.h>

#ifdef __cplusplus
extern "C"
{
#endif

/// @defgroup c_thread_count_bounds Thread count bounds
/// @brief Bounds of the number of threads of a thread pool whose number of threads varies.
/// @{

/// @brief Structure type of co_thread_count_bounds_info_0.
extern CO_API
const co_structure_type_0 CO_STRUCTURE_TYPE_0_THREAD_COUNT_BOUNDS_INFO_0;

/// @brief Bounds of the number of threads of the thread pool asked for by the next chain it is in.
///
/// Put it in the next chain of a co_event_facility_0_create_info_0 that also holds a co_dynamic_thread_pool_0_create_info_0,
/// before or after it. Like any next-chain entry, it narrows the choice to classes handling it.
///
/// The bounds are a requirement, not a hint: handlers that block until other handlers run deadlock when fewer threads
/// run than they need, so a class never runs fewer than minimum_thread_count nor more than maximum_thread_count threads.
/// A class that cannot honor the bounds fails with CO_RESULT_0_ERROR_NOT_SUPPORTED, and the instance tries the next
/// candidate class. A class whose thread pool has a fixed number of threads may accept equal bounds and run that
/// number of threads.
/// @pre The next chain also holds a co_dynamic_thread_pool_0_create_info_0, anywhere.
/// @pre The next chain holds no other co_thread_count_bounds_info_0.
typedef struct co_thread_count_bounds_info_0 CO_FINAL
{
	co_structure_type_0 structure_type;
	const co_in_structure_0* next_structure;

	/// @brief Minimum number of threads.
	size_t minimum_thread_count;
	/// @brief Maximum number of threads.
	/// @pre Not zero and not less than minimum_thread_count.
	size_t maximum_thread_count;
} co_thread_count_bounds_info_0;

/// @}

#ifdef __cplusplus
}
#endif
