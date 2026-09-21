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
#include <realtimeapiset.h>

#include <co/win32/clock_0.hpp>

#include <co/impl.hpp>

#include <cassert>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <type_traits>

// Rule: Make sure functions in this file are defined in the same order as in clock_0.h file.

namespace co
{

const structure_type_0 structure_type_0_win32_clock_0_create_info_0 = []() { return static_cast<structure_type_0>(reinterpret_cast<uintptr_t>(&structure_type_0_win32_clock_0_create_info_0)); }();

namespace win32
{

namespace
{

using clock_0_get_system_time_precise_as_file_time_fn = VOID (WINAPI *)(_Out_ LPFILETIME system_time_as_file_time) noexcept;
using clock_0_query_interrupt_time_precise_fn = VOID (WINAPI *)(_Out_ PULONGLONG interrupt_time) noexcept;

template <class Function>
bool probe(HMODULE module, const char* name, Function*& function) noexcept
{
	function = reinterpret_cast<Function*>(GetProcAddress(module, name));
	return function != nullptr;
}

// FILETIME ticks (100 ns units since 1601-01-01); below 2^63 for every value GetSystemTimePreciseAsFileTime() returns.
std::int64_t file_time_ticks(const FILETIME& file_time) noexcept
{
	return static_cast<std::int64_t>((static_cast<std::uint64_t>(file_time.dwHighDateTime) << 32) | file_time.dwLowDateTime);
}

// Calendar time of FILETIME ticks.
co_calendar_time_0 calendar_time_of_ticks(std::int64_t ticks) noexcept
{
	constexpr std::int64_t ticks_per_second = 10000000;
	constexpr std::int64_t seconds_per_day = 86400;
	const std::int64_t seconds = ticks / ticks_per_second;
	const std::int64_t second_of_day = seconds % seconds_per_day;
	const std::chrono::year_month_day date{std::chrono::sys_days{std::chrono::year{1601} / std::chrono::January / 1} + std::chrono::days{seconds / seconds_per_day}};
	return {
		.year = static_cast<std::int32_t>(static_cast<int>(date.year())),
		.month = static_cast<std::uint8_t>(static_cast<unsigned>(date.month())),
		.day = static_cast<std::uint8_t>(static_cast<unsigned>(date.day())),
		.hour = static_cast<std::uint8_t>(second_of_day / 3600),
		.minute = static_cast<std::uint8_t>(second_of_day / 60 % 60),
		.second = static_cast<std::uint8_t>(second_of_day % 60),
		.nanosecond = static_cast<std::uint32_t>(ticks % ticks_per_second * 100),
	};
}

class clock_0_impl final : public co_clock_0_t
{
public:

	clock_0_impl(const co_allocation_callbacks_0& allocation_callbacks, HMODULE realtime, clock_0_get_system_time_precise_as_file_time_fn get_system_time_fn, clock_0_query_interrupt_time_precise_fn query_interrupt_time_fn) noexcept
		: m_allocation_callbacks(allocation_callbacks)
		, m_realtime(realtime)
		, m_get_system_time_fn(get_system_time_fn)
		, m_query_interrupt_time_fn(query_interrupt_time_fn)
	{
		m_allocation_callbacks.next_structure = nullptr;
		FILETIME file_time;
		m_get_system_time_fn(&file_time);
		m_origin_ticks = file_time_ticks(file_time);
		m_origin = calendar_time_of_ticks(m_origin_ticks);
		co_clock_0_init(this, &vtable);
	}

	~clock_0_impl()
	{
		FreeLibrary(m_realtime);
	}

private:

	static clock_0_impl* from(co_clock_0 self) noexcept
	{
		return static_cast<clock_0_impl*>(self);
	}

	static void destroy(co_clock_0 self) noexcept
	{
		clock_0_impl* clock = from(self);
		impl::delete_object_0(clock->m_allocation_callbacks, clock);
	}

	static co_result_0 get_calendar_origin_0(co_clock_0 self, co_calendar_time_0* origin) noexcept
	{
		*origin = from(self)->m_origin;
		return CO_RESULT_0_SUCCESS;
	}

	static void get_time_0(co_clock_0 self, std::int64_t* time) noexcept
	{
		*time = from(self)->time();
	}

	static void get_base_time_0(co_clock_0 self, std::uint64_t* base_time) noexcept
	{
		*base_time = from(self)->base_time();
	}

	static void get_mapping_0(co_clock_0 self, co_clock_0_mapping_0* mapping, std::size_t* generation) noexcept
	{
		clock_0_impl* clock = from(self);
		// Pairs the system time with the middle of the base times read around it, which is not after the reading.
		const std::uint64_t before = clock->base_time();
		const std::int64_t time = clock->time();
		const std::uint64_t after = clock->base_time();
		*mapping = {
			.base_time = before + (after - before) / 2,
			.time = time,
			.rate_numerator = 1,
			.rate_denominator = 1,
		};
		*generation = 0;
	}

	static co_result_0 wait_change_0(co_clock_0 /* self */, std::size_t /* after_generation */, co_cancellation_0 /* cancellation */, co_executor_0 /* executor */, co_clock_0_wait_change_0_completion_fn /* completion_fn */, void* /* data */) noexcept
	{
		// TODO: detect changes of the system time.
		return CO_RESULT_0_ERROR_NOT_SUPPORTED;
	}

	static co_result_0 cancel_0(co_clock_0 /* self */) noexcept
	{
		// No wait is ever pending.
		return CO_RESULT_0_SUCCESS;
	}

	// System time in nanoseconds since the origin, saturated to the range of std::int64_t.
	std::int64_t time() const noexcept
	{
		FILETIME file_time;
		m_get_system_time_fn(&file_time);
		// Both ticks are below 2^63, so the difference fits.
		const std::int64_t since_origin = file_time_ticks(file_time) - m_origin_ticks;
		constexpr std::int64_t max = std::numeric_limits<std::int64_t>::max() / 100;
		constexpr std::int64_t min = std::numeric_limits<std::int64_t>::min() / 100;
		if (since_origin > max) {
			return std::numeric_limits<std::int64_t>::max();
		}
		if (since_origin < min) {
			return std::numeric_limits<std::int64_t>::min();
		}
		return since_origin * 100;
	}

	// Interrupt time in nanoseconds; wraps around after about 58000 years of uptime.
	std::uint64_t base_time() const noexcept
	{
		ULONGLONG interrupt_time;
		m_query_interrupt_time_fn(&interrupt_time);
		return static_cast<std::uint64_t>(interrupt_time) * 100;
	}

private:
	static const co_clock_0_vtable vtable;
	co_allocation_callbacks_0 m_allocation_callbacks;
	HMODULE m_realtime;
	clock_0_get_system_time_precise_as_file_time_fn m_get_system_time_fn;
	clock_0_query_interrupt_time_precise_fn m_query_interrupt_time_fn;
	std::int64_t m_origin_ticks = 0; // FILETIME ticks of the origin
	co_calendar_time_0 m_origin = {};
};

const co_clock_0_vtable clock_0_impl::vtable = {
	.api_version = CO_API_VERSION_0,
	.destroy = &clock_0_impl::destroy,
	.get_calendar_origin_0 = &clock_0_impl::get_calendar_origin_0,
	.get_time_0 = &clock_0_impl::get_time_0,
	.get_base_time_0 = &clock_0_impl::get_base_time_0,
	.get_mapping_0 = &clock_0_impl::get_mapping_0,
	.wait_change_0 = &clock_0_impl::wait_change_0,
	.cancel_0 = &clock_0_impl::cancel_0,
};

} // namespace

} // namespace win32

} // namespace co

const co_structure_type_0 CO_STRUCTURE_TYPE_0_WIN32_CLOCK_0_CREATE_INFO_0 = []() { return static_cast<co_structure_type_0>(reinterpret_cast<uintptr_t>(&co::structure_type_0_win32_clock_0_create_info_0)); }();

static_assert(co::detail::in_structure_of<co_win32_clock_0_create_info_0, co_in_structure_0>);
static_assert(co::detail::in_structure_of<co::win32::clock_0_create_info_0, co::in_structure_0>);
static_assert(std::is_standard_layout_v<co::win32::clock_0_create_info_0>);
static_assert(sizeof(co_win32_clock_0_create_info_0) == sizeof(co::win32::clock_0_create_info_0));
static_assert(offsetof(co_win32_clock_0_create_info_0, include_suspend_time) == offsetof(co::win32::clock_0_create_info_0, include_suspend_time));

co_result_0 co_win32_clock_0_create_0(const co_win32_clock_0_create_info_0* create_info, co_win32_clock_0* clock) noexcept
{
	assert(create_info->structure_type == CO_STRUCTURE_TYPE_0_WIN32_CLOCK_0_CREATE_INFO_0);
	const co_allocation_callbacks_0* allocation_callbacks = nullptr;
	for (const co_in_structure_0* entry = create_info->next_structure; entry != nullptr; entry = entry->next_structure) {
		if (entry->structure_type == CO_STRUCTURE_TYPE_0_ALLOCATION_CALLBACKS_0) {
			const co_allocation_callbacks_0* callbacks = reinterpret_cast<const co_allocation_callbacks_0*>(entry);
			assert(allocation_callbacks == nullptr && "several allocation callbacks in the Win32 clock create info");
			assert(callbacks->allocate_fn != nullptr && callbacks->deallocate_fn != nullptr);
			allocation_callbacks = callbacks;
		}
		else if (entry->structure_type != CO_STRUCTURE_TYPE_0_HINT_INFO_0) {
			return CO_RESULT_0_ERROR_NOT_SUPPORTED;
		}
	}
	if (allocation_callbacks == nullptr) {
		allocation_callbacks = reinterpret_cast<const co_allocation_callbacks_0*>(&co::impl::default_allocation_callbacks_0);
	}

	HMODULE kernel32 = GetModuleHandleW(L"kernel32.dll");
	if (kernel32 == NULL) {
		return CO_RESULT_0_ERROR_NOT_SUPPORTED;
	}
	co::win32::clock_0_get_system_time_precise_as_file_time_fn get_system_time_fn = nullptr;
	if (!co::win32::probe(kernel32, "GetSystemTimePreciseAsFileTime", get_system_time_fn)) {
		return CO_RESULT_0_ERROR_NOT_SUPPORTED;
	}
	// QueryInterruptTimePrecise() and QueryUnbiasedInterruptTimePrecise() are exported by this API set only (Windows 10).
	HMODULE realtime = LoadLibraryExW(L"api-ms-win-core-realtime-l1-1-1.dll", NULL, LOAD_LIBRARY_SEARCH_SYSTEM32);
	if (realtime == NULL) {
		return CO_RESULT_0_ERROR_NOT_SUPPORTED;
	}
	co::win32::clock_0_query_interrupt_time_precise_fn query_interrupt_time_fn = nullptr;
	if (!co::win32::probe(realtime, create_info->include_suspend_time ? "QueryInterruptTimePrecise" : "QueryUnbiasedInterruptTimePrecise", query_interrupt_time_fn)) {
		FreeLibrary(realtime);
		return CO_RESULT_0_ERROR_NOT_SUPPORTED;
	}

	co::win32::clock_0_impl* clock_impl = co::impl::new_object_0<co::win32::clock_0_impl>(*allocation_callbacks, *allocation_callbacks, realtime, get_system_time_fn, query_interrupt_time_fn);
	if (clock_impl == nullptr) {
		FreeLibrary(realtime);
		return CO_RESULT_0_ERROR_OUT_OF_MEMORY;
	}
	*clock = CO_WIN32_CLOCK_0_CAST(static_cast<co_clock_0>(clock_impl));
	return CO_RESULT_0_SUCCESS;
}
