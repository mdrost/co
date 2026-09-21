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

#include <co/win32.hpp>

#include <gtest/gtest.h>

#include <cstddef>

static_assert(co::detail::in_structure_of<co_win32_iocp_import_info_0, co_in_structure_0>);
static_assert(co::detail::in_structure_of<co::win32_iocp_import_info_0, co::in_structure_0>);
static_assert(sizeof(co_win32_iocp_import_info_0) == sizeof(co::win32_iocp_import_info_0));
static_assert(offsetof(co_win32_iocp_import_info_0, iocp) == offsetof(co::win32_iocp_import_info_0, iocp));

TEST(c_win32_iocp_import_info_0, links_into_event_loop_create_info_chain)
{
	HANDLE iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 1);
	ASSERT_NE(iocp, nullptr); // ASSERT_NE(iocp, NULL);
	const co_win32_iocp_import_info_0 import_info = {
		.structure_type = CO_STRUCTURE_TYPE_0_WIN32_IOCP_IMPORT_INFO_0,
		.next_structure = nullptr,
		.iocp = iocp,
	};
	const co_event_loop_0_create_info_0 create_info = {
		.structure_type = CO_STRUCTURE_TYPE_0_EVENT_LOOP_0_CREATE_INFO_0,
		.next_structure = CO_IN_STRUCTURE_0_CAST(&import_info),
		.required_services = nullptr,
		.required_service_count = 0,
	};
	EXPECT_EQ(create_info.next_structure->structure_type, CO_STRUCTURE_TYPE_0_WIN32_IOCP_IMPORT_INFO_0);
	CloseHandle(iocp);
}
