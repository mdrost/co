#pragma once

#include <co/thread_count_bounds_0.h>

#include <co.hpp>

#include <cstddef>

namespace co
{

/// @defgroup cpp_thread_count_bounds Thread count bounds
/// @brief Bounds of the number of threads of a thread pool whose number of threads varies.
/// @{

/// @brief Structure type of thread_count_bounds_info_0.
extern CO_API
const structure_type_0 structure_type_0_thread_count_bounds_info_0;

/// @brief Bounds of the number of threads, in a next chain of an event_facility_0_create_info_0 that also holds a
/// dynamic_thread_pool_0_create_info_0 (in any order).
/// A requirement, not a hint:
/// result_0::error_not_supported and the next candidate class is tried. See co_thread_count_bounds_info_0.
struct thread_count_bounds_info_0 final
{
	structure_type_0 structure_type;
	const in_structure_0* next_structure;

	/// @brief Minimum number of threads.
	std::size_t minimum_thread_count;
	/// @brief Maximum number of threads.
	/// @pre Not zero and not less than minimum_thread_count.
	std::size_t maximum_thread_count;
};

/// @}

} // namespace co
