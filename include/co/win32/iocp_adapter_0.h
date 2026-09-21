#pragma once

#include <co.h>

#ifdef __cplusplus
extern "C"
{
#endif

// Win32 IOCP adapter event loop: a built-in event loop class running on an I/O completion port the application owns.
// Create it with co_instance_0_create_event_domain_0() from a co_win32_iocp_adapter_0_event_loop_0_create_info_0,
// which starts with co_event_domain_0_create_info_0, to ask for this class. The class also accepts a generic
// co_event_loop_0_create_info_0 with a co_win32_iocp_import_info_0 (co/win32.h) in its next chain,
// which asks for any class running on the given port.

extern CO_API
const co_structure_type_0 CO_STRUCTURE_TYPE_0_WIN32_IOCP_ADAPTER_0_EVENT_LOOP_0_CREATE_INFO_0;

typedef struct co_win32_iocp_adapter_0_event_loop_0_create_info_0 CO_FINAL
{
	co_structure_type_0 structure_type;
	const co_in_structure_0* next_structure;

	/// @brief Services the event loop must provide; only these may be asked for with co_event_domain_0_get_service_0().
	/// @pre Not NULL when required_service_count is not zero.
	const co_service_type_0* required_services;
	/// @brief Number of elements of required_services.
	size_t required_service_count;

	// @pre An open handle to an I/O completion port; neither NULL nor INVALID_HANDLE_VALUE.
	HANDLE iocp;
} co_win32_iocp_adapter_0_event_loop_0_create_info_0;

#ifdef __cplusplus
}
#endif
