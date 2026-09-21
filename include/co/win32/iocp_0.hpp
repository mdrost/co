#pragma once

#include <co/win32/iocp_0.h>

#include <co.hpp>

namespace co
{

// Win32 IOCP event loop: a built-in event loop class.
// Create it with instance_0::create_event_domain_0() from a win32_iocp_0_event_loop_0_create_info_0, which starts
// with event_domain_0_create_info_0.

extern CO_API
const structure_type_0 structure_type_0_win32_iocp_0_event_loop_0_create_info_0;

struct win32_iocp_0_event_loop_0_create_info_0 final
{
	structure_type_0 structure_type;
	const in_structure_0* next_structure;

	/// Services the event loop must provide.
	const service_type_0* required_services;
	size_t required_service_count;
};

} // namespace co
