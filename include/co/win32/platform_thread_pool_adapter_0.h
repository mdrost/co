#pragma once

#include <co.h>

#ifdef __cplusplus
extern "C"
{
#endif

/// @defgroup c_win32_platform_thread_pool_adapter_0 Win32 platform thread pool adapter
/// @brief Event facility, executor and event domain group factory classes on top of the legacy Win32 thread pool.
/// @{

#pragma region Handles

typedef struct co_win32_platform_thread_pool_adapter_0_event_domain_group_factory_0_t co_win32_platform_thread_pool_adapter_0_event_domain_group_factory_0_t;
/// @brief Handle of a Win32 platform thread pool adapter event domain group factory; may be cast to
/// co_event_domain_group_factory_0 with CO_EVENT_DOMAIN_GROUP_FACTORY_0_CAST.
typedef co_win32_platform_thread_pool_adapter_0_event_domain_group_factory_0_t* co_win32_platform_thread_pool_adapter_0_event_domain_group_factory_0;

typedef struct co_win32_platform_thread_pool_adapter_0_event_domain_group_0_t co_win32_platform_thread_pool_adapter_0_event_domain_group_0_t;
/// @brief Handle of a Win32 platform thread pool adapter event domain group; may be cast to
/// co_event_domain_group_0 with CO_EVENT_DOMAIN_GROUP_0_CAST.
typedef co_win32_platform_thread_pool_adapter_0_event_domain_group_0_t* co_win32_platform_thread_pool_adapter_0_event_domain_group_0;

typedef struct co_win32_platform_thread_pool_adapter_0_event_facility_0_t co_win32_platform_thread_pool_adapter_0_event_facility_0_t;
/// @brief Handle of a Win32 platform thread pool adapter event facility; may be cast to
/// co_event_facility_0 with CO_EVENT_FACILITY_0_CAST.
typedef co_win32_platform_thread_pool_adapter_0_event_facility_0_t* co_win32_platform_thread_pool_adapter_0_event_facility_0;

#pragma endregion

#pragma region Event domain group factory

#define CO_WIN32_PLATFORM_THREAD_POOL_ADAPTER_0_EVENT_DOMAIN_GROUP_FACTORY_0_CAST(handle) ((co_win32_platform_thread_pool_adapter_0_event_domain_group_factory_0)(handle))

/// @brief Create info of a Win32 platform thread pool adapter event domain group factory.
///
/// The next chain holds exactly one co_allocation_callbacks_0, which allocate the factory, and may hold
/// co_debug_callback_0 entries; an entry point may pass on the next chain of its info.
typedef struct co_win32_platform_thread_pool_adapter_0_event_domain_group_factory_0_create_info_0 CO_FINAL
{
	co_structure_type_0 structure_type;
	const co_in_structure_0* next_structure;
} co_win32_platform_thread_pool_adapter_0_event_domain_group_factory_0_create_info_0;

/*
/// @brief Creates a Win32 platform thread pool adapter event domain group factory.
/// @pre create_info is not NULL.
CO_API
co_result_0 co_win32_platform_thread_pool_adapter_0_event_domain_group_factory_0_create_0(const co_win32_platform_thread_pool_adapter_0_event_domain_group_factory_0_create_info_0* create_info, co_win32_platform_thread_pool_adapter_0_event_domain_group_factory_0* event_domain_group_factory) CO_NOEXCEPT;
*/

/// @copydoc co_event_domain_group_factory_0_destroy()
static inline void co_win32_platform_thread_pool_adapter_0_event_domain_group_factory_0_destroy(co_win32_platform_thread_pool_adapter_0_event_domain_group_factory_0 event_domain_group_factory) CO_NOEXCEPT
{
	co_event_domain_group_factory_0_destroy(CO_EVENT_DOMAIN_GROUP_FACTORY_0_CAST(event_domain_group_factory));
}

/// @copydoc co_event_domain_group_factory_0_get_member_create_info_structure_types_0()
static inline void co_win32_platform_thread_pool_adapter_0_event_domain_group_factory_0_get_member_create_info_structure_types_0(co_win32_platform_thread_pool_adapter_0_event_domain_group_factory_0 event_domain_group_factory, const co_structure_type_0** member_create_info_structure_types, size_t* member_create_info_structure_type_count) CO_NOEXCEPT
{
	co_event_domain_group_factory_0_get_member_create_info_structure_types_0(CO_EVENT_DOMAIN_GROUP_FACTORY_0_CAST(event_domain_group_factory), member_create_info_structure_types, member_create_info_structure_type_count);
}

/// @copydoc co_event_domain_group_factory_0_get_handled_structure_types_0()
static inline void co_win32_platform_thread_pool_adapter_0_event_domain_group_factory_0_get_handled_structure_types_0(co_win32_platform_thread_pool_adapter_0_event_domain_group_factory_0 event_domain_group_factory, const co_structure_type_0** handled_structure_types, size_t* handled_structure_type_count) CO_NOEXCEPT
{
	co_event_domain_group_factory_0_get_handled_structure_types_0(CO_EVENT_DOMAIN_GROUP_FACTORY_0_CAST(event_domain_group_factory), handled_structure_types, handled_structure_type_count);
}

/// @copydoc co_event_domain_group_factory_0_get_provided_service_types_0()
static inline void co_win32_platform_thread_pool_adapter_0_event_domain_group_factory_0_get_provided_service_types_0(co_win32_platform_thread_pool_adapter_0_event_domain_group_factory_0 event_domain_group_factory, const co_service_type_0** provided_service_types, size_t* provided_service_type_count) CO_NOEXCEPT
{
	co_event_domain_group_factory_0_get_provided_service_types_0(CO_EVENT_DOMAIN_GROUP_FACTORY_0_CAST(event_domain_group_factory), provided_service_types, provided_service_type_count);
}

/// @copydoc co_event_domain_group_factory_0_get_member_count_range_0()
static inline void co_win32_platform_thread_pool_adapter_0_event_domain_group_factory_0_get_member_count_range_0(co_win32_platform_thread_pool_adapter_0_event_domain_group_factory_0 event_domain_group_factory, size_t* min_count, size_t* max_count) CO_NOEXCEPT
{
	co_event_domain_group_factory_0_get_member_count_range_0(CO_EVENT_DOMAIN_GROUP_FACTORY_0_CAST(event_domain_group_factory), min_count, max_count);
}

/// @copydoc co_event_domain_group_factory_0_get_rank_0()
static inline void co_win32_platform_thread_pool_adapter_0_event_domain_group_factory_0_get_rank_0(co_win32_platform_thread_pool_adapter_0_event_domain_group_factory_0 event_domain_group_factory, size_t* rank) CO_NOEXCEPT
{
	co_event_domain_group_factory_0_get_rank_0(CO_EVENT_DOMAIN_GROUP_FACTORY_0_CAST(event_domain_group_factory), rank);
}

/// @copydoc co_event_domain_group_factory_0_create_event_domain_group_0()
static inline co_result_0 co_win32_platform_thread_pool_adapter_0_event_domain_group_factory_0_create_event_domain_group_0(co_win32_platform_thread_pool_adapter_0_event_domain_group_factory_0 event_domain_group_factory, const co_event_domain_group_0_create_info_0* create_info, co_event_domain_group_0* group) CO_NOEXCEPT
{
	return co_event_domain_group_factory_0_create_event_domain_group_0(CO_EVENT_DOMAIN_GROUP_FACTORY_0_CAST(event_domain_group_factory), create_info, group);
}

#pragma endregion

#pragma region Event domain group

#define CO_WIN32_PLATFORM_THREAD_POOL_ADAPTER_0_EVENT_DOMAIN_GROUP_0_CAST(handle) ((co_win32_platform_thread_pool_adapter_0_event_domain_group_0)(handle))

/*
CO_API
co_result_0 co_win32_platform_thread_pool_adapter_0_event_domain_group_0_create_0() CO_NOEXCEPT;
*/

/// @copydoc co_event_domain_group_0_destroy()
static inline void co_win32_platform_thread_pool_adapter_0_event_domain_group_0_destroy(co_win32_platform_thread_pool_adapter_0_event_domain_group_0 event_domain_group) CO_NOEXCEPT
{
	co_event_domain_group_0_destroy(CO_EVENT_DOMAIN_GROUP_0_CAST(event_domain_group));
}

/// @copydoc co_event_domain_group_0_get_event_domain_0()
static inline void co_win32_platform_thread_pool_adapter_0_event_domain_group_0_get_event_domain_0(co_event_domain_group_0 event_domain_group, size_t domain_event_index, co_event_domain_0* event_domain_0) CO_NOEXCEPT
{
	co_event_domain_group_0_get_event_domain_0(CO_EVENT_DOMAIN_GROUP_0_CAST(event_domain_group), domain_event_index, event_domain_0);
}

#pragma endregion

#pragma region Event facility

/// @brief Structure type of co_win32_platform_thread_pool_adapter_0_event_facility_0_create_info_0.
extern CO_API
const co_structure_type_0 CO_STRUCTURE_TYPE_0_WIN32_PLATFORM_THREAD_POOL_ADAPTER_0_EVENT_FACILITY_0_CREATE_INFO_0;

/// @brief Create info of a Win32 platform thread pool adapter event facility; starts with co_event_domain_0_create_info_0.
///
/// The Win32 platform thread pool adapter event facility is a built-in event facility class providing a timer service
/// (CO_SERVICE_TYPE_0_TIMER_0_SERVICE_0). Create it with co_instance_0_create_event_domain_0() from this create info. The class
/// also accepts a generic co_event_facility_0_create_info_0 with a co_platform_thread_pool_0_create_info_0 in
/// its next chain. Creation fails with
/// CO_RESULT_0_ERROR_NOT_SUPPORTED when the legacy thread pool functions are not available.
typedef struct co_win32_platform_thread_pool_adapter_0_event_facility_0_create_info_0 CO_FINAL
{
	co_structure_type_0 structure_type;
	const co_in_structure_0* next_structure;

	/// @brief Services the event facility must provide; only these may be asked for with co_event_domain_0_get_service_0().
	/// @pre Not NULL when required_service_count is not zero.
	const co_service_type_0* required_services;
	/// @brief Number of elements of required_services.
	size_t required_service_count;
} co_win32_platform_thread_pool_adapter_0_event_facility_0_create_info_0;

/*
CO_API
co_result_0 co_win32_platform_thread_pool_adapter_0_event_facility_0_create_0(co_instance_0 instance, const co_win32_platform_thread_pool_adapter_0_event_facility_0_create_info_0* create_info, co_win32_platform_thread_pool_adapter_0_event_facility_0* event_facility) CO_NOEXCEPT;
*/

/// @brief Same as co_event_domain_0_destroy() on the event facility.
static inline void co_win32_platform_thread_pool_adapter_0_event_facility_0_destroy(co_win32_platform_thread_pool_adapter_0_event_facility_0 event_facility) CO_NOEXCEPT
{
	return co_event_facility_0_destroy(CO_EVENT_FACILITY_0_CAST(event_facility));
}

/// @brief Same as co_event_domain_0_get_service_0() on the event facility.
static inline co_result_0 co_win32_platform_thread_pool_adapter_0_event_facility_0_get_service_0(co_win32_platform_thread_pool_adapter_0_event_facility_0 event_facility, co_service_type_0 service_type, co_service_0* service) CO_NOEXCEPT
{
	return co_event_facility_0_get_service_0(CO_EVENT_FACILITY_0_CAST(event_facility), service_type, service);
}

#pragma endregion

#pragma region Executor

/*
/// @brief Structure type of co_win32_platform_thread_pool_adapter_0_executor_0_create_info_0.
extern CO_API
const co_structure_type_0 CO_STRUCTURE_TYPE_0_WIN32_PLATFORM_THREAD_POOL_ADAPTER_0_EXECUTOR_0_CREATE_INFO_0;

/// @brief Create info of a Win32 platform thread pool adapter executor.
///
/// The Win32 platform thread pool adapter executor is a built-in executor class posting handlers to the legacy thread
/// pool. Create it with co_win32_platform_thread_pool_adapter_0_executor_0_create_0() and destroy it with
/// co_executor_0_destroy().
typedef struct co_win32_platform_thread_pool_adapter_0_executor_0_create_info_0 CO_FINAL
{
	co_structure_type_0 structure_type;
	const co_in_structure_0* next_structure;

	/// @brief A Win32 platform thread pool adapter event facility; must outlive the executor.
	/// @pre Not NULL.
	/// @pre It is an event facility
	co_event_facility_0 win32_platform_thread_pool_adapter_0_event_facility_0_impl;
} co_win32_platform_thread_pool_adapter_0_executor_0_create_info_0;

/// @brief Creates a Win32 platform thread pool adapter executor; *executor is written only on success.
///
/// Returns CO_RESULT_0_ERROR_NOT_SUPPORTED for an unsupported next chain entry or when the legacy thread pool functions are
/// not available, and CO_RESULT_0_ERROR_OUT_OF_MEMORY when the executor cannot be allocated.
/// @pre create_info is not NULL.
/// @pre executor is not NULL.
CO_API
co_result_0 co_win32_platform_thread_pool_adapter_0_executor_0_create_0(co_instance_0 instance, const co_win32_platform_thread_pool_adapter_0_executor_0_create_info_0* create_info, co_executor_0* executor);
*/

#pragma endregion

/// @}

#ifdef __cplusplus
}
#endif
