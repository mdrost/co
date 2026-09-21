#pragma once

#include <co/fixed_thread_pool_0.h>

#include <co.hpp>

#include <cstdint>

namespace co
{

/// @defgroup cpp_fixed_thread_pool Fixed thread pool
/// @brief Event facilities running their handlers on a thread pool of a fixed number of threads they own.
///
/// A fixed thread pool is an abstract kind: there is no fixed_thread_pool_0 type. Creating one with a
/// fixed_thread_pool_0_create_info_0 yields an event_facility_0 of whichever class was chosen, which behaves as
/// described here.
/// @{

/// @brief Structure type of fixed_thread_pool_0_create_info_0.
extern CO_API
const structure_type_0 structure_type_0_fixed_thread_pool_0_create_info_0;

/// @brief Create info asking for a fixed thread pool, in the next chain of an event_facility_0_create_info_0.
/// See co_fixed_thread_pool_0_create_info_0.
struct fixed_thread_pool_0_create_info_0 final
{
	structure_type_0 structure_type;
	const in_structure_0* next_structure;

	/// @brief Number of threads of the thread pool.
	/// @pre Not zero.
	std::size_t thread_count;
};

/// @}

} // namespace co
