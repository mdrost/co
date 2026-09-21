#pragma once

#include <co.h>

#ifdef __cplusplus
extern "C"
{
#endif

/// @brief Structure type of co_win32_iocp_import_info_0.
extern CO_API
const co_structure_type_0 CO_STRUCTURE_TYPE_0_WIN32_IOCP_IMPORT_INFO_0;

/// @brief Import info asking for an event domain running on an I/O completion port the application owns.
///
/// Put it in the next chain of a co_event_loop_0_create_info_0 or co_event_facility_0_create_info_0. Names no class:
/// the class is chosen as for any create info (see co_instance_0_create_event_domain_0()) among the classes listing
/// its structure type among the handled types of their factory, for example the Win32 IOCP adapter event loop
/// (co/win32/iocp_adapter_0.h). The event domain neither closes the port nor outlives the use of it the application allows.
typedef struct co_win32_iocp_import_info_0 CO_FINAL
{
	co_structure_type_0 structure_type;
	const co_in_structure_0* next_structure;

	/// @brief The I/O completion port.
	/// @pre An open handle to an I/O completion port, as returned by CreateIoCompletionPort(); neither NULL nor
	/// INVALID_HANDLE_VALUE. It stays open until the event domain is destroyed.
	HANDLE iocp;
} co_win32_iocp_import_info_0;

/// @brief Entry point
/// co_event_domain_group_factory_0_entry_point_0_fn.
///
/// Register the classes in an instance with a co_event_domain_group_factory_source_0 whose entry_point_fn is this
/// function; data is ignored. Classes not supported on this machine are skipped.
CO_API
co_result_0 co_win32_event_domain_group_factory_0_entry_point_0(void* data, const co_event_domain_group_factory_0_entry_point_info_0* entry_point_info, size_t* factory_count, co_event_domain_group_factory_0* factories) CO_NOEXCEPT;

#ifdef __cplusplus
}
#endif
