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

#include "platform_thread_pool_adapter_0.hpp"

#include <co/impl.hpp>
#include <co/dynamic_thread_pool_0.hpp>
#include <co/platform_thread_pool_0.hpp>
#include <co/suspend_watcher_0_service_0.hpp>
#include <co/timer_0_service_0.hpp>

#include <atomic>
#include <cassert>
#include <cstddef>
#include <mutex>
#include <new>
#include <optional>
#include <type_traits>

// Allocate the completion handler in start_0(), where running out of memory can still be reported,
// so that posting it later (from elapsed() or cancel_0()) never allocates.
#define TIMER_PREALLOCATE_COMPLETION_HANDLER 1

namespace co
{

namespace win32
{

using platform_thread_pool_adapter_0_functions_0_queue_user_work_item_fn = _Must_inspect_result_ BOOL (WINAPI *)(_In_ LPTHREAD_START_ROUTINE function, _In_opt_ PVOID context, _In_ ULONG flags) noexcept;
//using platform_thread_pool_adapter_0_functions_0_create_timer_queue_fn = _Ret_maybenull_ HANDLE (WINAPI *)(VOID) noexcept;
//using platform_thread_pool_adapter_0_functions_0_delete_timer_queue_ex_fn = _Must_inspect_result_ BOOL (WINAPI *)(_In_ HANDLE timer_queue, _In_opt_ HANDLE completion_event) noexcept;
using platform_thread_pool_adapter_0_functions_0_create_timer_queue_timer_fn = _Must_inspect_result_ BOOL (WINAPI *)(_Outptr_ PHANDLE new_timer, _In_opt_ HANDLE timer_queue, _In_ WAITORTIMERCALLBACK callback, _In_opt_ PVOID parameter, _In_ DWORD due_time, _In_ DWORD period, _In_ ULONG flags) noexcept;
using platform_thread_pool_adapter_0_functions_0_change_timer_queue_timer_fn = _Must_inspect_result_ BOOL (WINAPI *)(_In_opt_ HANDLE timer_queue, _Inout_ HANDLE timer, _In_ ULONG due_time, _In_ ULONG period) noexcept;
using platform_thread_pool_adapter_0_functions_0_delete_timer_queue_timer_fn = _Must_inspect_result_ BOOL (WINAPI *)(_In_opt_ HANDLE timer_queue, _In_ HANDLE timer, _In_opt_ HANDLE completion_event) noexcept;
using platform_thread_pool_adapter_0_functions_0_get_last_error_fn = _Check_return_ _Post_equals_last_error_ DWORD (WINAPI *)(VOID) noexcept;

// Legacy thread pool functions, resolved at runtime so that a missing export
// makes facility creation fail instead of preventing the module from loading.
struct platform_thread_pool_adapter_0_functions_0 final
{
	platform_thread_pool_adapter_0_functions_0_queue_user_work_item_fn queue_user_work_item_fn;
	//platform_thread_pool_adapter_0_functions_0_create_timer_queue_fn create_timer_queue_fn;
	//platform_thread_pool_adapter_0_functions_0_delete_timer_queue_ex_fn delete_timer_queue_ex_fn;
	platform_thread_pool_adapter_0_functions_0_create_timer_queue_timer_fn create_timer_queue_timer_fn;
	platform_thread_pool_adapter_0_functions_0_change_timer_queue_timer_fn change_timer_queue_timer_fn;
	platform_thread_pool_adapter_0_functions_0_delete_timer_queue_timer_fn delete_timer_queue_timer_fn;
	platform_thread_pool_adapter_0_functions_0_get_last_error_fn get_last_error_fn;
};

template <class Function>
bool probe(HMODULE module, const char* name, Function*& function) noexcept
{
	function = reinterpret_cast<Function*>(GetProcAddress(module, name));
	return function != nullptr;
}

// Mirrors of the declarations of powrprof.h and winuser.h, which are not included.
constexpr DWORD device_notify_callback = 2; // DEVICE_NOTIFY_CALLBACK
constexpr ULONG pbt_apm_suspend = 0x4; // PBT_APMSUSPEND
// Sent on every resume, unlike PBT_APMRESUMESUSPEND, which is sent in addition only when the user caused the resume.
constexpr ULONG pbt_apm_resume_automatic = 0x12; // PBT_APMRESUMEAUTOMATIC

using platform_thread_pool_adapter_0_device_notify_callback_routine_fn = ULONG (CALLBACK *)(_In_opt_ PVOID context, _In_ ULONG type, _In_ PVOID setting) noexcept;

// DEVICE_NOTIFY_SUBSCRIBE_PARAMETERS
struct platform_thread_pool_adapter_0_device_notify_subscribe_parameters_0 final
{
	platform_thread_pool_adapter_0_device_notify_callback_routine_fn callback;
	PVOID context;
};

using platform_thread_pool_adapter_0_functions_0_power_register_suspend_resume_notification_fn = DWORD (WINAPI *)(_In_ DWORD flags, _In_ HANDLE recipient, _Out_ PVOID* registration_handle) noexcept;
using platform_thread_pool_adapter_0_functions_0_power_unregister_suspend_resume_notification_fn = DWORD (WINAPI *)(_In_ PVOID registration_handle) noexcept;

// Loads powrprof.dll (Windows 8 and later) and resolves its suspend and resume notification functions. On success, the
// caller owns powrprof and must free it with FreeLibrary().
result_0 probe_suspend_resume_functions(
	HMODULE& powrprof,
	platform_thread_pool_adapter_0_functions_0_power_register_suspend_resume_notification_fn& power_register_suspend_resume_notification_fn,
	platform_thread_pool_adapter_0_functions_0_power_unregister_suspend_resume_notification_fn& power_unregister_suspend_resume_notification_fn
) noexcept
{
	powrprof = LoadLibraryExW(L"powrprof.dll", NULL, LOAD_LIBRARY_SEARCH_SYSTEM32);
	if (powrprof == NULL) {
		return result_0::error_not_supported;
	}
	bool ok = true;
	ok &= probe(powrprof, "PowerRegisterSuspendResumeNotification", power_register_suspend_resume_notification_fn);
	ok &= probe(powrprof, "PowerUnregisterSuspendResumeNotification", power_unregister_suspend_resume_notification_fn);
	if (!ok) {
		FreeLibrary(powrprof);
		powrprof = NULL;
		return result_0::error_not_supported;
	}
	return result_0::success;
}

result_0 probe_functions(platform_thread_pool_adapter_0_functions_0& functions) noexcept
{
	HMODULE kernel32 = GetModuleHandleW(L"kernel32.dll");
	if (kernel32 == NULL) {
		return result_0::error_not_supported;
	}
	bool ok = true;
	ok &= probe(kernel32, "QueueUserWorkItem", functions.queue_user_work_item_fn);
	//ok &= probe(kernel32, "CreateTimerQueue", functions.create_timer_queue_fn);
	//ok &= probe(kernel32, "DeleteTimerQueueEx", functions.delete_timer_queue_ex_fn);
	ok &= probe(kernel32, "CreateTimerQueueTimer", functions.create_timer_queue_timer_fn);
	// Optional: selects platform_thread_pool_adapter_0_timer_0_impl_1 when available, platform_thread_pool_adapter_0_timer_0_impl_0 otherwise.
	probe(kernel32, "ChangeTimerQueueTimer", functions.change_timer_queue_timer_fn);
	ok &= probe(kernel32, "DeleteTimerQueueTimer", functions.delete_timer_queue_timer_fn);
	ok &= probe(kernel32, "GetLastError", functions.get_last_error_fn);
	return ok ? result_0::success : result_0::error_not_supported;
}

#pragma region Suspend watcher

// Both completion callback types of a suspend watcher have this signature.
using platform_thread_pool_adapter_0_suspend_watcher_0_completion_fn = void (*)(void* data, co_result_0 status, size_t generation) noexcept;

static_assert(std::is_same_v<co_suspend_watcher_0_wait_suspend_0_completion_fn, platform_thread_pool_adapter_0_suspend_watcher_0_completion_fn>);
static_assert(std::is_same_v<co_suspend_watcher_0_wait_resume_0_completion_fn, platform_thread_pool_adapter_0_suspend_watcher_0_completion_fn>);

enum class platform_thread_pool_adapter_0_suspend_watcher_0_event_0 : size_t
{
	suspend = 0,
	resume = 1,
};

class platform_thread_pool_adapter_0_suspend_watcher_0_impl_0;

// A pending wait. Allocated by the wait call, so that completing it never allocates, and deallocated by its completion.
struct platform_thread_pool_adapter_0_suspend_watcher_0_wait_0 final
{
	platform_thread_pool_adapter_0_suspend_watcher_0_impl_0* suspend_watcher;
	platform_thread_pool_adapter_0_suspend_watcher_0_event_0 event;
	// Registered while the wait is in the list of its suspend watcher, unless NULL.
	co_cancellation_0 cancellation;
	co_cancellation_0_registration_0 registration;
	co_executor_0 executor;
	platform_thread_pool_adapter_0_suspend_watcher_0_completion_fn completion_fn;
	void* data;
	co_result_0 status;
	// The generation the wait was given; once completed, the generation passed to the completion callback.
	size_t generation;
	co_allocation_callbacks_0_deallocate_fn deallocate_fn;
	void* deallocate_data;
	// Links of the list of pending waits of the suspend watcher; once taken from it, next links the waits to complete.
	platform_thread_pool_adapter_0_suspend_watcher_0_wait_0* prev;
	platform_thread_pool_adapter_0_suspend_watcher_0_wait_0* next;
	bool linked;
};

// Intrusive list of the pending waits of one event of a suspend watcher.
class platform_thread_pool_adapter_0_suspend_watcher_0_wait_list_0 final
{
public:

	bool empty() const noexcept
	{
		return head == nullptr;
	}

	void push(platform_thread_pool_adapter_0_suspend_watcher_0_wait_0* wait) noexcept
	{
		wait->prev = nullptr;
		wait->next = head;
		if (head != nullptr) {
			head->prev = wait;
		}
		head = wait;
		wait->linked = true;
	}

	void remove(platform_thread_pool_adapter_0_suspend_watcher_0_wait_0* wait) noexcept
	{
		assert(wait->linked);
		if (wait->prev != nullptr) {
			wait->prev->next = wait->next;
		}
		else {
			head = wait->next;
		}
		if (wait->next != nullptr) {
			wait->next->prev = wait->prev;
		}
		wait->prev = nullptr;
		wait->next = nullptr;
		wait->linked = false;
	}

	// Empties the list, gives every wait the status and generation and prepends them to the waits to complete; returns
	// the new head.
	platform_thread_pool_adapter_0_suspend_watcher_0_wait_0* take_all(co_result_0 status, size_t generation, platform_thread_pool_adapter_0_suspend_watcher_0_wait_0* completed) noexcept
	{
		while (head != nullptr) {
			platform_thread_pool_adapter_0_suspend_watcher_0_wait_0* wait = head;
			head = wait->next;
			wait->prev = nullptr;
			wait->next = completed;
			wait->linked = false;
			wait->status = status;
			wait->generation = generation;
			completed = wait;
		}
		return completed;
	}

private:
	platform_thread_pool_adapter_0_suspend_watcher_0_wait_0* head = nullptr;
};

class platform_thread_pool_adapter_0_suspend_watcher_0_service_0_impl_0;

class platform_thread_pool_adapter_0_suspend_watcher_0_impl_0 : protected co_suspend_watcher_0_t
{
public:

	platform_thread_pool_adapter_0_suspend_watcher_0_impl_0(
		const co_allocation_callbacks_0& allocation_callbacks,
		platform_thread_pool_adapter_0_suspend_watcher_0_service_0_impl_0* service
	) noexcept
		: allocation_callbacks({
			.structure_type = allocation_callbacks.structure_type,
			.next_structure = nullptr,
			.allocate_fn = allocation_callbacks.allocate_fn,
			.deallocate_fn = allocation_callbacks.deallocate_fn,
			.data = allocation_callbacks.data
			})
		, service(service)
	{
		co_suspend_watcher_0_init(this, &suspend_watcher_vtable);
	}

	~platform_thread_pool_adapter_0_suspend_watcher_0_impl_0()
	{
		assert(waits[0].empty() && waits[1].empty() && "a suspend watcher may be destroyed only when no wait is pending");
	}

	// Called by the service, with its mutex held, when the event happened. Advances the generation of the event,
	// prepends the waits it completes (all pending waits of the event, which wait after the previous generation) to the
	// waits to complete and returns the new head.
	platform_thread_pool_adapter_0_suspend_watcher_0_wait_0* notify(platform_thread_pool_adapter_0_suspend_watcher_0_event_0 event, platform_thread_pool_adapter_0_suspend_watcher_0_wait_0* completed) noexcept
	{
		std::lock_guard lock(mutex);
		const size_t generation = ++generations[static_cast<size_t>(event)];
		return waits[static_cast<size_t>(event)].take_all(CO_RESULT_0_SUCCESS, generation, completed);
	}

	// Completes the waits taken from their lists, linked through next: unregisters their cancellations and posts them.
	// Must be called without any lock held, since the executor may invoke the completions right away.
	static void complete(platform_thread_pool_adapter_0_suspend_watcher_0_wait_0* completed) noexcept
	{
		while (completed != nullptr) {
			platform_thread_pool_adapter_0_suspend_watcher_0_wait_0* wait = completed;
			// Once posted, the wait may be deallocated.
			completed = wait->next;
			if (wait->cancellation != nullptr) {
				// Waits for the cancellation callback of the wait if it is running; it finds the wait unlinked and returns.
				co_cancellation_0_unregister_0(wait->cancellation, &wait->registration);
			}
			post_or_terminate(wait);
		}
	}

	// Links of the list of suspend watchers of the service, guarded by the mutex of the service.
	platform_thread_pool_adapter_0_suspend_watcher_0_impl_0* service_prev = nullptr;
	platform_thread_pool_adapter_0_suspend_watcher_0_impl_0* service_next = nullptr;

private:

	static void destroy(platform_thread_pool_adapter_0_suspend_watcher_0_impl_0* self) noexcept;

	static void get_suspend_generation_0(platform_thread_pool_adapter_0_suspend_watcher_0_impl_0* self, size_t* generation) noexcept
	{
		std::lock_guard lock(self->mutex);
		*generation = self->generations[static_cast<size_t>(platform_thread_pool_adapter_0_suspend_watcher_0_event_0::suspend)];
	}

	static void get_resume_generation_0(platform_thread_pool_adapter_0_suspend_watcher_0_impl_0* self, size_t* generation) noexcept
	{
		std::lock_guard lock(self->mutex);
		*generation = self->generations[static_cast<size_t>(platform_thread_pool_adapter_0_suspend_watcher_0_event_0::resume)];
	}

	static co_result_0 wait_suspend_0(platform_thread_pool_adapter_0_suspend_watcher_0_impl_0* self, size_t after_generation, co_cancellation_0 cancellation, co_executor_0 executor, co_suspend_watcher_0_wait_suspend_0_completion_fn completion_fn, void* data) noexcept
	{
		return self->wait(platform_thread_pool_adapter_0_suspend_watcher_0_event_0::suspend, after_generation, cancellation, executor, completion_fn, data);
	}

	static co_result_0 wait_resume_0(platform_thread_pool_adapter_0_suspend_watcher_0_impl_0* self, size_t after_generation, co_cancellation_0 cancellation, co_executor_0 executor, co_suspend_watcher_0_wait_resume_0_completion_fn completion_fn, void* data) noexcept
	{
		return self->wait(platform_thread_pool_adapter_0_suspend_watcher_0_event_0::resume, after_generation, cancellation, executor, completion_fn, data);
	}

	static co_result_0 cancel_0(platform_thread_pool_adapter_0_suspend_watcher_0_impl_0* self) noexcept
	{
		platform_thread_pool_adapter_0_suspend_watcher_0_wait_0* completed = nullptr;
		{
			std::lock_guard lock(self->mutex);
			// Pending waits wait after the current generation, which is unchanged.
			completed = self->waits[0].take_all(CO_RESULT_0_CANCELLED, self->generations[0], completed);
			completed = self->waits[1].take_all(CO_RESULT_0_CANCELLED, self->generations[1], completed);
		}
		complete(completed);
		return CO_RESULT_0_SUCCESS;
	}

	co_result_0 wait(platform_thread_pool_adapter_0_suspend_watcher_0_event_0 event, size_t after_generation, co_cancellation_0 wait_cancellation, co_executor_0 executor, platform_thread_pool_adapter_0_suspend_watcher_0_completion_fn completion_fn, void* data) noexcept
	{
		platform_thread_pool_adapter_0_suspend_watcher_0_wait_0* wait = impl::new_object_0<platform_thread_pool_adapter_0_suspend_watcher_0_wait_0>(allocation_callbacks);
		if (!wait) {
			return CO_RESULT_0_ERROR_OUT_OF_MEMORY;
		}
		wait->suspend_watcher = this;
		wait->event = event;
		wait->cancellation = nullptr;
		wait->registration = {
			.callback_fn = &platform_thread_pool_adapter_0_suspend_watcher_0_impl_0::cancelled,
			.data = wait,
		};
		wait->executor = executor;
		wait->completion_fn = completion_fn;
		wait->data = data;
		wait->status = CO_RESULT_0_SUCCESS;
		wait->generation = after_generation;
		wait->deallocate_fn = allocation_callbacks.deallocate_fn;
		wait->deallocate_data = allocation_callbacks.data;
		wait->prev = nullptr;
		wait->next = nullptr;
		wait->linked = false;
		std::unique_lock lock(mutex);
		const size_t generation = generations[static_cast<size_t>(event)];
		// Generations wrap around: a wait is behind when its generation differs. Coarse check that it is not ahead.
		assert(static_cast<std::make_signed_t<size_t>>(generation - after_generation) >= 0 && "a wait may not be given a generation ahead of the current one");
		if (after_generation != generation) {
			lock.unlock();
			wait->generation = generation;
			if (co_result_0 result = post(wait); result != CO_RESULT_0_SUCCESS) {
				impl::delete_object_0(allocation_callbacks, wait);
				return result;
			}
			return CO_RESULT_0_SUCCESS;
		}
		// Register with the lock held, so that the cancellation callback, which takes the lock, finds the wait linked.
		if (wait_cancellation != nullptr) {
			if (!co_cancellation_0_register_0(wait_cancellation, &wait->registration)) {
				lock.unlock();
				wait->status = CO_RESULT_0_CANCELLED;
				if (co_result_0 result = post(wait); result != CO_RESULT_0_SUCCESS) {
					impl::delete_object_0(allocation_callbacks, wait);
					return result;
				}
				return CO_RESULT_0_SUCCESS;
			}
			wait->cancellation = wait_cancellation;
		}
		waits[static_cast<size_t>(event)].push(wait);
		return CO_RESULT_0_SUCCESS;
	}

	static void cancelled(void* data) noexcept
	{
		platform_thread_pool_adapter_0_suspend_watcher_0_wait_0* wait = static_cast<platform_thread_pool_adapter_0_suspend_watcher_0_wait_0*>(data);
		platform_thread_pool_adapter_0_suspend_watcher_0_impl_0* self = wait->suspend_watcher;
		{
			std::lock_guard lock(self->mutex);
			if (!wait->linked) {
				// Already taken by an event or cancel_0(), which completes it.
				return;
			}
			self->waits[static_cast<size_t>(wait->event)].remove(wait);
		}
		// This callback is the registration; it is not unregistered.
		wait->cancellation = nullptr;
		wait->status = CO_RESULT_0_CANCELLED;
		post_or_terminate(wait);
	}

	static co_result_0 post(platform_thread_pool_adapter_0_suspend_watcher_0_wait_0* wait) noexcept
	{
		return co_executor_0_post_0(wait->executor, {.invoke_fn = &platform_thread_pool_adapter_0_suspend_watcher_0_impl_0::invoke, .data = wait});
	}

	static void post_or_terminate(platform_thread_pool_adapter_0_suspend_watcher_0_wait_0* wait) noexcept
	{
		if (post(wait) != CO_RESULT_0_SUCCESS) {
			// TODO: Investigate if we can do here anything else than terminate.
			std::terminate();
		}
	}

	static void invoke(void* data) noexcept
	{
		platform_thread_pool_adapter_0_suspend_watcher_0_wait_0* wait = static_cast<platform_thread_pool_adapter_0_suspend_watcher_0_wait_0*>(data);
		platform_thread_pool_adapter_0_suspend_watcher_0_completion_fn completion_fn = wait->completion_fn;
		void* completion_data = wait->data;
		co_result_0 status = wait->status;
		size_t generation = wait->generation;
		co_allocation_callbacks_0_deallocate_fn deallocate_fn = wait->deallocate_fn;
		void* deallocate_data = wait->deallocate_data;
		// Deallocate first: the completion may destroy the suspend watcher and the allocation callbacks with it.
		std::destroy_at(wait);
		deallocate_fn(deallocate_data, wait, sizeof(platform_thread_pool_adapter_0_suspend_watcher_0_wait_0), alignof(platform_thread_pool_adapter_0_suspend_watcher_0_wait_0));
		completion_fn(completion_data, status, generation);
	}

private:
	static const co_suspend_watcher_0_vtable suspend_watcher_vtable;
	const co_allocation_callbacks_0 allocation_callbacks;
	platform_thread_pool_adapter_0_suspend_watcher_0_service_0_impl_0* const service;
	std::mutex mutex;
	// Indexed by platform_thread_pool_adapter_0_suspend_watcher_0_event_0.
	platform_thread_pool_adapter_0_suspend_watcher_0_wait_list_0 waits[2];
	size_t generations[2] = {0, 0};
};

const co_suspend_watcher_0_vtable platform_thread_pool_adapter_0_suspend_watcher_0_impl_0::suspend_watcher_vtable = {
	.api_version = CO_API_VERSION_0,
	.destroy = reinterpret_cast<co_suspend_watcher_0_destroy_fn>(&platform_thread_pool_adapter_0_suspend_watcher_0_impl_0::destroy),
	.get_suspend_generation_0 = reinterpret_cast<co_suspend_watcher_0_get_suspend_generation_0_fn>(&platform_thread_pool_adapter_0_suspend_watcher_0_impl_0::get_suspend_generation_0),
	.get_resume_generation_0 = reinterpret_cast<co_suspend_watcher_0_get_resume_generation_0_fn>(&platform_thread_pool_adapter_0_suspend_watcher_0_impl_0::get_resume_generation_0),
	.wait_suspend_0 = reinterpret_cast<co_suspend_watcher_0_wait_suspend_0_fn>(&platform_thread_pool_adapter_0_suspend_watcher_0_impl_0::wait_suspend_0),
	.wait_resume_0 = reinterpret_cast<co_suspend_watcher_0_wait_resume_0_fn>(&platform_thread_pool_adapter_0_suspend_watcher_0_impl_0::wait_resume_0),
	.cancel_0 = reinterpret_cast<co_suspend_watcher_0_cancel_0_fn>(&platform_thread_pool_adapter_0_suspend_watcher_0_impl_0::cancel_0),
};

#pragma endregion

#pragma region Suspend watcher service

// Subscribes once to the suspend and resume notifications of the system and fans them out to its suspend watchers.
class platform_thread_pool_adapter_0_suspend_watcher_0_service_0_impl_0 : protected co_suspend_watcher_0_service_0_t
{
public:

	platform_thread_pool_adapter_0_suspend_watcher_0_service_0_impl_0(
		const co_allocation_callbacks_0& allocation_callbacks,
		HMODULE powrprof,
		platform_thread_pool_adapter_0_functions_0_power_register_suspend_resume_notification_fn power_register_suspend_resume_notification_fn,
		platform_thread_pool_adapter_0_functions_0_power_unregister_suspend_resume_notification_fn power_unregister_suspend_resume_notification_fn
	) noexcept
		: allocation_callbacks({
			.structure_type = allocation_callbacks.structure_type,
			.next_structure = nullptr,
			.allocate_fn = allocation_callbacks.allocate_fn,
			.deallocate_fn = allocation_callbacks.deallocate_fn,
			.data = allocation_callbacks.data
			})
		, powrprof(powrprof)
		, power_register_suspend_resume_notification_fn(power_register_suspend_resume_notification_fn)
		, power_unregister_suspend_resume_notification_fn(power_unregister_suspend_resume_notification_fn)
		, subscribe_parameters{
			.callback = &platform_thread_pool_adapter_0_suspend_watcher_0_service_0_impl_0::notified,
			.context = this,
		}
	{
		co_suspend_watcher_0_service_0_init(this, &service_vtable, &suspend_watcher_service_vtable);
	}

	~platform_thread_pool_adapter_0_suspend_watcher_0_service_0_impl_0()
	{
		assert(head == nullptr && "a suspend watcher service may be destroyed only once all its suspend watchers have been destroyed");
	}

	// Creates the service and subscribes it to the notifications of the system. Fails with
	// CO_RESULT_0_ERROR_NOT_SUPPORTED when the system has no suspend and resume notifications (before Windows 8).
	static co_result_0 create_0(const co_allocation_callbacks_0& allocation_callbacks, co_suspend_watcher_0_service_0* suspend_watcher_service) noexcept
	{
		HMODULE powrprof;
		platform_thread_pool_adapter_0_functions_0_power_register_suspend_resume_notification_fn power_register_suspend_resume_notification_fn;
		platform_thread_pool_adapter_0_functions_0_power_unregister_suspend_resume_notification_fn power_unregister_suspend_resume_notification_fn;
		if (result_0 result = probe_suspend_resume_functions(powrprof, power_register_suspend_resume_notification_fn, power_unregister_suspend_resume_notification_fn); result != result_0::success) {
			return static_cast<co_result_0>(result);
		}
		platform_thread_pool_adapter_0_suspend_watcher_0_service_0_impl_0* service = impl::new_object_0<platform_thread_pool_adapter_0_suspend_watcher_0_service_0_impl_0>(
			allocation_callbacks,
			allocation_callbacks,
			powrprof,
			power_register_suspend_resume_notification_fn,
			power_unregister_suspend_resume_notification_fn
		);
		if (!service) {
			FreeLibrary(powrprof);
			return CO_RESULT_0_ERROR_OUT_OF_MEMORY;
		}
		// Subscribe once the object is at its final address, which the notifications are given.
		if (power_register_suspend_resume_notification_fn(device_notify_callback, &service->subscribe_parameters, &service->registration) != ERROR_SUCCESS) {
			service->registration = nullptr;
			destroy(service);
			return CO_RESULT_0_ERROR_SYSTEM;
		}
		*suspend_watcher_service = CO_SUSPEND_WATCHER_0_SERVICE_0_CAST(service);
		return CO_RESULT_0_SUCCESS;
	}

	void remove(platform_thread_pool_adapter_0_suspend_watcher_0_impl_0* suspend_watcher) noexcept
	{
		std::lock_guard lock(mutex);
		if (suspend_watcher->service_prev != nullptr) {
			suspend_watcher->service_prev->service_next = suspend_watcher->service_next;
		}
		else {
			head = suspend_watcher->service_next;
		}
		if (suspend_watcher->service_next != nullptr) {
			suspend_watcher->service_next->service_prev = suspend_watcher->service_prev;
		}
		suspend_watcher->service_prev = nullptr;
		suspend_watcher->service_next = nullptr;
	}

private:

	static void destroy(platform_thread_pool_adapter_0_suspend_watcher_0_service_0_impl_0* self) noexcept
	{
		if (self->registration != nullptr) {
			// Assumed to wait for a running notification callback, which uses the service.
			self->power_unregister_suspend_resume_notification_fn(self->registration);
		}
		HMODULE powrprof = self->powrprof;
		impl::delete_object_0(self->allocation_callbacks, self);
		FreeLibrary(powrprof);
	}

	static co_result_0 create_suspend_watcher_0(platform_thread_pool_adapter_0_suspend_watcher_0_service_0_impl_0* self, co_suspend_watcher_0* suspend_watcher) noexcept
	{
		platform_thread_pool_adapter_0_suspend_watcher_0_impl_0* suspend_watcher_impl = impl::new_object_0<platform_thread_pool_adapter_0_suspend_watcher_0_impl_0>(
			self->allocation_callbacks,
			self->allocation_callbacks,
			self
		);
		if (!suspend_watcher_impl) {
			return CO_RESULT_0_ERROR_OUT_OF_MEMORY;
		}
		{
			std::lock_guard lock(self->mutex);
			suspend_watcher_impl->service_prev = nullptr;
			suspend_watcher_impl->service_next = self->head;
			if (self->head != nullptr) {
				self->head->service_prev = suspend_watcher_impl;
			}
			self->head = suspend_watcher_impl;
		}
		*suspend_watcher = CO_SUSPEND_WATCHER_0_CAST(suspend_watcher_impl);
		return CO_RESULT_0_SUCCESS;
	}

	static ULONG CALLBACK notified(_In_opt_ PVOID context, _In_ ULONG type, _In_ PVOID /* setting */) noexcept
	{
		platform_thread_pool_adapter_0_suspend_watcher_0_service_0_impl_0* self = static_cast<platform_thread_pool_adapter_0_suspend_watcher_0_service_0_impl_0*>(context);
		platform_thread_pool_adapter_0_suspend_watcher_0_event_0 event;
		switch (type) {
			case pbt_apm_suspend:
				event = platform_thread_pool_adapter_0_suspend_watcher_0_event_0::suspend;
				break;
			case pbt_apm_resume_automatic:
				event = platform_thread_pool_adapter_0_suspend_watcher_0_event_0::resume;
				break;
			default:
				return ERROR_SUCCESS;
		}
		platform_thread_pool_adapter_0_suspend_watcher_0_wait_0* completed = nullptr;
		{
			// Suspend watchers cannot be destroyed while the lock is held, and their taken waits keep them alive after.
			std::lock_guard lock(self->mutex);
			for (platform_thread_pool_adapter_0_suspend_watcher_0_impl_0* suspend_watcher = self->head; suspend_watcher != nullptr; suspend_watcher = suspend_watcher->service_next) {
				completed = suspend_watcher->notify(event, completed);
			}
		}
		platform_thread_pool_adapter_0_suspend_watcher_0_impl_0::complete(completed);
		// Completions are posted before returning, and the system delivers the next notification only after this one
		// returned, which orders the completions of earlier events before those of later ones (see suspend_watcher_0.h).
		// TODO: Check that Windows really serializes these notifications; it is not documented. If it does not, order the
		// events in the service, for example by holding a separate posting mutex across taking and posting the waits, or by
		// numbering the events and posting them in that order.
		return ERROR_SUCCESS;
	}

private:
	static const co_service_0_vtable service_vtable;
	static const co_suspend_watcher_0_service_0_vtable suspend_watcher_service_vtable;
	const co_allocation_callbacks_0 allocation_callbacks;
	const HMODULE powrprof;
	const platform_thread_pool_adapter_0_functions_0_power_register_suspend_resume_notification_fn power_register_suspend_resume_notification_fn;
	const platform_thread_pool_adapter_0_functions_0_power_unregister_suspend_resume_notification_fn power_unregister_suspend_resume_notification_fn;
	platform_thread_pool_adapter_0_device_notify_subscribe_parameters_0 subscribe_parameters;
	PVOID registration = nullptr;
	std::mutex mutex;
	platform_thread_pool_adapter_0_suspend_watcher_0_impl_0* head = nullptr;
};

const co_service_0_vtable platform_thread_pool_adapter_0_suspend_watcher_0_service_0_impl_0::service_vtable = {
	.api_version = CO_API_VERSION_0,
	.destroy = reinterpret_cast<co_service_0_destroy_fn>(&platform_thread_pool_adapter_0_suspend_watcher_0_service_0_impl_0::destroy),
};

const co_suspend_watcher_0_service_0_vtable platform_thread_pool_adapter_0_suspend_watcher_0_service_0_impl_0::suspend_watcher_service_vtable = {
	.create_suspend_watcher_0 = reinterpret_cast<co_suspend_watcher_0_service_0_create_suspend_watcher_0_fn>(&platform_thread_pool_adapter_0_suspend_watcher_0_service_0_impl_0::create_suspend_watcher_0),
};

void platform_thread_pool_adapter_0_suspend_watcher_0_impl_0::destroy(platform_thread_pool_adapter_0_suspend_watcher_0_impl_0* self) noexcept
{
	self->service->remove(self);
	impl::delete_object_0(self->allocation_callbacks, self);
}

#pragma endregion

#pragma region Timer

struct platform_thread_pool_adapter_0_timer_0_completion_handler_0 final
{
	co_timer_0_wait_0_completion_fn completion_fn;
	void* data;
	co_result_0 status;
	co_allocation_callbacks_0_deallocate_fn deallocate_fn;
	void* deallocate_data;
};

class platform_thread_pool_adapter_0_timer_0_impl_0 : protected co_timer_0_t
{
public:

	platform_thread_pool_adapter_0_timer_0_impl_0(
		const co_allocation_callbacks_0& allocation_callbacks,
		co_clock_0 clock,
		platform_thread_pool_adapter_0_functions_0_create_timer_queue_timer_fn create_timer_queue_timer_fn,
		platform_thread_pool_adapter_0_functions_0_delete_timer_queue_timer_fn delete_timer_queue_timer_fn,
		platform_thread_pool_adapter_0_functions_0_get_last_error_fn get_last_error_fn
	) noexcept
		: allocation_callbacks({
			.structure_type = allocation_callbacks.structure_type,
			.next_structure = nullptr,
			.allocate_fn = allocation_callbacks.allocate_fn,
			.deallocate_fn = allocation_callbacks.deallocate_fn,
			.data = allocation_callbacks.data
		})
		, clock(clock)
		, create_timer_queue_timer_fn(create_timer_queue_timer_fn)
		, delete_timer_queue_timer_fn(delete_timer_queue_timer_fn)
		, get_last_error_fn(get_last_error_fn)
		, mutex()
		, timer(NULL)
		, start_executor(nullptr)
#if TIMER_PREALLOCATE_COMPLETION_HANDLER
		, start_completion_handler(nullptr)
#else
		, start_completion_fn(nullptr)
		, start_data(nullptr)
#endif
	{
		co_timer_0_init(this, &timer_vtable);
	}

	~platform_thread_pool_adapter_0_timer_0_impl_0()
	{
		assert(!timer);
		// Don't assert cancelled flag because it may be in any state.
		assert(!start_executor);
#if TIMER_PREALLOCATE_COMPLETION_HANDLER
		assert(!start_completion_handler);
#else
		assert(!start_completion_fn);
		assert(!start_data);
#endif
	}

private:

	static void get_clock_0(platform_thread_pool_adapter_0_timer_0_impl_0* self, co_clock_0* clock) noexcept
	{
		*clock = self->clock;
	}

	static void destroy(platform_thread_pool_adapter_0_timer_0_impl_0* self) noexcept
	{
		impl::delete_object_0(self->allocation_callbacks, self);
	}

	static co_result_0 start_0(platform_thread_pool_adapter_0_timer_0_impl_0* self, uint64_t due_ns, co_executor_0 executor, co_timer_0_wait_0_completion_fn completion_fn, void* data) noexcept
	{
		// Round due_ns up so that 1ns becomes 1ms.
		uint64_t due_ms = due_ns / 1000000 + (due_ns % 1000000 != 0 ? 1 : 0);
		if (due_ms >= INFINITE) {
			return CO_RESULT_0_ERROR_SYSTEM;
		}
#if TIMER_PREALLOCATE_COMPLETION_HANDLER
		// Allocate before creating the timer: once created, the timer may fire and the elapsed callback uses the
		// completion handler, so failing after creating it would leave a running timer behind.
		platform_thread_pool_adapter_0_timer_0_completion_handler_0* completion_handler = impl::new_object_0<platform_thread_pool_adapter_0_timer_0_completion_handler_0>(self->allocation_callbacks);
		if (!completion_handler) {
			return CO_RESULT_0_ERROR_OUT_OF_MEMORY;
		}
		completion_handler->completion_fn = completion_fn;
		completion_handler->data = data;
		// completion_handler->status will be set in elapsed callback or cancel_0
		completion_handler->deallocate_fn = self->allocation_callbacks.deallocate_fn;
		completion_handler->deallocate_data = self->allocation_callbacks.data;
#endif
		// Hold the lock while creating the timer: the elapsed callback deletes the timer through its handle,
		// but CreateTimerQueueTimer() does not guarantee that it writes the handle before the timer starts,
		// so a fast enough callback could otherwise read the handle before it is written.
		std::lock_guard lock(self->mutex);
		assert(!self->timer);
		// Don't assert cancelled flag because it may be in any state.
		assert(!self->start_executor);
#if TIMER_PREALLOCATE_COMPLETION_HANDLER
		assert(!self->start_completion_handler);
#else
		assert(!self->start_completion_fn);
		assert(!self->start_data);
#endif
		// Create the timer before assigning the other members so that assigning them does not delay its start;
		// the callback cannot see them unassigned because it waits for the lock.
		// TODO: Measure it if it's actually true.
		if (self->create_timer_queue_timer_fn(&self->timer, NULL, &platform_thread_pool_adapter_0_timer_0_impl_0::elapsed, self, static_cast<DWORD>(due_ms), 0, WT_EXECUTEONLYONCE) == 0) {
			DWORD last_error = self->get_last_error_fn();
			(void)last_error;
			// Restore sane state.
			self->timer = NULL;
#if TIMER_PREALLOCATE_COMPLETION_HANDLER
			impl::delete_object_0(self->allocation_callbacks, completion_handler);
#endif
			return CO_RESULT_0_ERROR_SYSTEM;
		}
		self->start_executor = executor;
#if TIMER_PREALLOCATE_COMPLETION_HANDLER
		self->start_completion_handler = completion_handler;
#else
		self->start_completion_fn = completion_fn;
		self->start_data = data;
#endif
		return CO_RESULT_0_SUCCESS;
	}

	static co_result_0 cancel_0(platform_thread_pool_adapter_0_timer_0_impl_0* self) noexcept
	{
#if !TIMER_PREALLOCATE_COMPLETION_HANDLER
		// We can access allocation_callbacks without lock because it is fixed once the timer is constructed.
		co_allocation_callbacks_0 allocation_callbacks = self->allocation_callbacks;
#endif
		HANDLE timer;
		co_executor_0 executor;
		platform_thread_pool_adapter_0_timer_0_completion_handler_0* completion_handler;
#if !TIMER_PREALLOCATE_COMPLETION_HANDLER
		co_timer_0_wait_0_completion_fn completion_fn;
		void* data;
#endif
#if 0
		// Alternative (!TIMER_PREALLOCATE_COMPLETION_HANDLER): allocate outside the lock so that an elapsed callback firing
		// meanwhile does not wait for the allocation, at the cost of allocating in vain when there is nothing to cancel:
		//
		//	completion_handler = impl::new_object_0<platform_thread_pool_adapter_0_timer_0_completion_handler_0>(allocation_callbacks);
		//	if (!completion_handler) {
		//		return CO_RESULT_0_ERROR_OUT_OF_MEMORY;
		//	}
		//	bool pending;
		//	{
		//		std::lock_guard lock(self->mutex);
		//		pending = self->start_executor != nullptr;
		//		if (pending) {
		//			timer = std::exchange(self->timer, HANDLE(NULL));
		//			executor = std::exchange(self->start_executor, nullptr);
		//			completion_fn = std::exchange(self->start_completion_fn, nullptr);
		//			data = std::exchange(self->start_data, nullptr);
		//		}
		//	}
		//	if (!pending) {
		//		// Deallocate outside the lock too.
		//		impl::delete_object_0(allocation_callbacks, completion_handler);
		//		return CO_RESULT_0_SUCCESS;
		//	}
#endif
		{
			// Hold the lock while accessing members of the timer: elapsed callback might be running right now.
			std::lock_guard lock(self->mutex);
			// Decide by the pending start, not by the timer: a previous cancel_0() may have deleted the timer
			// but failed to post, and put the start back without a timer so that this call retries the post.
			if (!self->start_executor) {
				// elapsed callback won - he will be responsible for posting completion handler and deleting timer.
				return CO_RESULT_0_SUCCESS;
			}
#if !TIMER_PREALLOCATE_COMPLETION_HANDLER
#if 1
			// Allocate before taking ownership of the pending start so that failing leaves nothing to restore.
			completion_handler = impl::new_object_0<platform_thread_pool_adapter_0_timer_0_completion_handler_0>(allocation_callbacks);
			if (!completion_handler) {
				return CO_RESULT_0_ERROR_OUT_OF_MEMORY;
			}
#endif
#endif
			// Take ownership of everything start_0() established.
			timer = std::exchange(self->timer, HANDLE(NULL));
			executor = std::exchange(self->start_executor, nullptr);
#if TIMER_PREALLOCATE_COMPLETION_HANDLER
			completion_handler = std::exchange(self->start_completion_handler, nullptr);
#else
			completion_fn = std::exchange(self->start_completion_fn, nullptr);
			data = std::exchange(self->start_data, nullptr);
#endif
		}
#if !TIMER_PREALLOCATE_COMPLETION_HANDLER
		completion_handler->completion_fn = completion_fn;
		completion_handler->data = data;
#endif
		completion_handler->status = CO_RESULT_0_CANCELLED;
#if !TIMER_PREALLOCATE_COMPLETION_HANDLER
		completion_handler->deallocate_fn = allocation_callbacks.deallocate_fn;
		completion_handler->deallocate_data = allocation_callbacks.data;
#endif
		// Delete the timer before posting the completion: once posted, the completion may run and destroy
		// it, so self must not be touched after a successful co_executor_0_post_0(). Deleting first also keeps the start
		// pending if the deletion fails.
		// The timer is already deleted (NULL) if a previous cancel_0() failed to post.
		// Wait for the elapsed callback (INVALID_HANDLE_VALUE): it may have fired before this call took the timer and
		// still be about to lock the mutex, which the posted completion may destroy. It finds no timer and returns.
		// Don't hold the lock while waiting: the callback waits for it.
		//
		// NOTE: Non-blocking alternative (DeleteTimerQueueTimer() with CompletionEvent = NULL) - considered, not implemented.
		// Without waiting, the elapsed callback may still be running (ERROR_IO_PENDING) or be dispatched but not have
		// locked the mutex yet, so posting right away could let the completion destroy self while the callback still
		// uses self->mutex (use-after-free). Something else must therefore tell when the callback is done:
		// A. Completion event + registered wait: pass an event as CompletionEvent, RegisterWaitForSingleObject()
		//    (WT_EXECUTEONLYONCE) on it and post the completion from that wait callback.
		//    - Per cancel: an event, a wait registration and a small context allocation; each can fail, and cancel_0()
		//      can return those failures.
		//    - One more function to probe: RegisterWaitForSingleObject (same era as timer queues).
		//    - The wait callback must UnregisterWait() and CloseHandle(); the wait handle must live somewhere that outlives
		//      the posted completion, and it has the same "handle written after the callback may run" race as
		//      CreateTimerQueueTimer() (needs a lock or a second allocation).
		//    - The post moves into a callback, so a failed post must terminate (like in elapsed) instead of
		//      cancel_0() returning the error.
		//    Preferred option if this is ever done, ideally with the event created once per timer rather than per cancel.
		// B. Reference-counted shared state: move the mutex and the pending-start fields into a separately allocated block
		//    passed as the callback parameter; the timer and the armed callback each hold a reference, so destroying the
		//    timer cannot free memory the callback still uses.
		//    - Changes the object layout and start_0/cancel_0/elapsed/destroy, adds an atomic refcount and one allocation
		//      per timer (can be done at timer creation, so no new run-time failure).
		//    - The hard part is deciding who drops the callback's reference. It is only safe if a non-blocking delete
		//      returning TRUE guarantees the callback never runs, and ERROR_IO_PENDING guarantees it runs (or is running)
		//      and drops it. The documentation is not explicit enough about queued-but-not-started callbacks to rely on
		//      that; without the guarantee, the event from A is needed anyway.
		// Why not now: the wait only lasts as long as an in-flight elapsed callback (lock, non-waiting delete, post), longer
		// only if it waits for a contended lock. Blocking is safe: the lock is released before waiting and cancel_0() never
		// runs inside its own timer callback (where a waiting delete would deadlock). Worth doing only if cancel_0() must
		// never block (e.g. called from a latency-sensitive event loop thread).
		if (timer != NULL && self->delete_timer_queue_timer_fn(NULL, timer, INVALID_HANDLE_VALUE) == 0) {
			DWORD last_error = self->get_last_error_fn();
			(void)last_error;
			{
				// Hold the lock while accessing members of the timer: elapsed callback might be running right now.
				std::lock_guard lock(self->mutex);
				// Put everything back so user can call cancel_0() again. If the timer fired meanwhile, its callback
				// found no timer and returned, so the start stays pending until cancel_0() succeeds.
				self->timer = timer;
				self->start_executor = executor;
#if TIMER_PREALLOCATE_COMPLETION_HANDLER
				self->start_completion_handler = completion_handler;
#else
				self->start_completion_fn = completion_fn;
				self->start_data = data;
#endif
			}
#if !TIMER_PREALLOCATE_COMPLETION_HANDLER
			impl::delete_object_0(allocation_callbacks, completion_handler);
#endif
			return CO_RESULT_0_ERROR_SYSTEM;
		}
		if (co_result_0 result = co_executor_0_post_0(executor, {.invoke_fn = &platform_thread_pool_adapter_0_timer_0_impl_0::invoke, .data = completion_handler}); result != CO_RESULT_0_SUCCESS) {
#if !TIMER_PREALLOCATE_COMPLETION_HANDLER
			impl::delete_object_0(allocation_callbacks, completion_handler);
#endif
			{
				// Hold the lock while accessing members of the timer: elapsed callback might be running right now.
				std::lock_guard lock(self->mutex);
				// Put everything back, except the already deleted timer, so user can call cancel_0() again;
				// it will skip the deletion and only retry the post.
				self->start_executor = executor;
#if TIMER_PREALLOCATE_COMPLETION_HANDLER
				self->start_completion_handler = completion_handler;
#else
				self->start_completion_fn = completion_fn;
				self->start_data = data;
#endif
			}
			return result;
		}
		return CO_RESULT_0_SUCCESS;
	}

private:

	static VOID CALLBACK elapsed(_In_ PVOID parameter, _In_ BOOLEAN /* timer_or_wait_fired */) noexcept
	{
		platform_thread_pool_adapter_0_timer_0_impl_0* self = static_cast<platform_thread_pool_adapter_0_timer_0_impl_0*>(parameter);

		// We can access allocation_callbacks without lock because it is fixed once the timer is constructed.
		co_allocation_callbacks_0 allocation_callbacks = self->allocation_callbacks;
		HANDLE timer;
		co_executor_0 executor;
#if TIMER_PREALLOCATE_COMPLETION_HANDLER
		platform_thread_pool_adapter_0_timer_0_completion_handler_0* completion_handler;
#else
		co_timer_0_wait_0_completion_fn completion_fn;
		void* data;
#endif
		{
			std::lock_guard lock(self->mutex);
			if (self->timer == NULL) {
				// cancel_0 won - it is responsible for posting completion handler and deleting timer.
				// Leave the pending start alone: a cancel_0() that failed to post has put it back to retry the post.
				return;
			}
			timer = std::exchange(self->timer, HANDLE(NULL));
			executor = std::exchange(self->start_executor, nullptr);
#if TIMER_PREALLOCATE_COMPLETION_HANDLER
			completion_handler = std::exchange(self->start_completion_handler, nullptr);
#else
			completion_fn = std::exchange(self->start_completion_fn, nullptr);
			data = std::exchange(self->start_data, nullptr);
#endif
			// You may be tempted to call co_executor_0_post_0 while still holding a lock so we could post faster and release lock later,
			// but once posted, the completion may run and destroy the timer, including this mutex.
		}
		// TODO: In theory, if something below fails (deleting the timer, posting the completion), this callback could turn
		// into a long-running one and retry in a loop, with sleeps in between, instead of terminating.
		// Investigate if it's what we are actually want to do.
#if !TIMER_PREALLOCATE_COMPLETION_HANDLER
		platform_thread_pool_adapter_0_timer_0_completion_handler_0* completion_handler = impl::new_object_0<platform_thread_pool_adapter_0_timer_0_completion_handler_0>(allocation_callbacks);
		if (!completion_handler) {
			// TODO: Investigate if we want to terminate or lead to undefined behavior but win some performance.
			std::terminate();
		}
		completion_handler->completion_fn = completion_fn;
		completion_handler->data = data;
#endif
		completion_handler->status = CO_RESULT_0_SUCCESS;
#if !TIMER_PREALLOCATE_COMPLETION_HANDLER
		completion_handler->deallocate_fn = allocation_callbacks.deallocate_fn;
		completion_handler->deallocate_data = allocation_callbacks.data;
#endif
		// Delete the timer before posting the completion: once posted, the completion may run and destroy
		// it, so self must not be touched after a successful co_executor_0_post_0().
		if (BOOL deleted = self->delete_timer_queue_timer_fn(NULL, timer, NULL); deleted == 0) {
			// According to documentation "ERROR_IO_PENDING" is ok. I don't know why but ok.
			if (DWORD last_error = self->get_last_error_fn(); last_error != ERROR_IO_PENDING) {
				// TODO: Investigate if we want to terminate, leak timers or post to executor a retry.
				std::terminate();
			}
		}
		if (co_result_0 result = co_executor_0_post_0(executor, {.invoke_fn = &platform_thread_pool_adapter_0_timer_0_impl_0::invoke, .data = completion_handler}); result != CO_RESULT_0_SUCCESS) {
			//impl::delete_object_0(allocation_callbacks, completion_handler);
			// TODO: Investigate if we can do here anything else than terminate.
			std::terminate();
		}
	}

	static void invoke(void* data) noexcept
	{
		platform_thread_pool_adapter_0_timer_0_completion_handler_0* completion_handler = static_cast<platform_thread_pool_adapter_0_timer_0_completion_handler_0*>(data);
		completion_handler->completion_fn(completion_handler->data, completion_handler->status);
		// TODO: refactor it into some helper function that doesn't take whole co_allocation_callbacks_0
		co_allocation_callbacks_0_deallocate_fn deallocate_fn = completion_handler->deallocate_fn;
		void* deallocate_data = completion_handler->deallocate_data;
		std::destroy_at(completion_handler);
		deallocate_fn(deallocate_data, completion_handler, sizeof(platform_thread_pool_adapter_0_timer_0_completion_handler_0), alignof(platform_thread_pool_adapter_0_timer_0_completion_handler_0));
	}

private:
	static const co_timer_0_vtable timer_vtable;
	const co_allocation_callbacks_0 allocation_callbacks;
	const co_clock_0 clock;
	const platform_thread_pool_adapter_0_functions_0_create_timer_queue_timer_fn create_timer_queue_timer_fn;
	const platform_thread_pool_adapter_0_functions_0_delete_timer_queue_timer_fn delete_timer_queue_timer_fn;
	const platform_thread_pool_adapter_0_functions_0_get_last_error_fn get_last_error_fn;
	std::mutex mutex;
	HANDLE timer;
	co_executor_0 start_executor;
#if TIMER_PREALLOCATE_COMPLETION_HANDLER
	platform_thread_pool_adapter_0_timer_0_completion_handler_0* start_completion_handler;
#else
	co_timer_0_wait_0_completion_fn start_completion_fn;
	void* start_data;
#endif
};

// TODO: Port to expiry_at_0/expiry_after_0/wait_0; start_0 above is the old implementation, kept for the port.
static co_result_0 platform_thread_pool_adapter_0_timer_0_expiry_at_0_not_supported(co_timer_0, int64_t) noexcept
{
	return CO_RESULT_0_ERROR_NOT_SUPPORTED;
}

static co_result_0 platform_thread_pool_adapter_0_timer_0_expiry_after_0_not_supported(co_timer_0, uint64_t) noexcept
{
	return CO_RESULT_0_ERROR_NOT_SUPPORTED;
}

static co_result_0 platform_thread_pool_adapter_0_timer_0_wait_0_not_supported(co_timer_0, co_cancellation_0, co_executor_0, co_timer_0_wait_0_completion_fn, void*) noexcept
{
	return CO_RESULT_0_ERROR_NOT_SUPPORTED;
}

const co_timer_0_vtable platform_thread_pool_adapter_0_timer_0_impl_0::timer_vtable = {
	.api_version = CO_API_VERSION_0,
	.destroy = reinterpret_cast<co_timer_0_destroy_fn>(&platform_thread_pool_adapter_0_timer_0_impl_0::destroy),
	.get_clock_0 = reinterpret_cast<co_timer_0_get_clock_0_fn>(&platform_thread_pool_adapter_0_timer_0_impl_0::get_clock_0),
	.expiry_at_0 = &platform_thread_pool_adapter_0_timer_0_expiry_at_0_not_supported,
	.expiry_after_0 = &platform_thread_pool_adapter_0_timer_0_expiry_after_0_not_supported,
	.wait_0 = &platform_thread_pool_adapter_0_timer_0_wait_0_not_supported,
	.cancel_0 = reinterpret_cast<co_timer_0_cancel_0_fn>(&platform_thread_pool_adapter_0_timer_0_impl_0::cancel_0),
};

class platform_thread_pool_adapter_0_timer_0_impl_1 : protected co_timer_0_t
{
public:

	platform_thread_pool_adapter_0_timer_0_impl_1(
		const co_allocation_callbacks_0& allocation_callbacks,
		co_clock_0 clock,
		platform_thread_pool_adapter_0_functions_0_create_timer_queue_timer_fn create_timer_queue_timer_fn,
		platform_thread_pool_adapter_0_functions_0_change_timer_queue_timer_fn change_timer_queue_timer_fn,
		platform_thread_pool_adapter_0_functions_0_delete_timer_queue_timer_fn delete_timer_queue_timer_fn,
		platform_thread_pool_adapter_0_functions_0_get_last_error_fn get_last_error_fn
	) noexcept
		: allocation_callbacks({
			.structure_type = allocation_callbacks.structure_type,
			.next_structure = nullptr,
			.allocate_fn = allocation_callbacks.allocate_fn,
			.deallocate_fn = allocation_callbacks.deallocate_fn,
			.data = allocation_callbacks.data
		})
		, clock(clock)
		, create_timer_queue_timer_fn(create_timer_queue_timer_fn)
		, change_timer_queue_timer_fn(change_timer_queue_timer_fn)
		, delete_timer_queue_timer_fn(delete_timer_queue_timer_fn)
		, get_last_error_fn(get_last_error_fn)
		, mutex()
		, timer(NULL)
		, cancelled(false)
		, start_executor(nullptr)
#if TIMER_PREALLOCATE_COMPLETION_HANDLER
		, start_completion_handler(nullptr)
#else
		, start_completion_fn(nullptr)
		, start_data(nullptr)
#endif
	{
		co_timer_0_init(this, &timer_vtable);
	}

	~platform_thread_pool_adapter_0_timer_0_impl_1()
	{
		assert(!timer);
		// Don't assert cancelled flag because it may be in any state.
		assert(!start_executor);
#if TIMER_PREALLOCATE_COMPLETION_HANDLER
		assert(!start_completion_handler);
#else
		assert(!start_completion_fn);
		assert(!start_data);
#endif
	}

private:

	static void get_clock_0(platform_thread_pool_adapter_0_timer_0_impl_1* self, co_clock_0* clock) noexcept
	{
		*clock = self->clock;
	}

	static void destroy(platform_thread_pool_adapter_0_timer_0_impl_1* self) noexcept
	{
		impl::delete_object_0(self->allocation_callbacks, self);
	}

	static co_result_0 start_0(platform_thread_pool_adapter_0_timer_0_impl_1* self, uint64_t due_ns, co_executor_0 executor, co_timer_0_wait_0_completion_fn completion_fn, void* data) noexcept
	{
		// Round due_ns up so that 1ns becomes 1ms.
		uint64_t due_ms = due_ns / 1000000 + (due_ns % 1000000 != 0 ? 1 : 0);
		if (due_ms >= INFINITE) {
			return CO_RESULT_0_ERROR_SYSTEM;
		}
#if TIMER_PREALLOCATE_COMPLETION_HANDLER
		// Allocate before creating the timer: once created, the timer may fire and the elapsed callback uses the
		// completion handler, so failing after creating it would leave a running timer behind.
		platform_thread_pool_adapter_0_timer_0_completion_handler_0* completion_handler = impl::new_object_0<platform_thread_pool_adapter_0_timer_0_completion_handler_0>(self->allocation_callbacks);
		if (!completion_handler) {
			return CO_RESULT_0_ERROR_OUT_OF_MEMORY;
		}
		completion_handler->completion_fn = completion_fn;
		completion_handler->data = data;
		// completion_handler->status will be set in elapsed callback or cancel_0
		completion_handler->deallocate_fn = self->allocation_callbacks.deallocate_fn;
		completion_handler->deallocate_data = self->allocation_callbacks.data;
#endif
		// Hold the lock while creating the timer: the elapsed callback deletes the timer through its handle,
		// but CreateTimerQueueTimer() does not guarantee that it writes the handle before the timer starts,
		// so a fast enough callback could otherwise read the handle before it is written.
		std::lock_guard lock(self->mutex);
		assert(!self->timer);
		// Don't assert cancelled flag because it may be in any state.
		assert(!self->start_executor);
#if TIMER_PREALLOCATE_COMPLETION_HANDLER
		assert(!self->start_completion_handler);
#else
		assert(!self->start_completion_fn);
		assert(!self->start_data);
#endif
		// Create the timer before assigning the other members so that assigning them does not delay its start;
		// the callback cannot see them unassigned because it waits for the lock.
		// TODO: Measure it if it's actually true.
		if (self->create_timer_queue_timer_fn(&self->timer, NULL, &platform_thread_pool_adapter_0_timer_0_impl_1::elapsed, self, static_cast<DWORD>(due_ms), 0, WT_EXECUTEONLYONCE) == 0) {
			DWORD last_error = self->get_last_error_fn();
			(void)last_error;
			// Restore sane state.
			self->timer = NULL;
#if TIMER_PREALLOCATE_COMPLETION_HANDLER
			impl::delete_object_0(self->allocation_callbacks, completion_handler);
#endif
			return CO_RESULT_0_ERROR_SYSTEM;
		}
		self->cancelled = false;
		self->start_executor = executor;
#if TIMER_PREALLOCATE_COMPLETION_HANDLER
		self->start_completion_handler = completion_handler;
#else
		self->start_completion_fn = completion_fn;
		self->start_data = data;
#endif
		return CO_RESULT_0_SUCCESS;
	}

	static co_result_0 cancel_0(platform_thread_pool_adapter_0_timer_0_impl_1* self) noexcept
	{
#if !TIMER_PREALLOCATE_COMPLETION_HANDLER
		// We can access allocation_callbacks without lock because it is fixed once the timer is constructed.
		co_allocation_callbacks_0 allocation_callbacks = self->allocation_callbacks;
#endif
		HANDLE timer;
		co_executor_0 executor;
		platform_thread_pool_adapter_0_timer_0_completion_handler_0* completion_handler;
#if !TIMER_PREALLOCATE_COMPLETION_HANDLER
		co_timer_0_wait_0_completion_fn completion_fn;
		void* data;
#endif
		{
			// Hold the lock while accessing members of the timer: elapsed callback might be running right now.
			std::lock_guard lock(self->mutex);
			// Decide by the pending start, not by the timer: a previous cancel_0() may have deleted the timer
			// but failed to post, and put the start back without a timer so that this call retries the post.
			if (!self->start_executor) {
				// elapsed callback won - he will be responsible for posting completion handler and deleting timer.
				return CO_RESULT_0_SUCCESS;
			}
			if (self->cancelled) {
				// Already cancelled and elapsed callback already forced.
				return CO_RESULT_0_SUCCESS;
			}
			timer = self->timer;
			if (timer != NULL) {
				// Force elapsed callback to fire immediately (in its thread) - it will check cancelled flag.
				self->cancelled = true;
				if (BOOL changed = self->change_timer_queue_timer_fn(NULL, timer, 0, 0); changed != 0) {
					return CO_RESULT_0_SUCCESS;
				}
				// Expected when the one-shot timer has already expired and its callback is about to run (waiting for the lock).
				DWORD last_error = self->get_last_error_fn();
				(void)last_error;
				// Don't wait for the callback to report the cancellation (possibly only at the due time): report it from here,
				// like platform_thread_pool_adapter_0_timer_0_impl_0 does.
				self->cancelled = false;
			}
#if !TIMER_PREALLOCATE_COMPLETION_HANDLER
#if 0
			// Alternative: allocate outside the lock so that an elapsed callback firing meanwhile does not wait for the
			// allocation. Take the lock as std::unique_lock and release it for the allocation; then check the state again,
			// because the elapsed callback or another cancel_0() may have taken the pending start meanwhile:
			//
			//	lock.unlock();
			//	completion_handler = impl::new_object_0<platform_thread_pool_adapter_0_timer_0_completion_handler_0>(allocation_callbacks);
			//	if (!completion_handler) {
			//		return CO_RESULT_0_ERROR_OUT_OF_MEMORY;
			//	}
			//	lock.lock();
			//	if (!self->start_executor || self->cancelled) {
			//		lock.unlock();
			//		// Deallocate outside the lock too.
			//		impl::delete_object_0(allocation_callbacks, completion_handler);
			//		return CO_RESULT_0_SUCCESS;
			//	}
			//	// The timer may have been deleted meanwhile by a cancel_0() that then failed to post.
			//	timer = self->timer;
#else
			// Allocate before taking ownership of the pending start so that failing leaves nothing to restore.
			completion_handler = impl::new_object_0<platform_thread_pool_adapter_0_timer_0_completion_handler_0>(allocation_callbacks);
			if (!completion_handler) {
				return CO_RESULT_0_ERROR_OUT_OF_MEMORY;
			}
#endif
#endif
			// Take ownership of everything start_0() established.
			self->timer = NULL;
			executor = std::exchange(self->start_executor, nullptr);
#if TIMER_PREALLOCATE_COMPLETION_HANDLER
			completion_handler = std::exchange(self->start_completion_handler, nullptr);
#else
			completion_fn = std::exchange(self->start_completion_fn, nullptr);
			data = std::exchange(self->start_data, nullptr);
#endif
		}
#if !TIMER_PREALLOCATE_COMPLETION_HANDLER
		completion_handler->completion_fn = completion_fn;
		completion_handler->data = data;
#endif
		completion_handler->status = CO_RESULT_0_CANCELLED;
#if !TIMER_PREALLOCATE_COMPLETION_HANDLER
		completion_handler->deallocate_fn = allocation_callbacks.deallocate_fn;
		completion_handler->deallocate_data = allocation_callbacks.data;
#endif
		// Delete the timer before posting the completion: once posted, the completion may run and destroy
		// it, so self must not be touched after a successful co_executor_0_post_0(). Deleting first also keeps the start
		// pending if the deletion fails.
		// The timer is already deleted (NULL) if a previous cancel_0() failed to post.
		// Wait for the elapsed callback (INVALID_HANDLE_VALUE): it may have fired before this call took the timer and
		// still be about to lock the mutex, which the posted completion may destroy. It finds no timer and returns.
		// Don't hold the lock while waiting: the callback waits for it.
		// NOTE: For a non-blocking alternative (DeleteTimerQueueTimer() without waiting), see the note in
		// platform_thread_pool_adapter_0_timer_0_impl_0::cancel_0(). Here it would only affect this fallback path.
		if (timer != NULL && self->delete_timer_queue_timer_fn(NULL, timer, INVALID_HANDLE_VALUE) == 0) {
			DWORD last_error = self->get_last_error_fn();
			(void)last_error;
			{
				// Hold the lock while accessing members of the timer: elapsed callback might be running right now.
				std::lock_guard lock(self->mutex);
				// Put everything back so user can call cancel_0() again. If the timer fired meanwhile, its callback
				// found no timer and returned, so the start stays pending until cancel_0() succeeds.
				self->timer = timer;
				self->start_executor = executor;
#if TIMER_PREALLOCATE_COMPLETION_HANDLER
				self->start_completion_handler = completion_handler;
#else
				self->start_completion_fn = completion_fn;
				self->start_data = data;
#endif
			}
#if !TIMER_PREALLOCATE_COMPLETION_HANDLER
			impl::delete_object_0(allocation_callbacks, completion_handler);
#endif
			return CO_RESULT_0_ERROR_SYSTEM;
		}
		if (co_result_0 result = co_executor_0_post_0(executor, {.invoke_fn = &platform_thread_pool_adapter_0_timer_0_impl_1::invoke, .data = completion_handler}); result != CO_RESULT_0_SUCCESS) {
#if !TIMER_PREALLOCATE_COMPLETION_HANDLER
			impl::delete_object_0(allocation_callbacks, completion_handler);
#endif
			{
				// Hold the lock while accessing members of the timer: elapsed callback might be running right now.
				std::lock_guard lock(self->mutex);
				// Put everything back, except the already deleted timer, so user can call cancel_0() again;
				// it will skip the deletion and only retry the post.
				self->start_executor = executor;
#if TIMER_PREALLOCATE_COMPLETION_HANDLER
				self->start_completion_handler = completion_handler;
#else
				self->start_completion_fn = completion_fn;
				self->start_data = data;
#endif
			}
			return result;
		}
		return CO_RESULT_0_SUCCESS;
	}

private:

	static VOID CALLBACK elapsed(_In_ PVOID parameter, _In_ BOOLEAN /* timer_or_wait_fired */) noexcept
	{
		platform_thread_pool_adapter_0_timer_0_impl_1* self = static_cast<platform_thread_pool_adapter_0_timer_0_impl_1*>(parameter);

		// We can access allocation_callbacks without lock because it is fixed once the timer is constructed.
		co_allocation_callbacks_0 allocation_callbacks = self->allocation_callbacks;
		HANDLE timer;
		bool cancelled;
		co_executor_0 executor;
#if TIMER_PREALLOCATE_COMPLETION_HANDLER
		platform_thread_pool_adapter_0_timer_0_completion_handler_0* completion_handler;
#else
		co_timer_0_wait_0_completion_fn completion_fn;
		void* data;
#endif
		{
			std::lock_guard lock(self->mutex);
			if (self->timer == NULL) {
				// cancel_0 fell back to deleting the timer itself - it is responsible for posting completion handler.
				// Leave the pending start alone: a cancel_0() that failed to post has put it back to retry the post.
				return;
			}
			timer = std::exchange(self->timer, HANDLE(NULL));
			cancelled = self->cancelled;
			executor = std::exchange(self->start_executor, nullptr);
#if TIMER_PREALLOCATE_COMPLETION_HANDLER
			completion_handler = std::exchange(self->start_completion_handler, nullptr);
#else
			completion_fn = std::exchange(self->start_completion_fn, nullptr);
			data = std::exchange(self->start_data, nullptr);
#endif
			// You may be tempted to call co_executor_0_post_0 while still holding a lock so we could post faster and release lock later,
			// but once posted, the completion may run and destroy the timer, including this mutex.
		}
		// TODO: In theory, if something below fails (deleting the timer, posting the completion), this callback could turn
		// into a long-running one and retry in a loop, with sleeps in between, instead of terminating.
		// Investigate if it's what we are actually want to do.
#if !TIMER_PREALLOCATE_COMPLETION_HANDLER
		platform_thread_pool_adapter_0_timer_0_completion_handler_0* completion_handler = impl::new_object_0<platform_thread_pool_adapter_0_timer_0_completion_handler_0>(allocation_callbacks);
		if (!completion_handler) {
			// TODO: Investigate if we want to terminate or lead to undefined behavior but win some performance.
			std::terminate();
		}
		completion_handler->completion_fn = completion_fn;
		completion_handler->data = data;
#endif
		if (cancelled) {
			completion_handler->status = CO_RESULT_0_CANCELLED;
		}
		else {
			completion_handler->status = CO_RESULT_0_SUCCESS;
		}
#if !TIMER_PREALLOCATE_COMPLETION_HANDLER
		completion_handler->deallocate_fn = allocation_callbacks.deallocate_fn;
		completion_handler->deallocate_data = allocation_callbacks.data;
#endif
		// Delete the timer before posting the completion: once posted, the completion may run and destroy
		// it, so self must not be touched after a successful co_executor_0_post_0().
		if (BOOL deleted = self->delete_timer_queue_timer_fn(NULL, timer, NULL); deleted == 0) {
			// According to documentation "ERROR_IO_PENDING" is ok. I don't know why but ok.
			if (DWORD last_error = self->get_last_error_fn(); last_error != ERROR_IO_PENDING) {
				// TODO: Investigate if we want to terminate, leak timers or post to executor a retry.
				std::terminate();
			}
		}
		if (co_result_0 result = co_executor_0_post_0(executor, {.invoke_fn = &platform_thread_pool_adapter_0_timer_0_impl_1::invoke, .data = completion_handler}); result != CO_RESULT_0_SUCCESS) {
			//impl::delete_object_0(allocation_callbacks, completion_handler);
			// TODO: Investigate if we can do here anything else than terminate.
			std::terminate();
		}
	}

	static void invoke(void* data) noexcept
	{
		platform_thread_pool_adapter_0_timer_0_completion_handler_0* completion_handler = static_cast<platform_thread_pool_adapter_0_timer_0_completion_handler_0*>(data);
		completion_handler->completion_fn(completion_handler->data, completion_handler->status);
		// TODO: refactor it into some helper function that doesn't take whole co_allocation_callbacks_0
		co_allocation_callbacks_0_deallocate_fn deallocate_fn = completion_handler->deallocate_fn;
		void* deallocate_data = completion_handler->deallocate_data;
		std::destroy_at(completion_handler);
		deallocate_fn(deallocate_data, completion_handler, sizeof(platform_thread_pool_adapter_0_timer_0_completion_handler_0), alignof(platform_thread_pool_adapter_0_timer_0_completion_handler_0));
	}

private:
	static const co_timer_0_vtable timer_vtable;
	const co_allocation_callbacks_0 allocation_callbacks;
	const co_clock_0 clock;
	const platform_thread_pool_adapter_0_functions_0_create_timer_queue_timer_fn create_timer_queue_timer_fn;
	const platform_thread_pool_adapter_0_functions_0_change_timer_queue_timer_fn change_timer_queue_timer_fn;
	const platform_thread_pool_adapter_0_functions_0_delete_timer_queue_timer_fn delete_timer_queue_timer_fn;
	const platform_thread_pool_adapter_0_functions_0_get_last_error_fn get_last_error_fn;
	std::mutex mutex;
	HANDLE timer;
	bool cancelled;
	co_executor_0 start_executor;
#if TIMER_PREALLOCATE_COMPLETION_HANDLER
	platform_thread_pool_adapter_0_timer_0_completion_handler_0* start_completion_handler;
#else
	co_timer_0_wait_0_completion_fn start_completion_fn;
	void* start_data;
#endif
};

const co_timer_0_vtable platform_thread_pool_adapter_0_timer_0_impl_1::timer_vtable = {
	.api_version = CO_API_VERSION_0,
	.destroy = reinterpret_cast<co_timer_0_destroy_fn>(&platform_thread_pool_adapter_0_timer_0_impl_1::destroy),
	.get_clock_0 = reinterpret_cast<co_timer_0_get_clock_0_fn>(&platform_thread_pool_adapter_0_timer_0_impl_1::get_clock_0),
	.expiry_at_0 = &platform_thread_pool_adapter_0_timer_0_expiry_at_0_not_supported,
	.expiry_after_0 = &platform_thread_pool_adapter_0_timer_0_expiry_after_0_not_supported,
	.wait_0 = &platform_thread_pool_adapter_0_timer_0_wait_0_not_supported,
	.cancel_0 = reinterpret_cast<co_timer_0_cancel_0_fn>(&platform_thread_pool_adapter_0_timer_0_impl_1::cancel_0),
};

#pragma endregion

#pragma region Timer service

class platform_thread_pool_adapter_0_timer_0_service_0_impl_0 : protected co_timer_0_service_0_t
{
public:

	platform_thread_pool_adapter_0_timer_0_service_0_impl_0(
		const co_allocation_callbacks_0& allocation_callbacks,
		platform_thread_pool_adapter_0_functions_0_create_timer_queue_timer_fn create_timer_queue_timer_fn,
		platform_thread_pool_adapter_0_functions_0_delete_timer_queue_timer_fn delete_timer_queue_timer_fn,
		platform_thread_pool_adapter_0_functions_0_get_last_error_fn get_last_error_fn
	) noexcept
		: allocation_callbacks({
			.structure_type = allocation_callbacks.structure_type,
			.next_structure = nullptr,
			.allocate_fn = allocation_callbacks.allocate_fn,
			.deallocate_fn = allocation_callbacks.deallocate_fn,
			.data = allocation_callbacks.data
		})
		, create_timer_queue_timer_fn(create_timer_queue_timer_fn)
		, delete_timer_queue_timer_fn(delete_timer_queue_timer_fn)
		, get_last_error_fn(get_last_error_fn)
	{
		co_timer_0_service_0_init(this, &service_vtable, &timer_service_vtable);
	}

private:

	static void destroy(platform_thread_pool_adapter_0_timer_0_service_0_impl_0* self) noexcept
	{
		impl::delete_object_0(self->allocation_callbacks, self);
	}

	static co_result_0 create_timer_0(platform_thread_pool_adapter_0_timer_0_service_0_impl_0* self, const co_timer_0_create_info_0* create_info, co_timer_0* timer) noexcept
	{
		platform_thread_pool_adapter_0_timer_0_impl_0* timer_impl = impl::new_object_0<platform_thread_pool_adapter_0_timer_0_impl_0>(
			self->allocation_callbacks,
			self->allocation_callbacks,
			create_info->clock,
			self->create_timer_queue_timer_fn,
			self->delete_timer_queue_timer_fn,
			self->get_last_error_fn
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
	const platform_thread_pool_adapter_0_functions_0_create_timer_queue_timer_fn create_timer_queue_timer_fn;
	const platform_thread_pool_adapter_0_functions_0_delete_timer_queue_timer_fn delete_timer_queue_timer_fn;
	const platform_thread_pool_adapter_0_functions_0_get_last_error_fn get_last_error_fn;
};

const co_service_0_vtable platform_thread_pool_adapter_0_timer_0_service_0_impl_0::service_vtable = {
	.api_version = CO_API_VERSION_0,
	.destroy = reinterpret_cast<co_service_0_destroy_fn>(&platform_thread_pool_adapter_0_timer_0_service_0_impl_0::destroy),
};

const co_timer_0_service_0_vtable platform_thread_pool_adapter_0_timer_0_service_0_impl_0::timer_service_vtable = {
	.create_timer_0 = reinterpret_cast<co_timer_0_service_0_create_timer_0_fn>(&platform_thread_pool_adapter_0_timer_0_service_0_impl_0::create_timer_0),
};

class platform_thread_pool_adapter_0_timer_0_service_0_impl_1 : protected co_timer_0_service_0_t
{
public:

	platform_thread_pool_adapter_0_timer_0_service_0_impl_1(
		const co_allocation_callbacks_0& allocation_callbacks,
		platform_thread_pool_adapter_0_functions_0_create_timer_queue_timer_fn create_timer_queue_timer_fn,
		platform_thread_pool_adapter_0_functions_0_change_timer_queue_timer_fn change_timer_queue_timer_fn,
		platform_thread_pool_adapter_0_functions_0_delete_timer_queue_timer_fn delete_timer_queue_timer_fn,
		platform_thread_pool_adapter_0_functions_0_get_last_error_fn get_last_error_fn
	) noexcept
		: allocation_callbacks({
			.structure_type = allocation_callbacks.structure_type,
			.next_structure = nullptr,
			.allocate_fn = allocation_callbacks.allocate_fn,
			.deallocate_fn = allocation_callbacks.deallocate_fn,
			.data = allocation_callbacks.data
		})
		, create_timer_queue_timer_fn(create_timer_queue_timer_fn)
		, change_timer_queue_timer_fn(change_timer_queue_timer_fn)
		, delete_timer_queue_timer_fn(delete_timer_queue_timer_fn)
		, get_last_error_fn(get_last_error_fn)
	{
		co_timer_0_service_0_init(this, &service_vtable, &timer_service_vtable);
	}

private:

	static void destroy(platform_thread_pool_adapter_0_timer_0_service_0_impl_1* self) noexcept
	{
		impl::delete_object_0(self->allocation_callbacks, self);
	}

	static co_result_0 create_timer_0(platform_thread_pool_adapter_0_timer_0_service_0_impl_1* self, const co_timer_0_create_info_0* create_info, co_timer_0* timer) noexcept
	{
		platform_thread_pool_adapter_0_timer_0_impl_1* timer_impl = impl::new_object_0<platform_thread_pool_adapter_0_timer_0_impl_1>(
			self->allocation_callbacks,
			self->allocation_callbacks,
			create_info->clock,
			self->create_timer_queue_timer_fn,
			self->change_timer_queue_timer_fn,
			self->delete_timer_queue_timer_fn,
			self->get_last_error_fn
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
	const platform_thread_pool_adapter_0_functions_0_create_timer_queue_timer_fn create_timer_queue_timer_fn;
	const platform_thread_pool_adapter_0_functions_0_change_timer_queue_timer_fn change_timer_queue_timer_fn;
	const platform_thread_pool_adapter_0_functions_0_delete_timer_queue_timer_fn delete_timer_queue_timer_fn;
	const platform_thread_pool_adapter_0_functions_0_get_last_error_fn get_last_error_fn;
};

const co_service_0_vtable platform_thread_pool_adapter_0_timer_0_service_0_impl_1::service_vtable = {
	.api_version = CO_API_VERSION_0,
	.destroy = reinterpret_cast<co_service_0_destroy_fn>(&platform_thread_pool_adapter_0_timer_0_service_0_impl_1::destroy),
};

const co_timer_0_service_0_vtable platform_thread_pool_adapter_0_timer_0_service_0_impl_1::timer_service_vtable = {
	.create_timer_0 = reinterpret_cast<co_timer_0_service_0_create_timer_0_fn>(&platform_thread_pool_adapter_0_timer_0_service_0_impl_1::create_timer_0),
};

#pragma endregion

#pragma region Event facility

} // namespace win32

const structure_type_0 structure_type_0_win32_platform_thread_pool_adapter_0_event_facility_0_create_info_0 = []() { return static_cast<structure_type_0>(reinterpret_cast<uintptr_t>(&structure_type_0_win32_platform_thread_pool_adapter_0_event_facility_0_create_info_0)); }();

} // namespace co

const co_structure_type_0 CO_STRUCTURE_TYPE_0_WIN32_PLATFORM_THREAD_POOL_ADAPTER_0_EVENT_FACILITY_0_CREATE_INFO_0 = []() { return static_cast<co_structure_type_0>(reinterpret_cast<uintptr_t>(&co::structure_type_0_win32_platform_thread_pool_adapter_0_event_facility_0_create_info_0)); }();

namespace co
{

namespace win32
{

class platform_thread_pool_adapter_0_event_facility_0_impl_0 : protected co_event_facility_0_t
{
public:

	platform_thread_pool_adapter_0_event_facility_0_impl_0(
		const co_allocation_callbacks_0& allocation_callbacks,
		co_suspend_watcher_0_service_0 suspend_watcher_service,
		co_timer_0_service_0 timer_service
	) noexcept
		: allocation_callbacks({
			.structure_type = allocation_callbacks.structure_type,
			.next_structure = nullptr,
			.allocate_fn = allocation_callbacks.allocate_fn,
			.deallocate_fn = allocation_callbacks.deallocate_fn,
			.data = allocation_callbacks.data
		})
		, suspend_watcher_service(suspend_watcher_service)
		, timer_service(timer_service)
	{
		co_event_facility_0_init(this, &event_domain_vtable, &event_facility_vtable);
	}

private:

	static void destroy(platform_thread_pool_adapter_0_event_facility_0_impl_0* self) noexcept
	{
		impl::delete_object_0(self->allocation_callbacks, self);
	}

	static co_result_0 get_service_0(platform_thread_pool_adapter_0_event_facility_0_impl_0* self, co_service_type_0 service_type, co_service_0* service) noexcept
	{
		if (service_type == CO_SERVICE_TYPE_0_SUSPEND_WATCHER_0_SERVICE_0 && self->suspend_watcher_service != nullptr) {
			*service = CO_SERVICE_0_CAST(self->suspend_watcher_service);
			return CO_RESULT_0_SUCCESS;
		}
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
	// NULL when the system has no suspend and resume notifications.
	co_suspend_watcher_0_service_0 suspend_watcher_service;
	co_timer_0_service_0 timer_service;
};

const co_event_domain_0_vtable platform_thread_pool_adapter_0_event_facility_0_impl_0::event_domain_vtable = {
	.api_version = CO_API_VERSION_0,
	.destroy = reinterpret_cast<co_event_domain_0_destroy_fn>(&platform_thread_pool_adapter_0_event_facility_0_impl_0::destroy),
	.get_service_0 = reinterpret_cast<co_event_domain_0_get_service_0_fn>(&platform_thread_pool_adapter_0_event_facility_0_impl_0::get_service_0),
};

const co_event_facility_0_vtable platform_thread_pool_adapter_0_event_facility_0_impl_0::event_facility_vtable = {
};

#pragma endregion

#pragma region Event domain group

class platform_thread_pool_adapter_0_event_domain_group_0_impl_0 : protected co_event_domain_group_0_t
{
public:

	platform_thread_pool_adapter_0_event_domain_group_0_impl_0(const co_allocation_callbacks_0& allocation_callbacks, co_suspend_watcher_0_service_0 suspend_watcher_service, co_timer_0_service_0 timer_service) noexcept
		: allocation_callbacks({
			.structure_type = allocation_callbacks.structure_type,
			.next_structure = nullptr,
			.allocate_fn = allocation_callbacks.allocate_fn,
			.deallocate_fn = allocation_callbacks.deallocate_fn,
			.data = allocation_callbacks.data
		})
		, event_facility(allocation_callbacks, suspend_watcher_service, timer_service)
	{
		co_event_domain_group_0_init(this, &event_domain_group_vtable);
	}

private:

	static void destroy(platform_thread_pool_adapter_0_event_domain_group_0_impl_0* self) noexcept
	{
		impl::delete_object_0(self->allocation_callbacks, self);
	}

	static void get_event_domain_0(platform_thread_pool_adapter_0_event_domain_group_0_impl_0* self, size_t event_domain_index, co_event_domain_0* event_domain) noexcept
	{
	}

private:
	static const co_event_domain_group_0_vtable event_domain_group_vtable;
	const co_allocation_callbacks_0 allocation_callbacks;
	platform_thread_pool_adapter_0_event_facility_0_impl_0 event_facility;
};

const co_event_domain_group_0_vtable platform_thread_pool_adapter_0_event_domain_group_0_impl_0::event_domain_group_vtable = {
	.api_version = CO_API_VERSION_0,
	.destroy = reinterpret_cast<co_event_domain_group_0_destroy_fn>(&platform_thread_pool_adapter_0_event_domain_group_0_impl_0::destroy),
	.get_event_domain_0 = reinterpret_cast<co_event_domain_group_0_get_event_domain_0_fn>(&platform_thread_pool_adapter_0_event_domain_group_0_impl_0::get_event_domain_0),
};

#pragma endregion

#pragma region Event domain group factory

class platform_thread_pool_adapter_0_event_domain_group_factory_0_impl_0 : protected co_event_domain_group_factory_0_t
{
public:

	// Takes ownership of suspend_watcher_service (which is NULL when the system has no suspend and resume notifications) and timer_service.
	platform_thread_pool_adapter_0_event_domain_group_factory_0_impl_0(const co_allocation_callbacks_0& allocation_callbacks, co_suspend_watcher_0_service_0 suspend_watcher_service, co_timer_0_service_0 timer_service) noexcept
		: allocation_callbacks({
			.structure_type = allocation_callbacks.structure_type,
			.next_structure = nullptr,
			.allocate_fn = allocation_callbacks.allocate_fn,
			.deallocate_fn = allocation_callbacks.deallocate_fn,
			.data = allocation_callbacks.data
		})
		, member_create_info_structure_types{
			CO_STRUCTURE_TYPE_0_EVENT_FACILITY_0_CREATE_INFO_0,
			CO_STRUCTURE_TYPE_0_WIN32_PLATFORM_THREAD_POOL_ADAPTER_0_EVENT_FACILITY_0_CREATE_INFO_0,
		}
		, handled_structure_types{
			CO_STRUCTURE_TYPE_0_DYNAMIC_THREAD_POOL_0_CREATE_INFO_0,
			CO_STRUCTURE_TYPE_0_PLATFORM_THREAD_POOL_0_CREATE_INFO_0,
		}
		, provided_service_types{
			CO_SERVICE_TYPE_0_SUSPEND_WATCHER_0_SERVICE_0,
			CO_SERVICE_TYPE_0_TIMER_0_SERVICE_0,
		}
		, provided_service_type_count(suspend_watcher_service != nullptr ? 2 : 1)
		, suspend_watcher_service(suspend_watcher_service)
		, timer_service(timer_service)
	{
		co_event_domain_group_factory_0_init(this, &event_domain_group_factory_vtable);
	}

private:

	static void destroy(platform_thread_pool_adapter_0_event_domain_group_factory_0_impl_0* self) noexcept
	{
		co_timer_0_service_0_destroy(self->timer_service);
		if (self->suspend_watcher_service != nullptr) {
			co_suspend_watcher_0_service_0_destroy(self->suspend_watcher_service);
		}
		impl::delete_object_0(self->allocation_callbacks, self);
	}

	static void get_member_create_info_structure_types_0(platform_thread_pool_adapter_0_event_domain_group_factory_0_impl_0* self, const co_structure_type_0** member_create_info_structure_types, size_t* member_create_info_structure_type_count) noexcept
	{
		*member_create_info_structure_types = self->member_create_info_structure_types;
		*member_create_info_structure_type_count = std::size(self->member_create_info_structure_types);
	}

	static void get_handled_structure_types_0(platform_thread_pool_adapter_0_event_domain_group_factory_0_impl_0* self, const co_structure_type_0** handled_structure_types, size_t* handled_structure_type_count) noexcept
	{
		*handled_structure_types = self->handled_structure_types;
		*handled_structure_type_count = std::size(self->handled_structure_types);
	}

	static void get_provided_service_types_0(platform_thread_pool_adapter_0_event_domain_group_factory_0_impl_0* self, const co_service_type_0** provided_service_types, size_t* provided_service_type_count) noexcept
	{
		*provided_service_types = self->provided_service_types;
		*provided_service_type_count = self->provided_service_type_count;
	}

	static void get_member_count_range_0(platform_thread_pool_adapter_0_event_domain_group_factory_0_impl_0* self, size_t* min_count, size_t* max_count) noexcept
	{
		*min_count = 1;
		*max_count = 1;
	}

	static void get_rank_0(platform_thread_pool_adapter_0_event_domain_group_factory_0_impl_0* self, size_t* rank) noexcept
	{
		*rank = 0;
	}

	static co_result_0 create_event_domain_group_0(platform_thread_pool_adapter_0_event_domain_group_factory_0_impl_0* self, const co_event_domain_group_0_create_info_0* create_info, co_event_domain_group_0* event_domain_group) noexcept
	{
		*event_domain_group = CO_EVENT_DOMAIN_GROUP_0_CAST(impl::new_object_0<platform_thread_pool_adapter_0_event_domain_group_0_impl_0>(
			self->allocation_callbacks,
			self->allocation_callbacks,
			self->suspend_watcher_service,
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
	const co_service_type_0 provided_service_types[2];
	const size_t provided_service_type_count;
	const co_suspend_watcher_0_service_0 suspend_watcher_service;
	const co_timer_0_service_0 timer_service;
};

const co_event_domain_group_factory_0_vtable platform_thread_pool_adapter_0_event_domain_group_factory_0_impl_0::event_domain_group_factory_vtable = {
	.api_version = CO_API_VERSION_0,
	.destroy = reinterpret_cast<co_event_domain_group_factory_0_destroy_fn>(&platform_thread_pool_adapter_0_event_domain_group_factory_0_impl_0::destroy),
	.get_member_create_info_structure_types_0 = reinterpret_cast<co_event_domain_group_factory_0_get_member_create_info_structure_types_0_fn>(&platform_thread_pool_adapter_0_event_domain_group_factory_0_impl_0::get_member_create_info_structure_types_0),
	.get_handled_structure_types_0 = reinterpret_cast<co_event_domain_group_factory_0_get_handled_structure_types_0_fn>(&platform_thread_pool_adapter_0_event_domain_group_factory_0_impl_0::get_handled_structure_types_0),
	.get_provided_service_types_0 = reinterpret_cast<co_event_domain_group_factory_0_get_provided_service_types_0_fn>(&platform_thread_pool_adapter_0_event_domain_group_factory_0_impl_0::get_provided_service_types_0),
	.get_member_count_range_0 = reinterpret_cast<co_event_domain_group_factory_0_get_member_count_range_0_fn>(&platform_thread_pool_adapter_0_event_domain_group_factory_0_impl_0::get_member_count_range_0),
	.get_rank_0 = reinterpret_cast<co_event_domain_group_factory_0_get_rank_0_fn>(&platform_thread_pool_adapter_0_event_domain_group_factory_0_impl_0::get_rank_0),
	.create_event_domain_group_0 = reinterpret_cast<co_event_domain_group_factory_0_create_event_domain_group_0_fn>(&platform_thread_pool_adapter_0_event_domain_group_factory_0_impl_0::create_event_domain_group_0),
};

#pragma endregion

class platform_thread_pool_adapter_0_executor_0_impl final : public co_executor_0_t
{
public:

	platform_thread_pool_adapter_0_executor_0_impl(const co_allocation_callbacks_0& allocation_callbacks, const platform_thread_pool_adapter_0_functions_0& functions) noexcept
		: m_allocation_callbacks(allocation_callbacks)
		, m_functions(functions)
	{
		m_allocation_callbacks.next_structure = nullptr;
		co_executor_0_init(this, &vtable);
	}

	static const co_executor_0_vtable vtable;

private:

	static void destroy(platform_thread_pool_adapter_0_executor_0_impl* self) noexcept
	{
		impl::delete_object_0(self->m_allocation_callbacks, self);
	}

	static co_result_0 post_0(platform_thread_pool_adapter_0_executor_0_impl* self, co_executor_handler_0 handler_) noexcept
	{
		co_executor_handler_0* handler = new (std::nothrow) co_executor_handler_0(handler_);
		if (handler == nullptr) {
			return CO_RESULT_0_ERROR_OUT_OF_MEMORY;
		}
		if (!self->m_functions.queue_user_work_item_fn(&platform_thread_pool_adapter_0_executor_0_impl::invoke, handler, WT_EXECUTEDEFAULT)) {
			// TODO: maybe some fallback mechanism?
			delete handler;
			return CO_RESULT_0_ERROR_SYSTEM;
		}
		return CO_RESULT_0_SUCCESS;
	}

	static DWORD WINAPI invoke(_In_ LPVOID parameter)
	{
		co_executor_handler_0 handler = *static_cast<co_executor_handler_0*>(parameter);
		delete static_cast<co_executor_handler_0*>(parameter);
		handler.invoke_fn(handler.data);
		return 0;
	}

private:
	co_allocation_callbacks_0 m_allocation_callbacks;
	platform_thread_pool_adapter_0_functions_0 m_functions;
};

const co_executor_0_vtable platform_thread_pool_adapter_0_executor_0_impl::vtable = {
	.api_version = CO_API_VERSION_0,
	.destroy = reinterpret_cast<co_executor_0_destroy_fn>(&platform_thread_pool_adapter_0_executor_0_impl::destroy),
	.post_0 = reinterpret_cast<co_executor_0_post_0_fn>(&platform_thread_pool_adapter_0_executor_0_impl::post_0),
};

} // namespace win32

//const structure_type_0 structure_type_0_win32_platform_thread_pool_adapter_0_executor_0_create_info_0 = []() { return static_cast<structure_type_0>(reinterpret_cast<uintptr_t>(&structure_type_0_win32_platform_thread_pool_adapter_0_executor_0_create_info_0)); }();

} // namespace co

//const co_structure_type_0 CO_STRUCTURE_TYPE_0_WIN32_PLATFORM_THREAD_POOL_ADAPTER_0_EXECUTOR_0_CREATE_INFO_0 = []() { return static_cast<co_structure_type_0>(reinterpret_cast<uintptr_t>(&co::structure_type_0_win32_platform_thread_pool_adapter_0_executor_0_create_info_0)); }();

/*
co_result_0 co_win32_platform_thread_pool_adapter_0_executor_0_create_0(co_instance_0 instance, const co_win32_platform_thread_pool_adapter_0_executor_0_create_info_0* create_info, co_executor_0* executor)
{
	using namespace co::detail;

	if (executor == nullptr || create_info->structure_type != CO_STRUCTURE_TYPE_0_WIN32_PLATFORM_THREAD_POOL_ADAPTER_0_EXECUTOR_0_CREATE_INFO_0 || create_info->win32_platform_thread_pool_adapter_0_event_facility_0_impl == nullptr) {
		report(instance, CO_DEBUG_SEVERITY_0_ERROR, CO_RESULT_0_ERROR_INVALID_ARGUMENT, "co_win32_platform_thread_pool_adapter_0_executor_0_create_0: invalid create info or null executor", nullptr);
		return CO_RESULT_0_ERROR_INVALID_ARGUMENT;
	}
	co_result_0 result = check_next_chain(instance, create_info->next_structure);
	if (result != CO_RESULT_0_SUCCESS) {
		return result;
	}
	const co_allocation_callbacks_0* allocation_callbacks = nullptr;
	result = find_allocation_callbacks(instance, create_info->next_structure, &allocation_callbacks);
	if (result != CO_RESULT_0_SUCCESS) {
		return result;
	}
	co::win32::platform_thread_pool_adapter_0_functions functions = {};
	result = static_cast<co_result_0>(co::win32::probe_functions(functions));
	if (result != CO_RESULT_0_SUCCESS) {
		return result;
	}
	co::win32::platform_thread_pool_adapter_0_executor_0_impl* object = new_object<co::win32::platform_thread_pool_adapter_0_executor_0_impl>(*allocation_callbacks, *allocation_callbacks, functions);
	if (object == nullptr) {
		report(instance, CO_DEBUG_SEVERITY_0_ERROR, CO_RESULT_0_ERROR_OUT_OF_MEMORY, "co_win32_platform_thread_pool_adapter_0_executor_0_create_0: cannot allocate the executor", nullptr);
		return CO_RESULT_0_ERROR_OUT_OF_MEMORY;
	}
	*executor = object;
	return CO_RESULT_0_SUCCESS;
}
*/

#pragma region Event domain group factory

co_result_0 co_win32_platform_thread_pool_adapter_0_event_domain_group_factory_0_create_0(const co_win32_platform_thread_pool_adapter_0_event_domain_group_factory_0_create_info_0* create_info, co_win32_platform_thread_pool_adapter_0_event_domain_group_factory_0* event_domain_group_factory) noexcept
{
	co_result_0 result;
	const co_allocation_callbacks_0* allocation_callbacks = nullptr;
	//result = co::detail::find_allocation_callbacks(instance, create_info->next_structure, &allocation_callbacks);
	if (result != CO_RESULT_0_SUCCESS) {
		return result;
	}
	co::win32::platform_thread_pool_adapter_0_functions_0_create_timer_queue_timer_fn create_timer_queue_timer_fn;
	co::win32::platform_thread_pool_adapter_0_functions_0_change_timer_queue_timer_fn change_timer_queue_timer_fn;
	co::win32::platform_thread_pool_adapter_0_functions_0_delete_timer_queue_timer_fn delete_timer_queue_timer_fn;
	co::win32::platform_thread_pool_adapter_0_functions_0_get_last_error_fn get_last_error_fn;
	co_timer_0_service_0 timer_service;
	if (change_timer_queue_timer_fn) {
		timer_service = CO_TIMER_0_SERVICE_0_CAST(co::impl::new_object_0<co::win32::platform_thread_pool_adapter_0_timer_0_service_0_impl_1>(
			*allocation_callbacks,
			*allocation_callbacks,
			create_timer_queue_timer_fn,
			change_timer_queue_timer_fn,
			delete_timer_queue_timer_fn,
			get_last_error_fn
		));
	}
	else {
		timer_service = CO_TIMER_0_SERVICE_0_CAST(co::impl::new_object_0<co::win32::platform_thread_pool_adapter_0_timer_0_service_0_impl_0>(
			*allocation_callbacks,
			*allocation_callbacks,
			create_timer_queue_timer_fn,
			delete_timer_queue_timer_fn,
			get_last_error_fn
		));
	}
	// Optional: the factory provides no suspend watcher service when the system has no suspend and resume notifications.
	co_suspend_watcher_0_service_0 suspend_watcher_service = nullptr;
	result = co::win32::platform_thread_pool_adapter_0_suspend_watcher_0_service_0_impl_0::create_0(*allocation_callbacks, &suspend_watcher_service);
	if (result != CO_RESULT_0_SUCCESS && result != CO_RESULT_0_ERROR_NOT_SUPPORTED) {
		co_timer_0_service_0_destroy(timer_service);
		return result;
	}
	return CO_RESULT_0_ERROR_UNKNOWN;
}

#pragma endregion

// Test

static_assert(co::detail::event_domain_create_info_of<co_win32_platform_thread_pool_adapter_0_event_facility_0_create_info_0, co_event_domain_0_create_info_0>);
static_assert(co::detail::event_domain_create_info_of<co::win32::platform_thread_pool_adapter_0_event_facility_0_create_info_0, co::event_domain_0_create_info_0>);
static_assert(sizeof(co_win32_platform_thread_pool_adapter_0_event_facility_0_create_info_0) == sizeof(co::win32::platform_thread_pool_adapter_0_event_facility_0_create_info_0));

/*
static_assert(std::is_standard_layout_v<co_win32_platform_thread_pool_adapter_0_executor_0_create_info_0>);
static_assert(offsetof(co_win32_platform_thread_pool_adapter_0_executor_0_create_info_0, structure_type) == offsetof(co_in_structure_0, structure_type));
static_assert(offsetof(co_win32_platform_thread_pool_adapter_0_executor_0_create_info_0, next_structure) == offsetof(co_in_structure_0, next_structure));
static_assert(std::is_standard_layout_v<co::win32::platform_thread_pool_adapter_0_executor_0_create_info_0>);
static_assert(offsetof(co::win32::platform_thread_pool_adapter_0_executor_0_create_info_0, structure_type) == offsetof(co::in_structure_0, structure_type));
static_assert(offsetof(co::win32::platform_thread_pool_adapter_0_executor_0_create_info_0, next_structure) == offsetof(co::in_structure_0, next_structure));
static_assert(sizeof(co_win32_platform_thread_pool_adapter_0_executor_0_create_info_0) == sizeof(co::win32::platform_thread_pool_adapter_0_executor_0_create_info_0));
static_assert(offsetof(co_win32_platform_thread_pool_adapter_0_executor_0_create_info_0, structure_type) == offsetof(co::win32::platform_thread_pool_adapter_0_executor_0_create_info_0, structure_type));
static_assert(offsetof(co_win32_platform_thread_pool_adapter_0_executor_0_create_info_0, next_structure) == offsetof(co::win32::platform_thread_pool_adapter_0_executor_0_create_info_0, next_structure));
static_assert(offsetof(co_win32_platform_thread_pool_adapter_0_executor_0_create_info_0, win32_platform_thread_pool_adapter_0_event_facility_0_impl) == offsetof(co::win32::platform_thread_pool_adapter_0_executor_0_create_info_0, win32_platform_thread_pool_adapter_0_event_facility_0_impl));
*/
