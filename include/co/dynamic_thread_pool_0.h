#pragma once

#include <co.h>

#ifdef __cplusplus
extern "C"
{
#endif

/// @defgroup c_dynamic_thread_pool Dynamic thread pool
/// @brief Event facilities running their handlers on a thread pool they own, whose number of threads varies.
///
/// A dynamic thread pool is an abstract kind: there is no co_dynamic_thread_pool_0 handle. Creating one with a
/// co_dynamic_thread_pool_0_create_info_0 yields a co_event_facility_0 of whichever class was chosen, which behaves as
/// described here.
/// @{

/// @brief Structure type of co_dynamic_thread_pool_0_create_info_0.
extern CO_API
const co_structure_type_0 CO_STRUCTURE_TYPE_0_DYNAMIC_THREAD_POOL_0_CREATE_INFO_0;

/// @brief Create info asking for an event facility running its handlers on a thread pool it owns, whose number of threads varies.
///
/// Put it in the next chain of a co_event_facility_0_create_info_0. Names no class: the class is chosen as for any
/// create info (see co_instance_0_create_event_domain_0()). The class chooses the bounds of the number of threads unless
/// the next chain also holds a co_thread_count_bounds_info_0 (co/thread_count_bounds_0.h).
/// @pre The next chain holds neither co_platform_thread_pool_0_create_info_0 nor
/// co_fixed_thread_pool_0_create_info_0.
typedef struct co_dynamic_thread_pool_0_create_info_0 CO_FINAL
{
	co_structure_type_0 structure_type;
	const co_in_structure_0* next_structure;
} co_dynamic_thread_pool_0_create_info_0;

/// @}

#ifdef __cplusplus
}
#endif
