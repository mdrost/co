#define WIN32_LEAN_AND_MEAN
#define NOGDICAPMASKS
#define NOVIRTUALKEYCODES
#define NOWINMESSAGES
#define NOWINSTYLES
#define NOSYSMETRICS
#define NOMENUS
#define NOICONS
#define NOKEYSTATES
#define NOSYSCOMMANDS
#define NORASTEROPS
#define NOSHOWWINDOW
#define NOATOM
#define NOCLIPBOARD
#define NOCOLOR
#define NOCTLMGR
#define NODRAWTEXT
#define NOGDI
#define NOKERNEL
#define NOUSER
#define NONLS
#define NOMB
#define NOMEMMGR
#define NOMETAFILE
#define NOMINMAX
#define NOMSG
#define NOOPENFILE
#define NOSCROLL
#define NOSERVICE
#define NOSOUND
#define NOTEXTMETRIC
#define NOWH
#define NOWINOFFSETS
#define NOCOMM
#define NOKANJI
#define NOHELP
#define NOPROFILER
#define NODEFERWINDOWPOS
#define NOMCX
#define NOIME // ?
#define NORESOURCE // ?
#define WIN32_NO_STATUS
#include <Windows.h>
//#undef WIN32_NO_STATUS
//#include <winternl.h>
//#include <ntstatus.h>

#include <co/win32.h>
#include <co/win32/platform_thread_pool_adapter_0.hpp>

#include "test_guards.hpp"

#include <gtest/gtest.h>

#include <type_traits>

static_assert(co::detail::event_domain_create_info_of<co_win32_platform_thread_pool_adapter_0_event_facility_0_create_info_0, co_event_domain_0_create_info_0>);
static_assert(co::detail::event_domain_create_info_of<co::win32::platform_thread_pool_adapter_0_event_facility_0_create_info_0, co::event_domain_0_create_info_0>);
static_assert(sizeof(co_win32_platform_thread_pool_adapter_0_event_facility_0_create_info_0) == sizeof(co::win32::platform_thread_pool_adapter_0_event_facility_0_create_info_0));

TEST(c_win32_platform_thread_pool_adapter_0_event_facility_0, creates)
{
}

TEST(c_win32_platform_thread_pool_adapter_0_event_facility_0, creates_via_instance_0)
{
	const co_event_domain_group_factory_source_0 win32_classes = {
		.structure_type = CO_STRUCTURE_TYPE_0_EVENT_DOMAIN_GROUP_FACTORY_SOURCE_0,
		.next_structure = NULL,
		.entry_point_fn = &co_win32_event_domain_group_factory_0_entry_point_0,
		.data = NULL,
	};
	co_instance_0_create_info_0 instance_create_info = {
		.structure_type = CO_STRUCTURE_TYPE_0_INSTANCE_0_CREATE_INFO_0,
		.next_structure = CO_IN_STRUCTURE_0_CAST(&win32_classes),
		.api_version = CO_API_VERSION_0,
	};
	co_instance_0 instance;
	ASSERT_EQ(co_instance_0_create_0(&instance_create_info, &instance), CO_RESULT_0_SUCCESS);
	auto instance_guard = co_test::guard(instance);
	co_win32_platform_thread_pool_adapter_0_event_facility_0_create_info_0 event_facility_create_info = {
		.structure_type = CO_STRUCTURE_TYPE_0_WIN32_PLATFORM_THREAD_POOL_ADAPTER_0_EVENT_FACILITY_0_CREATE_INFO_0,
		.next_structure = NULL,
		.required_services = NULL,
		.required_service_count = 0,
	};
	co_event_domain_0 event_domain;
	ASSERT_EQ(co_instance_0_create_event_domain_0(instance, CO_EVENT_DOMAIN_0_CREATE_INFO_0_CAST(&event_facility_create_info), &event_domain), CO_RESULT_0_SUCCESS);
	auto event_domain_guard = co_test::guard(event_domain);
}
