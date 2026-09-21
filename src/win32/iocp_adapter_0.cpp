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

#include "win32/iocp_adapter_0.hpp"

#include <cstddef>
#include <type_traits>

namespace co
{

namespace
{

class win32_iocp_adapter_0_event_loop_0 final
{
public:

	using create_info_type = win32_iocp_adapter_0_event_loop_0_create_info_0;

	static structure_type_0 create_info_structure_type() noexcept
	{
		return structure_type_0_win32_iocp_adapter_0_event_loop_0_create_info_0;
	}

	result_0 construct_0(event_loop_0 /* self */, const win32_iocp_adapter_0_event_loop_0_create_info_0& create_info) noexcept
	{
		if (create_info.iocp == NULL || create_info.iocp == INVALID_HANDLE_VALUE) {
			return result_0::error_invalid_argument;
		}
		m_iocp = create_info.iocp;
		return result_0::success;
	}

	result_0 run_0() noexcept
	{
		return result_0::success;
	}

	result_0 stop_0() noexcept
	{
		return result_0::success;
	}

private:
	HANDLE m_iocp = NULL;
};

} // namespace

const structure_type_0 structure_type_0_win32_iocp_adapter_0_event_loop_0_create_info_0 = []() { return static_cast<structure_type_0>(reinterpret_cast<uintptr_t>(&structure_type_0_win32_iocp_adapter_0_event_loop_0_create_info_0)); }();

} // namespace co

const co_structure_type_0 CO_STRUCTURE_TYPE_0_WIN32_IOCP_ADAPTER_0_EVENT_LOOP_0_CREATE_INFO_0 = []() { return static_cast<co_structure_type_0>(reinterpret_cast<uintptr_t>(&co::structure_type_0_win32_iocp_adapter_0_event_loop_0_create_info_0)); }();

static_assert(co::detail::event_domain_create_info_of<co_win32_iocp_adapter_0_event_loop_0_create_info_0, co_event_domain_0_create_info_0>);
static_assert(co::detail::event_domain_create_info_of<co::win32_iocp_adapter_0_event_loop_0_create_info_0, co::event_domain_0_create_info_0>);
static_assert(sizeof(co_win32_iocp_adapter_0_event_loop_0_create_info_0) == sizeof(co::win32_iocp_adapter_0_event_loop_0_create_info_0));
static_assert(offsetof(co_win32_iocp_adapter_0_event_loop_0_create_info_0, iocp) == offsetof(co::win32_iocp_adapter_0_event_loop_0_create_info_0, iocp));
