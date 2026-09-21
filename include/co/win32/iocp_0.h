#pragma once

#include <co.h>

#ifdef __cplusplus
extern "C"
{
#endif

// Win32 IOCP event loop: a built-in event loop class.
// Create it with co_instance_0_create_event_domain_0() from a co_win32_iocp_0_event_loop_0_create_info_0, which
// starts with co_event_domain_0_create_info_0.

extern CO_API
const co_structure_type_0 CO_STRUCTURE_TYPE_0_WIN32_IOCP_0_EVENT_LOOP_0_CREATE_INFO_0;

typedef struct co_win32_iocp_0_event_loop_0_create_info_0 CO_FINAL
{
	co_structure_type_0 structure_type;
	const co_in_structure_0* next_structure;

	/// @brief Services the event loop must provide; only these may be asked for with co_event_domain_0_get_service_0().
	/// @pre Not NULL when required_service_count is not zero.
	const co_service_type_0* required_services;
	/// @brief Number of elements of required_services.
	size_t required_service_count;
} co_win32_iocp_0_event_loop_0_create_info_0;

#ifdef __cplusplus
}
#endif
