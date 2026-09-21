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

#include "win32/platform_thread_pool_adapter_0.hpp"

#include <co/impl.hpp>
#include "dynamic_thread_pool_0.hpp"
#include "platform_thread_pool_0.hpp"
#include "timer_0_service_0.hpp"

#include <atomic>
#include <cassert>
#include <cstddef>
#include <mutex>
#include <new>
#include <optional>
#include <type_traits>

// cancel_0() does not post the completion itself; it makes the timer fire immediately,
// so elapsed() is the only place that posts the completion and handles post failures.
#define TIMER_POST_ONLY_IN_ELAPSED_CALLBACK 1

// Allocate the completion handler in start_0(), where running out of memory can still be reported,
// so that posting it later (from elapsed() or cancel_0()) never allocates.
// Forced by TIMER_POST_ONLY_IN_ELAPSED_CALLBACK (elapsed() has no caller to report an allocation failure to);
// otherwise chosen by the trailing value.
#define TIMER_PREALLOCATE_COMPLETION_HANDLER (TIMER_POST_ONLY_IN_ELAPSED_CALLBACK || 1)

namespace co
{

namespace win32
{

using platform_thread_pool_adapter_0_functions_0_queue_user_work_item_fn = _Must_inspect_result_ BOOL (WINAPI *)(_In_ LPTHREAD_START_ROUTINE function, _In_opt_ PVOID context, _In_ ULONG flags) noexcept;
using platform_thread_pool_adapter_0_functions_0_create_timer_queue_fn = _Ret_maybenull_ HANDLE (WINAPI *)(VOID) noexcept;
using platform_thread_pool_adapter_0_functions_0_delete_timer_queue_ex_fn = _Must_inspect_result_ BOOL (WINAPI *)(_In_ HANDLE timer_queue, _In_opt_ HANDLE completion_event) noexcept;
using platform_thread_pool_adapter_0_functions_0_create_timer_queue_timer_fn = _Must_inspect_result_ BOOL (WINAPI *)(_Outptr_ PHANDLE new_timer, _In_opt_ HANDLE timer_queue, _In_ WAITORTIMERCALLBACK callback, _In_opt_ PVOID parameter, _In_ DWORD due_time, _In_ DWORD period, _In_ ULONG flags) noexcept;
#if TIMER_POST_ONLY_IN_ELAPSED_CALLBACK
using platform_thread_pool_adapter_0_functions_0_change_timer_queue_timer_fn = _Must_inspect_result_ BOOL (WINAPI *)(_In_opt_ HANDLE timer_queue, _Inout_ HANDLE timer, _In_ ULONG due_time, _In_ ULONG period) noexcept;
#endif
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
#if TIMER_POST_ONLY_IN_ELAPSED_CALLBACK
	platform_thread_pool_adapter_0_functions_0_change_timer_queue_timer_fn change_timer_queue_timer_fn;
#endif
	platform_thread_pool_adapter_0_functions_0_delete_timer_queue_timer_fn delete_timer_queue_timer_fn;
	platform_thread_pool_adapter_0_functions_0_get_last_error_fn get_last_error_fn;
};

template <class Function>
bool probe(HMODULE module, const char* name, Function*& function) noexcept
{
	function = reinterpret_cast<Function*>(GetProcAddress(module, name));
	return function != nullptr;
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
#if TIMER_POST_ONLY_IN_ELAPSED_CALLBACK
	ok &= probe(kernel32, "ChangeTimerQueueTimer", functions.change_timer_queue_timer_fn);
#endif
	ok &= probe(kernel32, "DeleteTimerQueueTimer", functions.delete_timer_queue_timer_fn);
	ok &= probe(kernel32, "GetLastError", functions.get_last_error_fn);
	return ok ? result_0::success : result_0::error_not_supported;
}

#pragma region Timer

struct platform_thread_pool_adapter_0_timer_0_completion_handler_0 final
{
	co_timer_0_start_0_completion_fn completion_fn;
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
		platform_thread_pool_adapter_0_functions_0_create_timer_queue_timer_fn create_timer_queue_timer_fn,
#if TIMER_POST_ONLY_IN_ELAPSED_CALLBACK
		platform_thread_pool_adapter_0_functions_0_change_timer_queue_timer_fn change_timer_queue_timer_fn,
#endif
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
#if TIMER_POST_ONLY_IN_ELAPSED_CALLBACK
		, change_timer_queue_timer_fn(change_timer_queue_timer_fn)
#endif
		, delete_timer_queue_timer_fn(delete_timer_queue_timer_fn)
		, get_last_error_fn(get_last_error_fn)
		, mutex()
		, timer(NULL)
#if TIMER_POST_ONLY_IN_ELAPSED_CALLBACK
		, cancelled(false)
#endif
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

	static void destroy(platform_thread_pool_adapter_0_timer_0_impl_0* self) noexcept
	{
		impl::delete_object_0(self->allocation_callbacks, self);
	}

	static co_result_0 start_0(platform_thread_pool_adapter_0_timer_0_impl_0* self, uint64_t due_ns, co_executor_0 executor, co_timer_0_start_0_completion_fn completion_fn, void* data) noexcept
	{
		// Round due_ns up so that 1ns becomes 1ms.
		uint64_t due_ms = due_ns / 1000000 + (due_ns % 1000000 != 0 ? 1 : 0);
		if (due_ms >= INFINITE) {
			return CO_RESULT_0_ERROR_SYSTEM;
		}
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
			// Restore sane state.
			self->timer = NULL;
			DWORD last_error = self->get_last_error_fn();
			(void)last_error;
			return CO_RESULT_0_ERROR_SYSTEM;
		}
#if TIMER_PREALLOCATE_COMPLETION_HANDLER
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
#if TIMER_POST_ONLY_IN_ELAPSED_CALLBACK
		self->cancelled = false;
#endif
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
#if TIMER_POST_ONLY_IN_ELAPSED_CALLBACK
		HANDLE timer;
		{
			// Hold the lock while accessing members of the timer: elapsed callback might be running right now.
			std::lock_guard lock(self->mutex);
			timer = self->timer;
			self->cancelled = true;
			// TODO: Investigate if we can call ChangeTimerQueueTimer() while still holding a lock.
		}
		if (timer == NULL) {
			// elapsed callback won (or cancel_0 was called incorrectly).
			// TODO: Decide if that would require some special result code.
			return CO_RESULT_0_SUCCESS;
		}
		// Force elapsed callback to fire immediately (in timer thread) - it will check cancelled flag.
		if (BOOL changed = self->change_timer_queue_timer_fn(NULL, timer, 0, 0); changed == 0) {
			DWORD last_error = self->get_last_error_fn();
			(void)last_error;
			return CO_RESULT_0_ERROR_SYSTEM;
		}
		return CO_RESULT_0_SUCCESS;
#else
#if !TIMER_PREALLOCATE_COMPLETION_HANDLER
		// We can access allocation_callbacks without lock because it is fixed once the timer is constructed.
		co_allocation_callbacks_0 allocation_callbacks = self->allocation_callbacks;
#endif
		HANDLE timer;
		co_executor_0 executor;
#if TIMER_PREALLOCATE_COMPLETION_HANDLER
		platform_thread_pool_adapter_0_timer_0_completion_handler_0* completion_handler;
#else
		co_timer_0_start_0_completion_fn completion_fn;
		void* data;
#endif
		{
			// Hold the lock while accessing members of the timer: elapsed callback might be running right now.
			std::lock_guard lock(self->mutex);
			// Decide by the pending start, not by the timer: a previous cancel_0() may have deleted the timer
			// but failed to post, and put the start back without a timer so that this call retries the post.
			if (!self->start_executor) {
				// elapsed callback won (or cancel_0 was called incorrectly) - he will be responsible for posting completion handler and deleting timer.
				// TODO: Decide if that would require some special result code.
				return CO_RESULT_0_SUCCESS;
			}
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
		platform_thread_pool_adapter_0_timer_0_completion_handler_0* completion_handler = impl::new_object_0<platform_thread_pool_adapter_0_timer_0_completion_handler_0>(allocation_callbacks);
		if (!completion_handler) {
			// TODO: Investigate if we want to terminate or lead to undefined behavior but win some performance.
			std::terminate();
		}
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
		if (timer != NULL && self->delete_timer_queue_timer_fn(NULL, timer, NULL) == 0) {
			// According to documentation "ERROR_IO_PENDING" is ok. I don't know why but ok.
			if (DWORD last_error = self->get_last_error_fn(); last_error != ERROR_IO_PENDING) {
				{
					// Hold the lock while accessing members of the timer: elapsed callback might be running right now.
					std::lock_guard lock(self->mutex);
					// Put everything back so user can call cancel_0() again.
					self->timer = timer;
					self->start_executor = executor;
#if TIMER_PREALLOCATE_COMPLETION_HANDLER
					self->start_completion_handler = completion_handler;
#else
					self->start_completion_fn = completion_fn;
					self->start_data = data;
#endif
				}
				return CO_RESULT_0_ERROR_SYSTEM;
			}
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
#endif
	}

private:

	static VOID CALLBACK elapsed(_In_ PVOID parameter, _In_ BOOLEAN /* timer_or_wait_fired */) noexcept
	{
		platform_thread_pool_adapter_0_timer_0_impl_0* self = static_cast<platform_thread_pool_adapter_0_timer_0_impl_0*>(parameter);

		// We can access allocation_callbacks without lock because it is fixed once the timer is constructed.
		co_allocation_callbacks_0 allocation_callbacks = self->allocation_callbacks;
		HANDLE timer;
#if TIMER_POST_ONLY_IN_ELAPSED_CALLBACK
		bool cancelled;
#endif
		co_executor_0 executor;
#if TIMER_PREALLOCATE_COMPLETION_HANDLER
		platform_thread_pool_adapter_0_timer_0_completion_handler_0* completion_handler;
#else
		co_timer_0_start_0_completion_fn completion_fn;
		void* data;
#endif
		{
			std::lock_guard lock(self->mutex);
#if !TIMER_POST_ONLY_IN_ELAPSED_CALLBACK
			if (self->timer == NULL) {
				// cancel_0 won - it is responsible for posting completion handler and deleting timer.
				// Leave the pending start alone: a cancel_0() that failed to post has put it back to retry the post.
				return;
			}
#endif
			timer = std::exchange(self->timer, HANDLE(NULL));
#if TIMER_POST_ONLY_IN_ELAPSED_CALLBACK
			cancelled = self->cancelled;
#endif
			executor = std::exchange(self->start_executor, nullptr);
#if TIMER_PREALLOCATE_COMPLETION_HANDLER
			completion_handler = std::exchange(self->start_completion_handler, nullptr);
#else
			completion_fn = std::exchange(self->start_completion_fn, nullptr);
			data = std::exchange(self->start_data, nullptr);
#endif
			// TODO: Explain why we call co_executor_0_post_0 outside of lock since in theory we could post faster and release lock later.
		}
		// TODO: In theory
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
#if TIMER_POST_ONLY_IN_ELAPSED_CALLBACK
		if (cancelled) {
			completion_handler->status = CO_RESULT_0_CANCELLED;
		}
		else {
			completion_handler->status = CO_RESULT_0_SUCCESS;
		}
#else
		completion_handler->status = CO_RESULT_0_SUCCESS;
#endif
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
			// TODO: Right now we are going to terminate anyway - do we need to cleanup anything?
			impl::delete_object_0(allocation_callbacks, completion_handler);
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
	const platform_thread_pool_adapter_0_functions_0_create_timer_queue_timer_fn create_timer_queue_timer_fn;
#if TIMER_POST_ONLY_IN_ELAPSED_CALLBACK
	const platform_thread_pool_adapter_0_functions_0_change_timer_queue_timer_fn change_timer_queue_timer_fn;
#endif
	const platform_thread_pool_adapter_0_functions_0_delete_timer_queue_timer_fn delete_timer_queue_timer_fn;
	const platform_thread_pool_adapter_0_functions_0_get_last_error_fn get_last_error_fn;
	std::mutex mutex;
	HANDLE timer;
#if TIMER_POST_ONLY_IN_ELAPSED_CALLBACK
	bool cancelled;
#endif
	co_executor_0 start_executor;
#if TIMER_PREALLOCATE_COMPLETION_HANDLER
	platform_thread_pool_adapter_0_timer_0_completion_handler_0* start_completion_handler;
#else
	co_timer_0_start_0_completion_fn start_completion_fn;
	void* start_data;
#endif
};

const co_timer_0_vtable platform_thread_pool_adapter_0_timer_0_impl_0::timer_vtable = {
	.api_version = CO_API_VERSION_0,
	.destroy = reinterpret_cast<co_timer_0_destroy_fn>(&platform_thread_pool_adapter_0_timer_0_impl_0::destroy),
	.start_0 = reinterpret_cast<co_timer_0_start_0_fn>(&platform_thread_pool_adapter_0_timer_0_impl_0::start_0),
	.cancel_0 = reinterpret_cast<co_timer_0_cancel_0_fn>(&platform_thread_pool_adapter_0_timer_0_impl_0::cancel_0),
};

#pragma endregion

#pragma region Timer service

class platform_thread_pool_adapter_0_timer_0_service_0_impl_0 : protected co_timer_0_service_0_t
{
public:

	platform_thread_pool_adapter_0_timer_0_service_0_impl_0(
		const co_allocation_callbacks_0& allocation_callbacks,
		platform_thread_pool_adapter_0_functions_0_create_timer_queue_timer_fn create_timer_queue_timer_fn,
#if TIMER_POST_ONLY_IN_ELAPSED_CALLBACK
		const platform_thread_pool_adapter_0_functions_0_change_timer_queue_timer_fn change_timer_queue_timer_fn,
#endif
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
#if TIMER_POST_ONLY_IN_ELAPSED_CALLBACK
		, change_timer_queue_timer_fn(change_timer_queue_timer_fn)
#endif
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

	static co_result_0 create_timer_0(platform_thread_pool_adapter_0_timer_0_service_0_impl_0* self, co_timer_0* timer) noexcept
	{
		platform_thread_pool_adapter_0_timer_0_impl_0* timer_impl = impl::new_object_0<platform_thread_pool_adapter_0_timer_0_impl_0>(
			self->allocation_callbacks,
			self->allocation_callbacks,
			self->create_timer_queue_timer_fn,
#if TIMER_POST_ONLY_IN_ELAPSED_CALLBACK
			self->change_timer_queue_timer_fn,
#endif
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
#if TIMER_POST_ONLY_IN_ELAPSED_CALLBACK
	const platform_thread_pool_adapter_0_functions_0_change_timer_queue_timer_fn change_timer_queue_timer_fn;
#endif
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
		platform_thread_pool_adapter_0_functions_0_create_timer_queue_timer_fn create_timer_queue_timer_fn,
#if TIMER_POST_ONLY_IN_ELAPSED_CALLBACK
		const platform_thread_pool_adapter_0_functions_0_change_timer_queue_timer_fn change_timer_queue_timer_fn,
#endif
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
		, timer_service(
			allocation_callbacks,
			create_timer_queue_timer_fn,
#if TIMER_POST_ONLY_IN_ELAPSED_CALLBACK
			change_timer_queue_timer_fn,
#endif
			delete_timer_queue_timer_fn,
			get_last_error_fn
		)
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
		*service = CO_SERVICE_0_CAST(&self->timer_service);
		return CO_RESULT_0_SUCCESS;
	}

private:
	static const co_event_domain_0_vtable event_domain_vtable;
	static const co_event_facility_0_vtable event_facility_vtable;
	platform_thread_pool_adapter_0_timer_0_service_0_impl_0 timer_service;
	const co_allocation_callbacks_0 allocation_callbacks;
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

	platform_thread_pool_adapter_0_event_domain_group_0_impl_0(const co_allocation_callbacks_0& allocation_callbacks) noexcept
		: allocation_callbacks({
			.structure_type = allocation_callbacks.structure_type,
			.next_structure = nullptr,
			.allocate_fn = allocation_callbacks.allocate_fn,
			.deallocate_fn = allocation_callbacks.deallocate_fn,
			.data = allocation_callbacks.data
		})
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

	platform_thread_pool_adapter_0_event_domain_group_factory_0_impl_0(const co_allocation_callbacks_0& allocation_callbacks) noexcept
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
			CO_SERVICE_TYPE_0_TIMER_0_SERVICE_0,
		}
	{
		co_event_domain_group_factory_0_init(this, &event_domain_group_factory_vtable);
	}

private:

	static void destroy(platform_thread_pool_adapter_0_event_domain_group_factory_0_impl_0* self) noexcept
	{
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
		*provided_service_type_count = std::size(self->provided_service_types);
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

	static co_result_0 create_event_domain_group_0(platform_thread_pool_adapter_0_event_domain_group_factory_0_impl_0* self, const co_event_domain_group_0_create_info_0* create_info, co_event_domain_group_0* group) noexcept
	{
		return CO_RESULT_0_SUCCESS;
	}

private:
	static const co_event_domain_group_factory_0_vtable event_domain_group_factory_vtable;
	const co_allocation_callbacks_0 allocation_callbacks;
	const co_structure_type_0 member_create_info_structure_types[2];
	const co_structure_type_0 handled_structure_types[2];
	const co_service_type_0 provided_service_types[1];
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
