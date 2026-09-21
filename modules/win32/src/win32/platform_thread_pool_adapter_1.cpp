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

#include "platform_thread_pool_adapter_1.hpp"

#include <co/dynamic_thread_pool_0.hpp>
#include <co/platform_thread_pool_0.hpp>
#include <co/timer_0_service_0.hpp>

#include <co/impl.hpp>

#include <array> // std::size()

namespace co::win32
{

namespace
{

using platform_thread_pool_adapter_1_get_last_error_fn = _Check_return_ _Post_equals_last_error_ DWORD (WINAPI *)(VOID) noexcept;

using platform_thread_pool_adapter_1_create_threadpool_timer_fn = _Must_inspect_result_ PTP_TIMER (WINAPI *)(_In_ PTP_TIMER_CALLBACK callback, _Inout_opt_ PVOID data, _In_opt_ PTP_CALLBACK_ENVIRON callback_environ) noexcept;
using platform_thread_pool_adapter_1_close_threadpool_timer_fn = VOID (WINAPI *)(_Inout_ PTP_TIMER timer) noexcept;
using platform_thread_pool_adapter_1_set_threadpool_timer_fn = VOID (WINAPI *)(_Inout_ PTP_TIMER timer, _In_opt_ PFILETIME due_time, _In_ DWORD period_ms, _In_opt_ DWORD window_length_ms) noexcept;

template <class Function>
void get(HMODULE module, const char* name, Function*& function) noexcept
{
	function = reinterpret_cast<Function*>(GetProcAddress(module, name));
}

} // namespace

} // namespace co::win32

#pragma region Timer

namespace co::win32
{

namespace
{

class platform_thread_pool_adapter_1_timer_0_impl_0 final : protected co_timer_0_t
{
public:

	platform_thread_pool_adapter_1_timer_0_impl_0(
		const co_allocation_callbacks_0& allocation_callbacks,
		co_clock_0 clock,
		platform_thread_pool_adapter_1_get_last_error_fn get_last_error_fn,
		platform_thread_pool_adapter_1_create_threadpool_timer_fn create_threadpool_timer_fn,
		platform_thread_pool_adapter_1_close_threadpool_timer_fn close_threadpool_timer_fn,
		platform_thread_pool_adapter_1_set_threadpool_timer_fn set_threadpool_timer_fn
	) noexcept
		: allocation_callbacks({
			.structure_type = allocation_callbacks.structure_type,
			.next_structure = nullptr,
			.allocate_fn = allocation_callbacks.allocate_fn,
			.deallocate_fn = allocation_callbacks.deallocate_fn,
			.data = allocation_callbacks.data
		})
		, clock(clock)
		, get_last_error_fn(get_last_error_fn)
		, create_threadpool_timer_fn(create_threadpool_timer_fn)
		, close_threadpool_timer_fn(close_threadpool_timer_fn)
		, set_threadpool_timer_fn(set_threadpool_timer_fn)
	{
		co_timer_0_init(this, &timer_vtable);
	}

private:

	static void destroy(platform_thread_pool_adapter_1_timer_0_impl_0* self) noexcept
	{
		impl::delete_object_0(self->allocation_callbacks, self);
	}

	static void get_clock_0(platform_thread_pool_adapter_1_timer_0_impl_0* self, co_clock_0* clock) noexcept
	{
		*clock = self->clock;
	}

	static co_result_0 expiry_at_0(platform_thread_pool_adapter_1_timer_0_impl_0* self, int64_t time_ns) noexcept
	{
		return CO_RESULT_0_ERROR_UNKNOWN;
	}

	static co_result_0 expiry_after_0(platform_thread_pool_adapter_1_timer_0_impl_0* self, uint64_t duration_ns) noexcept
	{
		return CO_RESULT_0_ERROR_UNKNOWN;
	}

	static co_result_0 wait_0(platform_thread_pool_adapter_1_timer_0_impl_0* self, co_cancellation_0 cancellation, co_executor_0 executor, co_timer_0_wait_0_completion_fn completion_fn, void* data) noexcept
	{
		return CO_RESULT_0_ERROR_UNKNOWN;
	}

	static co_result_0 cancel_0(platform_thread_pool_adapter_1_timer_0_impl_0* self) noexcept
	{
		return CO_RESULT_0_ERROR_UNKNOWN;
	}

private:
	static const co_timer_0_vtable timer_vtable;
	const co_allocation_callbacks_0 allocation_callbacks;
	const co_clock_0 clock;
	const platform_thread_pool_adapter_1_get_last_error_fn get_last_error_fn;
	const platform_thread_pool_adapter_1_create_threadpool_timer_fn create_threadpool_timer_fn;
	const platform_thread_pool_adapter_1_close_threadpool_timer_fn close_threadpool_timer_fn;
	const platform_thread_pool_adapter_1_set_threadpool_timer_fn set_threadpool_timer_fn;
};

const co_timer_0_vtable platform_thread_pool_adapter_1_timer_0_impl_0::timer_vtable = {
	.api_version = CO_API_VERSION_0,
	.destroy = reinterpret_cast<co_timer_0_destroy_fn>(&platform_thread_pool_adapter_1_timer_0_impl_0::destroy),
	.get_clock_0 = reinterpret_cast<co_timer_0_get_clock_0_fn>(&platform_thread_pool_adapter_1_timer_0_impl_0::get_clock_0),
	.expiry_at_0 = reinterpret_cast<co_timer_0_expiry_at_0_fn>(&platform_thread_pool_adapter_1_timer_0_impl_0::expiry_at_0),
	.expiry_after_0 = reinterpret_cast<co_timer_0_expiry_after_0_fn>(&platform_thread_pool_adapter_1_timer_0_impl_0::expiry_after_0),
	.wait_0 = reinterpret_cast<co_timer_0_wait_0_fn>(&platform_thread_pool_adapter_1_timer_0_impl_0::wait_0),
	.cancel_0 = reinterpret_cast<co_timer_0_cancel_0_fn>(&platform_thread_pool_adapter_1_timer_0_impl_0::cancel_0),
};

} // namespace

} // namespace co::win32

#pragma endregion

#pragma region Timer service

namespace co::win32
{

namespace
{

class platform_thread_pool_adapter_1_timer_0_service_0_impl_0 : protected co_timer_0_service_0_t
{
public:

	platform_thread_pool_adapter_1_timer_0_service_0_impl_0(
		const co_allocation_callbacks_0& allocation_callbacks,
		platform_thread_pool_adapter_1_get_last_error_fn get_last_error_fn,
		platform_thread_pool_adapter_1_create_threadpool_timer_fn create_threadpool_timer_fn,
		platform_thread_pool_adapter_1_close_threadpool_timer_fn close_threadpool_timer_fn,
		platform_thread_pool_adapter_1_set_threadpool_timer_fn set_threadpool_timer_fn
	) noexcept
		: allocation_callbacks({
			.structure_type = allocation_callbacks.structure_type,
			.next_structure = nullptr,
			.allocate_fn = allocation_callbacks.allocate_fn,
			.deallocate_fn = allocation_callbacks.deallocate_fn,
			.data = allocation_callbacks.data
		})
		, get_last_error_fn(get_last_error_fn)
		, create_threadpool_timer_fn(create_threadpool_timer_fn)
		, close_threadpool_timer_fn(close_threadpool_timer_fn)
		, set_threadpool_timer_fn(set_threadpool_timer_fn)
	{
		co_timer_0_service_0_init(this, &service_vtable, &timer_service_vtable);
	}

private:

	static void destroy(platform_thread_pool_adapter_1_timer_0_service_0_impl_0* self) noexcept
	{
		impl::delete_object_0(self->allocation_callbacks, self);
	}

	static co_result_0 create_timer_0(platform_thread_pool_adapter_1_timer_0_service_0_impl_0* self, const co_timer_0_create_info_0* create_info, co_timer_0* timer) noexcept
	{
		platform_thread_pool_adapter_1_timer_0_impl_0* timer_impl = impl::new_object_0<platform_thread_pool_adapter_1_timer_0_impl_0>(
			self->allocation_callbacks,
			self->allocation_callbacks,
			create_info->clock,
			self->get_last_error_fn,
			self->create_threadpool_timer_fn,
			self->close_threadpool_timer_fn,
			self->set_threadpool_timer_fn
		);
		*timer = CO_TIMER_0_CAST(timer_impl);
		if (!*timer) {
			return CO_RESULT_0_ERROR_OUT_OF_MEMORY;
		}
		return CO_RESULT_0_SUCCESS;
	}

private:
	static const co_service_0_vtable service_vtable;
	static const co_timer_0_service_0_vtable timer_service_vtable;
	const co_allocation_callbacks_0 allocation_callbacks;
	const platform_thread_pool_adapter_1_get_last_error_fn get_last_error_fn;
	const platform_thread_pool_adapter_1_create_threadpool_timer_fn create_threadpool_timer_fn;
	const platform_thread_pool_adapter_1_close_threadpool_timer_fn close_threadpool_timer_fn;
	const platform_thread_pool_adapter_1_set_threadpool_timer_fn set_threadpool_timer_fn;
};

const co_service_0_vtable platform_thread_pool_adapter_1_timer_0_service_0_impl_0::service_vtable = {
	.api_version = CO_API_VERSION_0,
	.destroy = reinterpret_cast<co_service_0_destroy_fn>(&platform_thread_pool_adapter_1_timer_0_service_0_impl_0::destroy),
};

const co_timer_0_service_0_vtable platform_thread_pool_adapter_1_timer_0_service_0_impl_0::timer_service_vtable = {
	.create_timer_0 = reinterpret_cast<co_timer_0_service_0_create_timer_0_fn>(&platform_thread_pool_adapter_1_timer_0_service_0_impl_0::create_timer_0),
};

} // namespace

} // namespace co::win32

#pragma endregion

#pragma region Event facility

namespace co::win32
{

namespace
{

class platform_thread_pool_adapter_1_event_facility_0_impl_0 : protected co_event_facility_0_t
{
public:

	platform_thread_pool_adapter_1_event_facility_0_impl_0(
		const co_allocation_callbacks_0& allocation_callbacks,
		co_timer_0_service_0 timer_service
	) noexcept
		: allocation_callbacks({
			.structure_type = allocation_callbacks.structure_type,
			.next_structure = nullptr,
			.allocate_fn = allocation_callbacks.allocate_fn,
			.deallocate_fn = allocation_callbacks.deallocate_fn,
			.data = allocation_callbacks.data
		})
		, timer_service(timer_service)
	{
		co_event_facility_0_init(this, &event_domain_vtable, &event_facility_vtable);
	}

private:

	static void destroy(platform_thread_pool_adapter_1_event_facility_0_impl_0* self) noexcept
	{
		impl::delete_object_0(self->allocation_callbacks, self);
	}

	static co_result_0 get_service_0(platform_thread_pool_adapter_1_event_facility_0_impl_0* self, co_service_type_0 service_type, co_service_0* service) noexcept
	{
		if (service_type == CO_SERVICE_TYPE_0_TIMER_0_SERVICE_0) {
			*service = CO_SERVICE_0_CAST(self->timer_service);
			return CO_RESULT_0_SUCCESS;
		}
		return CO_RESULT_0_ERROR_SERVICE_NOT_FOUND;
	}

private:
	static const co_event_domain_0_vtable event_domain_vtable;
	static const co_event_facility_0_vtable event_facility_vtable;
	const co_allocation_callbacks_0 allocation_callbacks;
	co_timer_0_service_0 timer_service;
};

const co_event_domain_0_vtable platform_thread_pool_adapter_1_event_facility_0_impl_0::event_domain_vtable = {
	.api_version = CO_API_VERSION_0,
	.destroy = reinterpret_cast<co_event_domain_0_destroy_fn>(&platform_thread_pool_adapter_1_event_facility_0_impl_0::destroy),
	.get_service_0 = reinterpret_cast<co_event_domain_0_get_service_0_fn>(&platform_thread_pool_adapter_1_event_facility_0_impl_0::get_service_0),
};

const co_event_facility_0_vtable platform_thread_pool_adapter_1_event_facility_0_impl_0::event_facility_vtable = {
};

} // namespace

} // namespace co::win32

#pragma endregion

#pragma region Event domain group

namespace co::win32
{

namespace
{

class platform_thread_pool_adapter_1_event_domain_group_0_impl_0 : protected co_event_domain_group_0_t
{
public:

	platform_thread_pool_adapter_1_event_domain_group_0_impl_0(
		const co_allocation_callbacks_0& allocation_callbacks,
		co_timer_0_service_0 timer_service
	) noexcept
		: allocation_callbacks({
			.structure_type = allocation_callbacks.structure_type,
			.next_structure = nullptr,
			.allocate_fn = allocation_callbacks.allocate_fn,
			.deallocate_fn = allocation_callbacks.deallocate_fn,
			.data = allocation_callbacks.data
		})
		, event_facility(allocation_callbacks, timer_service)
	{
		co_event_domain_group_0_init(this, &event_domain_group_vtable);
	}

private:

	static void destroy(platform_thread_pool_adapter_1_event_domain_group_0_impl_0* self) noexcept
	{
		impl::delete_object_0(self->allocation_callbacks, self);
	}

	static void get_event_domain_0(platform_thread_pool_adapter_1_event_domain_group_0_impl_0* self, size_t event_domain_index, co_event_domain_0* event_domain) noexcept
	{
	}

private:
	static const co_event_domain_group_0_vtable event_domain_group_vtable;
	const co_allocation_callbacks_0 allocation_callbacks;
	platform_thread_pool_adapter_1_event_facility_0_impl_0 event_facility;
};

const co_event_domain_group_0_vtable platform_thread_pool_adapter_1_event_domain_group_0_impl_0::event_domain_group_vtable = {
	.api_version = CO_API_VERSION_0,
	.destroy = reinterpret_cast<co_event_domain_group_0_destroy_fn>(&platform_thread_pool_adapter_1_event_domain_group_0_impl_0::destroy),
	.get_event_domain_0 = reinterpret_cast<co_event_domain_group_0_get_event_domain_0_fn>(&platform_thread_pool_adapter_1_event_domain_group_0_impl_0::get_event_domain_0),
};

} // namespace

} // namespace co::win32

#pragma endregion

#pragma region Event domain group factory

namespace co::win32
{

namespace
{

class platform_thread_pool_adapter_1_event_domain_group_factory_0_impl_0 : protected co_event_domain_group_factory_0_t
{
public:

	// Takes ownership of timer_service.
	platform_thread_pool_adapter_1_event_domain_group_factory_0_impl_0(
		const co_allocation_callbacks_0& allocation_callbacks,
		co_timer_0_service_0 timer_service
	) noexcept
		: allocation_callbacks({
			.structure_type = allocation_callbacks.structure_type,
			.next_structure = nullptr,
			.allocate_fn = allocation_callbacks.allocate_fn,
			.deallocate_fn = allocation_callbacks.deallocate_fn,
			.data = allocation_callbacks.data
		})
		, member_create_info_structure_types{
			CO_STRUCTURE_TYPE_0_EVENT_FACILITY_0_CREATE_INFO_0,
			CO_STRUCTURE_TYPE_0_WIN32_PLATFORM_THREAD_POOL_ADAPTER_1_EVENT_FACILITY_0_CREATE_INFO_0,
		}
		, handled_structure_types{
			CO_STRUCTURE_TYPE_0_DYNAMIC_THREAD_POOL_0_CREATE_INFO_0,
			CO_STRUCTURE_TYPE_0_PLATFORM_THREAD_POOL_0_CREATE_INFO_0,
		}
		, provided_service_types{
			CO_SERVICE_TYPE_0_TIMER_0_SERVICE_0,
		}
		, timer_service(timer_service)
	{
		co_event_domain_group_factory_0_init(this, &event_domain_group_factory_vtable);
	}

private:

	static void destroy(platform_thread_pool_adapter_1_event_domain_group_factory_0_impl_0* self) noexcept
	{
		co_timer_0_service_0_destroy(self->timer_service);
		impl::delete_object_0(self->allocation_callbacks, self);
	}

	static void get_member_create_info_structure_types_0(platform_thread_pool_adapter_1_event_domain_group_factory_0_impl_0* self, const co_structure_type_0** member_create_info_structure_types, size_t* member_create_info_structure_type_count) noexcept
	{
		*member_create_info_structure_types = self->member_create_info_structure_types;
		*member_create_info_structure_type_count = std::size(self->member_create_info_structure_types);
	}

	static void get_handled_structure_types_0(platform_thread_pool_adapter_1_event_domain_group_factory_0_impl_0* self, const co_structure_type_0** handled_structure_types, size_t* handled_structure_type_count) noexcept
	{
		*handled_structure_types = self->handled_structure_types;
		*handled_structure_type_count = std::size(self->handled_structure_types);
	}

	static void get_provided_service_types_0(platform_thread_pool_adapter_1_event_domain_group_factory_0_impl_0* self, const co_service_type_0** provided_service_types, size_t* provided_service_type_count) noexcept
	{
		*provided_service_types = self->provided_service_types;
		*provided_service_type_count = std::size(self->provided_service_types);
	}

	static void get_member_count_range_0(platform_thread_pool_adapter_1_event_domain_group_factory_0_impl_0* self, size_t* min_count, size_t* max_count) noexcept
	{
		*min_count = 1;
		*max_count = 1;
	}

	static void get_rank_0(platform_thread_pool_adapter_1_event_domain_group_factory_0_impl_0* self, size_t* rank) noexcept
	{
		*rank = 0;
	}

	static co_result_0 create_event_domain_group_0(platform_thread_pool_adapter_1_event_domain_group_factory_0_impl_0* self, const co_event_domain_group_0_create_info_0* create_info, co_event_domain_group_0* event_domain_group) noexcept
	{
		*event_domain_group = CO_EVENT_DOMAIN_GROUP_0_CAST(impl::new_object_0<platform_thread_pool_adapter_1_event_domain_group_0_impl_0>(
			self->allocation_callbacks,
			self->allocation_callbacks,
			self->timer_service
		));
		if (!*event_domain_group) {
			return CO_RESULT_0_ERROR_OUT_OF_MEMORY;
		}
		return CO_RESULT_0_SUCCESS;
	}

private:
	static const co_event_domain_group_factory_0_vtable event_domain_group_factory_vtable;
	const co_allocation_callbacks_0 allocation_callbacks;
	const co_structure_type_0 member_create_info_structure_types[2];
	const co_structure_type_0 handled_structure_types[2];
	const co_service_type_0 provided_service_types[1];
	const co_timer_0_service_0 timer_service;
};

const co_event_domain_group_factory_0_vtable platform_thread_pool_adapter_1_event_domain_group_factory_0_impl_0::event_domain_group_factory_vtable = {
	.api_version = CO_API_VERSION_0,
	.destroy = reinterpret_cast<co_event_domain_group_factory_0_destroy_fn>(&platform_thread_pool_adapter_1_event_domain_group_factory_0_impl_0::destroy),
	.get_member_create_info_structure_types_0 = reinterpret_cast<co_event_domain_group_factory_0_get_member_create_info_structure_types_0_fn>(&platform_thread_pool_adapter_1_event_domain_group_factory_0_impl_0::get_member_create_info_structure_types_0),
	.get_handled_structure_types_0 = reinterpret_cast<co_event_domain_group_factory_0_get_handled_structure_types_0_fn>(&platform_thread_pool_adapter_1_event_domain_group_factory_0_impl_0::get_handled_structure_types_0),
	.get_provided_service_types_0 = reinterpret_cast<co_event_domain_group_factory_0_get_provided_service_types_0_fn>(&platform_thread_pool_adapter_1_event_domain_group_factory_0_impl_0::get_provided_service_types_0),
	.get_member_count_range_0 = reinterpret_cast<co_event_domain_group_factory_0_get_member_count_range_0_fn>(&platform_thread_pool_adapter_1_event_domain_group_factory_0_impl_0::get_member_count_range_0),
	.get_rank_0 = reinterpret_cast<co_event_domain_group_factory_0_get_rank_0_fn>(&platform_thread_pool_adapter_1_event_domain_group_factory_0_impl_0::get_rank_0),
	.create_event_domain_group_0 = reinterpret_cast<co_event_domain_group_factory_0_create_event_domain_group_0_fn>(&platform_thread_pool_adapter_1_event_domain_group_factory_0_impl_0::create_event_domain_group_0),
};

} // namespace

} // namespace co::win32

#pragma endregion