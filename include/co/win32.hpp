#pragma once

#include <co/win32.h>

#include <co.hpp>

namespace co
{

// Win32 IOCP import info: in the next chain of an event_loop_0_create_info_0 or event_facility_0_create_info_0, asks
// for an event domain running on an I/O completion port the application owns. See co_win32_iocp_import_info_0.

extern CO_API
const structure_type_0 structure_type_0_win32_iocp_import_info_0;

struct win32_iocp_import_info_0 final
{
	structure_type_0 structure_type;
	const in_structure_0* next_structure;

	// @pre An open handle to an I/O completion port; neither NULL nor INVALID_HANDLE_VALUE. See co_win32_iocp_import_info_0.
	HANDLE iocp;
};

} // namespace co
