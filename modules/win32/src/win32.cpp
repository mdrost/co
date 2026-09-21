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

#include "win32.hpp"

#include "win32/tp_executor_priority.hpp"

#include <cstddef>
#include <type_traits>

namespace co
{

const structure_type_0 structure_type_0_win32_iocp_import_info_0 = []() { return static_cast<structure_type_0>(reinterpret_cast<uintptr_t>(&structure_type_0_win32_iocp_import_info_0)); }();

} // namespace co

const co_structure_type_0 CO_STRUCTURE_TYPE_0_WIN32_IOCP_IMPORT_INFO_0 = []() { return static_cast<co_structure_type_0>(reinterpret_cast<uintptr_t>(&co::structure_type_0_win32_iocp_import_info_0)); }();

static_assert(co::detail::in_structure_of<co_win32_iocp_import_info_0, co_in_structure_0>);
static_assert(co::detail::in_structure_of<co::win32_iocp_import_info_0, co::in_structure_0>);
static_assert(sizeof(co_win32_iocp_import_info_0) == sizeof(co::win32_iocp_import_info_0));
static_assert(offsetof(co_win32_iocp_import_info_0, iocp) == offsetof(co::win32_iocp_import_info_0, iocp));

static_assert(std::is_same_v<decltype(&co_win32_event_domain_group_factory_0_entry_point_0), co_event_domain_group_factory_0_entry_point_0_fn>);

co_result_0 co_win32_event_domain_group_factory_0_entry_point_0(void* /* data */, const co_event_domain_group_factory_0_entry_point_info_0* /* entry_point_info */, std::size_t* factory_count, co_event_domain_group_factory_0* /* factories */) noexcept
{
	// TODO: create the factories of the Win32 classes as they are ported to event domain groups.
	*factory_count = 0;
	return CO_RESULT_0_SUCCESS;
}
