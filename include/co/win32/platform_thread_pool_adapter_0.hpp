#pragma once

#include <co/win32/platform_thread_pool_adapter_0.h>

#include <co.hpp>

namespace co
{

// Win32 platform thread pool adapter event facility: a built-in event facility class providing a timer service (service_type_0_timer_0_service_0).
// Create it with instance_0::create_event_domain_0() from a win32_platform_thread_pool_adapter_0_event_facility_0_create_info_0.
// Creation fails with result_0::error_not_supported when the legacy thread pool functions are not available.

extern CO_API
const structure_type_0 structure_type_0_win32_platform_thread_pool_adapter_0_event_facility_0_create_info_0;

namespace win32
{

struct platform_thread_pool_adapter_0_event_facility_0_create_info_0 final
{
	structure_type_0 structure_type;
	const in_structure_0* next_structure;

	/// Services the event facility must provide.
	const service_type_0* required_services;
	size_t required_service_count;
};

} // namespace win32

/*
// Win32 platform thread pool adapter executor: a built-in executor class posting handlers to the legacy thread pool.
// Create it with win32_platform_thread_pool_adapter_0_executor_0_create_0() and destroy it with executor_0::destroy().

extern CO_API
const structure_type_0 structure_type_0_win32_platform_thread_pool_adapter_0_executor_0_create_info_0;
*/

namespace win32
{

/*
struct platform_thread_pool_adapter_0_executor_0_create_info_0 final
{
	structure_type_0 structure_type;
	const in_structure_0* next_structure;

	// A Win32 thread pool event facility; must outlive the executor.
	// @pre Not null.
	// @pre It is an event facility of the Win32 thread pool event facility class.
	event_facility_0 win32_platform_thread_pool_adapter_0_event_facility_0_impl;
};

// See co_win32_platform_thread_pool_adapter_0_executor_0_create_0().
inline result_0 platform_thread_pool_adapter_0_executor_0_create_0(instance_0 instance, const platform_thread_pool_adapter_0_executor_0_create_info_0& create_info, executor_0* executor) noexcept
{
	return static_cast<result_0>(co_win32_platform_thread_pool_adapter_0_executor_0_create_0(static_cast<co_instance_0>(instance), reinterpret_cast<const co_win32_platform_thread_pool_adapter_0_executor_0_create_info_0*>(&create_info), reinterpret_cast<co_executor_0*>(executor)));
}
*/

} // namespace win32

} // namespace co
