#pragma once

#include <co.h>

#ifdef __cplusplus
extern "C"
{
#endif

/// @defgroup c_platform_thread_pool Platform thread pool
/// @brief Event facilities adapting the thread pool of the platform, the runtime the application runs on.
///
/// A platform thread pool is an abstract kind: there is no co_platform_thread_pool_0 handle. Creating one with a
/// co_platform_thread_pool_0_create_info_0 yields a co_event_facility_0 of whichever class was chosen, which behaves as
/// described here.
/// @{

/// @brief Structure type of co_platform_thread_pool_0_create_info_0.
extern CO_API
const co_structure_type_0 CO_STRUCTURE_TYPE_0_PLATFORM_THREAD_POOL_0_CREATE_INFO_0;

/// @brief Create info asking for an event facility adapting the thread pool of the platform, the runtime the application runs on,
/// whose existence the application does not control.
///
/// The platform is the operating system (for example the default process thread pool on Windows) or an application
/// framework (for example the global thread pool of Qt). Put it in the next chain of a co_event_facility_0_create_info_0.
/// Event domain group classes declare it by listing its structure type among the handled types of their factory, and the
/// class is chosen as for any create info (see co_instance_0_create_event_domain_0()): by the required services and
/// the other entries of the next chain, then by rank, then the last registered one. A framework module registered after
/// the built-in classes thus replaces the operating system thread pool wherever it provides the required services. To ask
/// for a given platform, use the create info of its class instead, for example
/// co_win32_platform_thread_pool_adapter_0_event_facility_0_create_info_0.
///
/// @pre The next chain holds neither co_fixed_thread_pool_0_create_info_0 nor co_dynamic_thread_pool_0_create_info_0.
typedef struct co_platform_thread_pool_0_create_info_0 CO_FINAL
{
	co_structure_type_0 structure_type;
	const co_in_structure_0* next_structure;
} co_platform_thread_pool_0_create_info_0;

/// @}

#ifdef __cplusplus
}
#endif
