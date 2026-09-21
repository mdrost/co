#pragma once

#include <co/dynamic_thread_pool_0.h>

#include <co.hpp>

namespace co
{

/// @defgroup cpp_dynamic_thread_pool Dynamic thread pool
/// @brief Event facilities running their handlers on a thread pool they own, whose number of threads varies.
///
/// A dynamic thread pool is an abstract kind: there is no dynamic_thread_pool_0 type. Creating one with a
/// dynamic_thread_pool_0_create_info_0 yields an event_facility_0 of whichever class was chosen, which behaves as
/// described here.
/// @{

/// @brief Structure type of dynamic_thread_pool_0_create_info_0.
extern CO_API
const structure_type_0 structure_type_0_dynamic_thread_pool_0_create_info_0;

/// @brief Create info asking for a dynamic thread pool, in the next chain of an event_facility_0_create_info_0.
///
/// The class chooses the bounds of the number of threads unless the next chain also holds a thread_count_bounds_info_0
/// (co/thread_count_bounds_0.hpp). See co_dynamic_thread_pool_0_create_info_0.
struct dynamic_thread_pool_0_create_info_0 final
{
	structure_type_0 structure_type;
	const in_structure_0* next_structure;
};

/// @}

} // namespace co
