#define WIN32_LEAN_AND_MEAN
#define NOGDICAPMASKS
#define NOVIRTUALKEYCODES
//#define NOWINMESSAGES
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
//#define NOUSER
#define NONLS
#define NOMB
#define NOMEMMGR
#define NOMETAFILE
#define NOMINMAX
//#define NOMSG
#define NOOPENFILE
#define NOSCROLL
#define NOSERVICE
#define NOSOUND
#define NOTEXTMETRIC
#define NOWH
//#define NOWINOFFSETS
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

#include "message_pump_0.hpp"

#include <co/timer_0_service_0.hpp>
#include <co/time_watcher_0_service_0.hpp>

#include <co/impl.hpp>

#include <array> // std::size()
#include <cassert>
#include <cstdint>
#include <iomanip>
#include <memory>
#include <mutex>
#include <sstream>
#include <type_traits>

namespace co::win32
{

namespace
{



} // namespace

} // namespace co::win32

namespace co::win32
{

namespace
{



} // namespace

} // namespace co::win32

#pragma region Time watcher

namespace co::win32
{

namespace
{

class message_pump_0_time_watcher_0_impl_0;

// A pending wait. Allocated by the wait call, so that completing it never allocates, and deallocated by its completion.
struct message_pump_0_time_watcher_0_wait_0 final
{
	message_pump_0_time_watcher_0_impl_0* time_watcher;
	// Registered while the wait is in the list of its time watcher, unless NULL.
	co_cancellation_0 cancellation;
	co_cancellation_0_registration_0 registration;
	co_executor_0 executor;
	co_time_watcher_0_wait_change_0_completion_fn completion_fn;
	void* data;
	co_result_0 status;
	// The generation the wait was given; once completed, the generation passed to the completion callback.
	size_t generation;
	co_allocation_callbacks_0_deallocate_fn deallocate_fn;
	void* deallocate_data;
	// Links of the list of pending waits of the time watcher; once taken from it, next links the waits to complete.
	message_pump_0_time_watcher_0_wait_0* prev;
	message_pump_0_time_watcher_0_wait_0* next;
	bool linked;
};

// Intrusive list of the pending waits of a time watcher.
class message_pump_0_time_watcher_0_wait_list_0 final
{
public:

	bool empty() const noexcept
	{
		return head == nullptr;
	}

	void push(message_pump_0_time_watcher_0_wait_0* wait) noexcept
	{
		wait->prev = nullptr;
		wait->next = head;
		if (head != nullptr) {
			head->prev = wait;
		}
		head = wait;
		wait->linked = true;
	}

	void remove(message_pump_0_time_watcher_0_wait_0* wait) noexcept
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

	// Empties the list, gives every wait the status and generation and prepends them to the waits to complete;
	// returns the new head.
	message_pump_0_time_watcher_0_wait_0* take_all(co_result_0 status, size_t generation, message_pump_0_time_watcher_0_wait_0* completed) noexcept
	{
		while (head != nullptr) {
			message_pump_0_time_watcher_0_wait_0* wait = head;
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
	message_pump_0_time_watcher_0_wait_0* head = nullptr;
};

class message_pump_0_time_watcher_0_service_0_impl_0;

class message_pump_0_time_watcher_0_impl_0 final : protected co_time_watcher_0_t
{
public:

	message_pump_0_time_watcher_0_impl_0(
		const co_allocation_callbacks_0& allocation_callbacks,
		message_pump_0_time_watcher_0_service_0_impl_0* service
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
		co_time_watcher_0_init(this, &time_watcher_vtable);
	}

	~message_pump_0_time_watcher_0_impl_0()
	{
		assert(waits.empty() && "a time watcher may be destroyed only when no wait is pending");
	}

	// Called by the service, with its mutex held, when the system time changed. Advances the generation, prepends the
	// waits it completes (all pending waits, which wait after the previous generation) to the waits to complete and
	// returns the new head.
	message_pump_0_time_watcher_0_wait_0* notify(message_pump_0_time_watcher_0_wait_0* completed) noexcept
	{
		std::lock_guard lock(mutex);
		const size_t new_generation = ++generation;
		return waits.take_all(CO_RESULT_0_SUCCESS, new_generation, completed);
	}

	// Completes the waits taken from their lists, linked through next: unregisters their cancellations and posts them.
	// Must be called without any lock held, since the executor may invoke the completions right away.
	static void complete(message_pump_0_time_watcher_0_wait_0* completed) noexcept
	{
		while (completed != nullptr) {
			message_pump_0_time_watcher_0_wait_0* wait = completed;
			// Once posted, the wait may be deallocated.
			completed = wait->next;
			if (wait->cancellation != nullptr) {
				// Waits for the cancellation callback of the wait if it is running; it finds the wait unlinked and returns.
				co_cancellation_0_unregister_0(wait->cancellation, &wait->registration);
			}
			post_or_terminate(wait);
		}
	}

	// Links of the list of time watchers of the service, guarded by the mutex of the service.
	message_pump_0_time_watcher_0_impl_0* service_prev = nullptr;
	message_pump_0_time_watcher_0_impl_0* service_next = nullptr;

private:

	static void destroy(message_pump_0_time_watcher_0_impl_0* self) noexcept;

	static void get_generation_0(message_pump_0_time_watcher_0_impl_0* self, size_t* generation) noexcept
	{
		std::lock_guard lock(self->mutex);
		*generation = self->generation;
	}

	static co_result_0 wait_change_0(message_pump_0_time_watcher_0_impl_0* self, size_t after_generation, co_cancellation_0 cancellation, co_executor_0 executor, co_time_watcher_0_wait_change_0_completion_fn completion_fn, void* data) noexcept
	{
		message_pump_0_time_watcher_0_wait_0* wait = impl::new_object_0<message_pump_0_time_watcher_0_wait_0>(self->allocation_callbacks);
		if (!wait) {
			return CO_RESULT_0_ERROR_OUT_OF_MEMORY;
		}
		wait->time_watcher = self;
		wait->cancellation = nullptr;
		wait->registration = {
			.callback_fn = &message_pump_0_time_watcher_0_impl_0::cancelled,
			.data = wait,
		};
		wait->executor = executor;
		wait->completion_fn = completion_fn;
		wait->data = data;
		wait->status = CO_RESULT_0_SUCCESS;
		wait->generation = after_generation;
		wait->deallocate_fn = self->allocation_callbacks.deallocate_fn;
		wait->deallocate_data = self->allocation_callbacks.data;
		wait->prev = nullptr;
		wait->next = nullptr;
		wait->linked = false;
		std::unique_lock lock(self->mutex);
		const size_t generation = self->generation;
		// Generations wrap around: a wait is behind when its generation differs. Coarse check that it is not ahead.
		assert(static_cast<std::make_signed_t<size_t>>(generation - after_generation) >= 0 && "a wait may not be given a generation ahead of the current one");
		if (after_generation != generation) {
			lock.unlock();
			wait->generation = generation;
			if (co_result_0 result = post(wait); result != CO_RESULT_0_SUCCESS) {
				impl::delete_object_0(self->allocation_callbacks, wait);
				return result;
			}
			return CO_RESULT_0_SUCCESS;
		}
		// Register with the lock held, so that the cancellation callback, which takes the lock, finds the wait linked.
		if (cancellation != nullptr) {
			if (!co_cancellation_0_register_0(cancellation, &wait->registration)) {
				lock.unlock();
				wait->status = CO_RESULT_0_CANCELLED;
				if (co_result_0 result = post(wait); result != CO_RESULT_0_SUCCESS) {
					impl::delete_object_0(self->allocation_callbacks, wait);
					return result;
				}
				return CO_RESULT_0_SUCCESS;
			}
			wait->cancellation = cancellation;
		}
		self->waits.push(wait);
		return CO_RESULT_0_SUCCESS;
	}

	static co_result_0 cancel_0(message_pump_0_time_watcher_0_impl_0* self) noexcept
	{
		message_pump_0_time_watcher_0_wait_0* completed = nullptr;
		{
			std::lock_guard lock(self->mutex);
			// Pending waits wait after the current generation, which is unchanged.
			completed = self->waits.take_all(CO_RESULT_0_CANCELLED, self->generation, completed);
		}
		complete(completed);
		return CO_RESULT_0_SUCCESS;
	}

private:

	static void cancelled(void* data) noexcept
	{
		message_pump_0_time_watcher_0_wait_0* wait = static_cast<message_pump_0_time_watcher_0_wait_0*>(data);
		message_pump_0_time_watcher_0_impl_0* self = wait->time_watcher;
		{
			std::lock_guard lock(self->mutex);
			if (!wait->linked) {
				// Already taken by a change or cancel_0(), which completes it.
				return;
			}
			self->waits.remove(wait);
		}
		// This callback is the registration; it is not unregistered.
		wait->cancellation = nullptr;
		wait->status = CO_RESULT_0_CANCELLED;
		post_or_terminate(wait);
	}

	static co_result_0 post(message_pump_0_time_watcher_0_wait_0* wait) noexcept
	{
		return co_executor_0_post_0(wait->executor, {.invoke_fn = &message_pump_0_time_watcher_0_impl_0::invoke, .data = wait});
	}

	static void post_or_terminate(message_pump_0_time_watcher_0_wait_0* wait) noexcept
	{
		if (post(wait) != CO_RESULT_0_SUCCESS) {
			// TODO: Investigate if we can do here anything else than terminate.
			std::terminate();
		}
	}

	static void invoke(void* data) noexcept
	{
		message_pump_0_time_watcher_0_wait_0* wait = static_cast<message_pump_0_time_watcher_0_wait_0*>(data);
		co_time_watcher_0_wait_change_0_completion_fn completion_fn = wait->completion_fn;
		void* completion_data = wait->data;
		co_result_0 status = wait->status;
		size_t generation = wait->generation;
		co_allocation_callbacks_0_deallocate_fn deallocate_fn = wait->deallocate_fn;
		void* deallocate_data = wait->deallocate_data;
		// Deallocate first: the completion may destroy the time watcher and the allocation callbacks with it.
		std::destroy_at(wait);
		deallocate_fn(deallocate_data, wait, sizeof(message_pump_0_time_watcher_0_wait_0), alignof(message_pump_0_time_watcher_0_wait_0));
		completion_fn(completion_data, status, generation);
	}

private:
	static const co_time_watcher_0_vtable time_watcher_vtable;
	const co_allocation_callbacks_0 allocation_callbacks;
	message_pump_0_time_watcher_0_service_0_impl_0* const service;
	std::mutex mutex;
	message_pump_0_time_watcher_0_wait_list_0 waits;
	size_t generation = 0;
};

const co_time_watcher_0_vtable message_pump_0_time_watcher_0_impl_0::time_watcher_vtable = {
	.api_version = CO_API_VERSION_0,
	.destroy = reinterpret_cast<co_time_watcher_0_destroy_fn>(&message_pump_0_time_watcher_0_impl_0::destroy),
	.get_generation_0 = reinterpret_cast<co_time_watcher_0_get_generation_0_fn>(&message_pump_0_time_watcher_0_impl_0::get_generation_0),
	.wait_change_0 = reinterpret_cast<co_time_watcher_0_wait_change_0_fn>(&message_pump_0_time_watcher_0_impl_0::wait_change_0),
	.cancel_0 = reinterpret_cast<co_time_watcher_0_cancel_0_fn>(&message_pump_0_time_watcher_0_impl_0::cancel_0),
};

} // namespace

} // namespace co::win32

#pragma endregion

#pragma region Time watcher service

namespace co::win32
{

namespace
{

// Fans the WM_TIMECHANGE messages received by the window of its event loop out to its time watchers. Owned by the event
// loop, so changes are only seen while the event loop runs.
class message_pump_0_time_watcher_0_service_0_impl_0 final : protected co_time_watcher_0_service_0_t
{
public:

	message_pump_0_time_watcher_0_service_0_impl_0(
		const co_allocation_callbacks_0& allocation_callbacks
	) noexcept
		: allocation_callbacks({
			.structure_type = allocation_callbacks.structure_type,
			.next_structure = nullptr,
			.allocate_fn = allocation_callbacks.allocate_fn,
			.deallocate_fn = allocation_callbacks.deallocate_fn,
			.data = allocation_callbacks.data
		})
	{
		co_time_watcher_0_service_0_init(this, &service_vtable, &time_watcher_service_vtable);
	}

	~message_pump_0_time_watcher_0_service_0_impl_0()
	{
		assert(head == nullptr && "a time watcher service may be destroyed only once all its time watchers have been destroyed");
	}

	// Called by the window procedure of the event loop when the system time changed.
	void notify() noexcept
	{
		message_pump_0_time_watcher_0_wait_0* completed = nullptr;
		{
			// Time watchers cannot be destroyed while the lock is held, and their taken waits keep them alive after.
			std::lock_guard lock(mutex);
			for (message_pump_0_time_watcher_0_impl_0* time_watcher = head; time_watcher != nullptr; time_watcher = time_watcher->service_next) {
				completed = time_watcher->notify(completed);
			}
		}
		// The messages are dispatched one after another on the thread of the event loop, which orders the completions of
		// earlier changes before those of later ones.
		message_pump_0_time_watcher_0_impl_0::complete(completed);
	}

	void remove(message_pump_0_time_watcher_0_impl_0* time_watcher) noexcept
	{
		std::lock_guard lock(mutex);
		if (time_watcher->service_prev != nullptr) {
			time_watcher->service_prev->service_next = time_watcher->service_next;
		}
		else {
			head = time_watcher->service_next;
		}
		if (time_watcher->service_next != nullptr) {
			time_watcher->service_next->service_prev = time_watcher->service_prev;
		}
		time_watcher->service_prev = nullptr;
		time_watcher->service_next = nullptr;
	}

private:

	static void destroy(message_pump_0_time_watcher_0_service_0_impl_0* /* self */) noexcept
	{
		assert(false && "the time watcher service of an event loop is owned by it and must not be destroyed by the user");
	}

	static co_result_0 create_time_watcher_0(message_pump_0_time_watcher_0_service_0_impl_0* self, co_time_watcher_0* time_watcher) noexcept
	{
		message_pump_0_time_watcher_0_impl_0* time_watcher_impl = impl::new_object_0<message_pump_0_time_watcher_0_impl_0>(
			self->allocation_callbacks,
			self->allocation_callbacks,
			self
		);
		if (!time_watcher_impl) {
			return CO_RESULT_0_ERROR_OUT_OF_MEMORY;
		}
		{
			std::lock_guard lock(self->mutex);
			time_watcher_impl->service_prev = nullptr;
			time_watcher_impl->service_next = self->head;
			if (self->head != nullptr) {
				self->head->service_prev = time_watcher_impl;
			}
			self->head = time_watcher_impl;
		}
		*time_watcher = CO_TIME_WATCHER_0_CAST(time_watcher_impl);
		return CO_RESULT_0_SUCCESS;
	}

private:
	static const co_service_0_vtable service_vtable;
	static const co_time_watcher_0_service_0_vtable time_watcher_service_vtable;
	const co_allocation_callbacks_0 allocation_callbacks;
	std::mutex mutex;
	message_pump_0_time_watcher_0_impl_0* head = nullptr;
};

const co_service_0_vtable message_pump_0_time_watcher_0_service_0_impl_0::service_vtable = {
	.api_version = CO_API_VERSION_0,
	.destroy = reinterpret_cast<co_service_0_destroy_fn>(&message_pump_0_time_watcher_0_service_0_impl_0::destroy),
};

const co_time_watcher_0_service_0_vtable message_pump_0_time_watcher_0_service_0_impl_0::time_watcher_service_vtable = {
	.create_time_watcher_0 = reinterpret_cast<co_time_watcher_0_service_0_create_time_watcher_0_fn>(&message_pump_0_time_watcher_0_service_0_impl_0::create_time_watcher_0),
};

void message_pump_0_time_watcher_0_impl_0::destroy(message_pump_0_time_watcher_0_impl_0* self) noexcept
{
	self->service->remove(self);
	impl::delete_object_0(self->allocation_callbacks, self);
}

} // namespace

} // namespace co::win32

#pragma endregion

#pragma region Timer

namespace co::win32
{

namespace
{

class message_pump_0_timer_0_impl_0 final : protected co_timer_0_t
{
public:

	message_pump_0_timer_0_impl_0(
		const co_allocation_callbacks_0& allocation_callbacks,
		co_clock_0 clock
	) noexcept
		: allocation_callbacks({
			.structure_type = allocation_callbacks.structure_type,
			.next_structure = nullptr,
			.allocate_fn = allocation_callbacks.allocate_fn,
			.deallocate_fn = allocation_callbacks.deallocate_fn,
			.data = allocation_callbacks.data
		})
		, clock(clock)
	{
		co_timer_0_init(this, &timer_vtable);
	}

private:

	static void destroy(message_pump_0_timer_0_impl_0* self) noexcept
	{
		impl::delete_object_0(self->allocation_callbacks, self);
	}

	static void get_clock_0(message_pump_0_timer_0_impl_0* self, co_clock_0* clock) noexcept
	{
		*clock = self->clock;
	}

	static co_result_0 expiry_at_0(message_pump_0_timer_0_impl_0* self, int64_t time_ns) noexcept
	{
		return CO_RESULT_0_ERROR_UNKNOWN;
	}

	static co_result_0 expiry_after_0(message_pump_0_timer_0_impl_0* self, uint64_t duration_ns) noexcept
	{
		return CO_RESULT_0_ERROR_UNKNOWN;
	}

	static co_result_0 wait_0(message_pump_0_timer_0_impl_0* self, co_cancellation_0 cancellation, co_executor_0 executor, co_timer_0_wait_0_completion_fn completion_fn, void* data) noexcept
	{
		return CO_RESULT_0_ERROR_UNKNOWN;
	}

	static co_result_0 cancel_0(message_pump_0_timer_0_impl_0* self) noexcept
	{
		return CO_RESULT_0_ERROR_UNKNOWN;
	}

private:
	static const co_timer_0_vtable timer_vtable;
	const co_allocation_callbacks_0 allocation_callbacks;
	const co_clock_0 clock;
};

const co_timer_0_vtable message_pump_0_timer_0_impl_0::timer_vtable = {
	.api_version = CO_API_VERSION_0,
	.destroy = reinterpret_cast<co_timer_0_destroy_fn>(&message_pump_0_timer_0_impl_0::destroy),
	.get_clock_0 = reinterpret_cast<co_timer_0_get_clock_0_fn>(&message_pump_0_timer_0_impl_0::get_clock_0),
	.expiry_at_0 = reinterpret_cast<co_timer_0_expiry_at_0_fn>(&message_pump_0_timer_0_impl_0::expiry_at_0),
	.expiry_after_0 = reinterpret_cast<co_timer_0_expiry_after_0_fn>(&message_pump_0_timer_0_impl_0::expiry_after_0),
	.wait_0 = reinterpret_cast<co_timer_0_wait_0_fn>(&message_pump_0_timer_0_impl_0::wait_0),
	.cancel_0 = reinterpret_cast<co_timer_0_cancel_0_fn>(&message_pump_0_timer_0_impl_0::cancel_0),
};

} // namespace

} // namespace co::win32

#pragma endregion

#pragma region Timer service

namespace co::win32
{

namespace
{

class message_pump_0_timer_0_service_0_impl_0 : protected co_timer_0_service_0_t
{
public:

	message_pump_0_timer_0_service_0_impl_0(
		const co_allocation_callbacks_0& allocation_callbacks
	) noexcept
		: allocation_callbacks({
			.structure_type = allocation_callbacks.structure_type,
			.next_structure = nullptr,
			.allocate_fn = allocation_callbacks.allocate_fn,
			.deallocate_fn = allocation_callbacks.deallocate_fn,
			.data = allocation_callbacks.data
		})
	{
		co_timer_0_service_0_init(this, &service_vtable, &timer_service_vtable);
	}

private:

	static void destroy(message_pump_0_timer_0_service_0_impl_0* self) noexcept
	{
		impl::delete_object_0(self->allocation_callbacks, self);
	}

	static co_result_0 create_timer_0(message_pump_0_timer_0_service_0_impl_0* self, const co_timer_0_create_info_0* create_info, co_timer_0* timer) noexcept
	{
		message_pump_0_timer_0_impl_0* timer_impl = impl::new_object_0<message_pump_0_timer_0_impl_0>(
			self->allocation_callbacks,
			self->allocation_callbacks,
			create_info->clock
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
};

const co_service_0_vtable message_pump_0_timer_0_service_0_impl_0::service_vtable = {
	.api_version = CO_API_VERSION_0,
	.destroy = reinterpret_cast<co_service_0_destroy_fn>(&message_pump_0_timer_0_service_0_impl_0::destroy),
};

const co_timer_0_service_0_vtable message_pump_0_timer_0_service_0_impl_0::timer_service_vtable = {
	.create_timer_0 = reinterpret_cast<co_timer_0_service_0_create_timer_0_fn>(&message_pump_0_timer_0_service_0_impl_0::create_timer_0),
};

} // namespace

} // namespace co::win32

#pragma endregion

#pragma region Event loop

namespace co::win32
{

namespace
{

class message_pump_0_event_loop_0_impl_0 : protected co_event_loop_0_t
{
public:

	// Takes ownership of window_class.
	message_pump_0_event_loop_0_impl_0(
		const co_allocation_callbacks_0& allocation_callbacks,
		ATOM window_class
	) noexcept
		: allocation_callbacks({
			.structure_type = allocation_callbacks.structure_type,
			.next_structure = nullptr,
			.allocate_fn = allocation_callbacks.allocate_fn,
			.deallocate_fn = allocation_callbacks.deallocate_fn,
			.data = allocation_callbacks.data
		})
		, window_class(window_class)
		, time_watcher_service(allocation_callbacks)
		, timer_service(allocation_callbacks)
	{
		co_event_loop_0_init(this, &event_domain_vtable, &event_loop_vtable);
	}

private:

	static void destroy(message_pump_0_event_loop_0_impl_0* self) noexcept
	{
		if (UnregisterClass(MAKEINTATOM(self->window_class), NULL) == 0) {
			DWORD last_error = GetLastError();
			(void)last_error;
			std::terminate();
		}
		impl::delete_object_0(self->allocation_callbacks, self);
	}

	static co_result_0 get_service_0(message_pump_0_event_loop_0_impl_0* self, co_service_type_0 service_type, co_service_0* service) noexcept
	{
		if (service_type == CO_SERVICE_TYPE_0_TIME_WATCHER_0_SERVICE_0) {
			*service = CO_SERVICE_0_CAST(&self->time_watcher_service);
			return CO_RESULT_0_SUCCESS;
		}
		if (service_type == CO_SERVICE_TYPE_0_TIMER_0_SERVICE_0) {
			*service = CO_SERVICE_0_CAST(&self->timer_service);
			return CO_RESULT_0_SUCCESS;
		}
		return CO_RESULT_0_ERROR_SERVICE_NOT_FOUND;
	}

	static co_result_0 run_0(message_pump_0_event_loop_0_impl_0* self) noexcept
	{
		// A hidden top-level window rather than a message-only one: only top-level windows receive broadcast messages such
		// as WM_TIMECHANGE.
		HWND window = CreateWindowEx(
			0, // dwExStyle
			MAKEINTATOM(self->window_class), // lpClassName
			nullptr, // lpWindowName
			0, // dwStyle (not WS_VISIBLE)
			0, 0, 0, 0, // X, Y, nWidth, nHeight
			NULL, // hWndParent
			NULL, // hMenu
			NULL, // hInstance
			self // lpParam
		);
		if (window == NULL) {
			DWORD last_error = GetLastError();
			(void)last_error;
			return CO_RESULT_0_ERROR_SYSTEM;
		}
		MSG message = {};
		BOOL get_message_result = 0;
		while ((get_message_result = GetMessage(&message, window, 0, 0)) != 0) {
			if (get_message_result == -1) {
				DWORD last_error = GetLastError();
				(void)last_error;
				// TODO: handle error
			}
			else {
				TranslateMessage(&message);
				DispatchMessage(&message);
			}
		}
		WPARAM exit_code = message.wParam;
		(void)exit_code;
		if (DestroyWindow(window) == 0) {
			DWORD last_error = GetLastError();
			(void)last_error;
			return CO_RESULT_0_ERROR_SYSTEM;
		}
		return CO_RESULT_0_SUCCESS;
	}

	static co_result_0 stop_0(message_pump_0_event_loop_0_impl_0* self) noexcept
	{
		return CO_RESULT_0_SUCCESS;
	}

public:

	static LRESULT CALLBACK window_procedure(HWND window, UINT message , WPARAM w_param, LPARAM l_param)
	{
		message_pump_0_event_loop_0_impl_0* self = nullptr;
		if (message == WM_NCCREATE) {
			CREATESTRUCT* create_struct = reinterpret_cast<CREATESTRUCT*>(l_param);
			self = reinterpret_cast<message_pump_0_event_loop_0_impl_0*>(create_struct->lpCreateParams);
			SetWindowLongPtr(window, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
			self->window = window;
		}
		else {
			self = reinterpret_cast<message_pump_0_event_loop_0_impl_0*>(GetWindowLongPtr(window, GWLP_USERDATA));
		}
		if (self) {
			switch (message) {
				case WM_TIMECHANGE:
					self->time_watcher_service.notify();
					return 0;
			}
			return DefWindowProc(window, message, w_param, l_param);
		}
		return DefWindowProc(window, message, w_param, l_param);
	}

private:
	static const co_event_domain_0_vtable event_domain_vtable;
	static const co_event_loop_0_vtable event_loop_vtable;
	const co_allocation_callbacks_0 allocation_callbacks;
	const ATOM window_class;
	message_pump_0_time_watcher_0_service_0_impl_0 time_watcher_service;
	message_pump_0_timer_0_service_0_impl_0 timer_service;
	HWND window;
};

const co_event_domain_0_vtable message_pump_0_event_loop_0_impl_0::event_domain_vtable = {
	.api_version = CO_API_VERSION_0,
	.destroy = reinterpret_cast<co_event_domain_0_destroy_fn>(&message_pump_0_event_loop_0_impl_0::destroy),
	.get_service_0 = reinterpret_cast<co_event_domain_0_get_service_0_fn>(&message_pump_0_event_loop_0_impl_0::get_service_0),
};

const co_event_loop_0_vtable message_pump_0_event_loop_0_impl_0::event_loop_vtable = {
	.run_0 = reinterpret_cast<co_event_loop_0_run_0_fn>(&message_pump_0_event_loop_0_impl_0::run_0),
	.stop_0 = reinterpret_cast<co_event_loop_0_stop_0_fn>(&message_pump_0_event_loop_0_impl_0::stop_0),
};

} // namespace

} // namespace co::win32

#pragma endregion

#pragma region Event domain group

namespace co::win32
{

namespace
{

class message_pump_0_event_domain_group_0_impl_0 : protected co_event_domain_group_0_t
{
public:

	message_pump_0_event_domain_group_0_impl_0(
		const co_allocation_callbacks_0& allocation_callbacks,
		co_event_loop_0 event_loop
	) noexcept
		: allocation_callbacks({
			.structure_type = allocation_callbacks.structure_type,
			.next_structure = nullptr,
			.allocate_fn = allocation_callbacks.allocate_fn,
			.deallocate_fn = allocation_callbacks.deallocate_fn,
			.data = allocation_callbacks.data
		})
		, event_loop(event_loop)
	{
		co_event_domain_group_0_init(this, &event_domain_group_vtable);
	}

private:

	static void destroy(message_pump_0_event_domain_group_0_impl_0* self) noexcept
	{
		impl::delete_object_0(self->allocation_callbacks, self);
	}

	static void get_event_domain_0(message_pump_0_event_domain_group_0_impl_0* self, size_t event_domain_index, co_event_domain_0* event_domain) noexcept
	{
		*event_domain = CO_EVENT_DOMAIN_0_CAST(self->event_loop);
	}

private:
	static const co_event_domain_group_0_vtable event_domain_group_vtable;
	const co_allocation_callbacks_0 allocation_callbacks;
	co_event_loop_0 event_loop;
};

const co_event_domain_group_0_vtable message_pump_0_event_domain_group_0_impl_0::event_domain_group_vtable = {
	.api_version = CO_API_VERSION_0,
	.destroy = reinterpret_cast<co_event_domain_group_0_destroy_fn>(&message_pump_0_event_domain_group_0_impl_0::destroy),
	.get_event_domain_0 = reinterpret_cast<co_event_domain_group_0_get_event_domain_0_fn>(&message_pump_0_event_domain_group_0_impl_0::get_event_domain_0),
};

} // namespace

} // namespace co::win32

#pragma endregion

#pragma region Event domain group factory

namespace co::win32
{

namespace
{

class message_pump_0_event_domain_group_factory_0_impl_0 : protected co_event_domain_group_factory_0_t
{
public:

	message_pump_0_event_domain_group_factory_0_impl_0(
		const co_allocation_callbacks_0& allocation_callbacks,
		ATOM window_class,
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
			CO_STRUCTURE_TYPE_0_EVENT_LOOP_0_CREATE_INFO_0,
			CO_STRUCTURE_TYPE_0_WIN32_MESSAGE_PUMP_0_EVENT_LOOP_0_CREATE_INFO_0,
		}
		//, handled_structure_types{
		//}
		, provided_service_types{
			CO_SERVICE_TYPE_0_TIME_WATCHER_0_SERVICE_0,
			CO_SERVICE_TYPE_0_TIMER_0_SERVICE_0,
		}
	{
		co_event_domain_group_factory_0_init(this, &event_domain_group_factory_vtable);
	}

private:

	static void destroy(message_pump_0_event_domain_group_factory_0_impl_0* self) noexcept
	{
		impl::delete_object_0(self->allocation_callbacks, self);
	}

	static void get_member_create_info_structure_types_0(message_pump_0_event_domain_group_factory_0_impl_0* self, const co_structure_type_0** member_create_info_structure_types, size_t* member_create_info_structure_type_count) noexcept
	{
		*member_create_info_structure_types = self->member_create_info_structure_types;
		*member_create_info_structure_type_count = std::size(self->member_create_info_structure_types);
	}

	static void get_handled_structure_types_0(message_pump_0_event_domain_group_factory_0_impl_0* self, const co_structure_type_0** handled_structure_types, size_t* handled_structure_type_count) noexcept
	{
		//*handled_structure_types = self->handled_structure_types;
		//*handled_structure_type_count = std::size(self->handled_structure_types);
		*handled_structure_types = nullptr;
		*handled_structure_type_count = 0;
	}

	static void get_provided_service_types_0(message_pump_0_event_domain_group_factory_0_impl_0* self, const co_service_type_0** provided_service_types, size_t* provided_service_type_count) noexcept
	{
		*provided_service_types = self->provided_service_types;
		*provided_service_type_count = std::size(self->provided_service_types);
	}

	static void get_member_count_range_0(message_pump_0_event_domain_group_factory_0_impl_0* self, size_t* min_count, size_t* max_count) noexcept
	{
		*min_count = 1;
		*max_count = 1;
	}

	static void get_rank_0(message_pump_0_event_domain_group_factory_0_impl_0* self, size_t* rank) noexcept
	{
		*rank = 0;
	}

	static co_result_0 create_event_domain_group_0(message_pump_0_event_domain_group_factory_0_impl_0* self, const co_event_domain_group_0_create_info_0* create_info, co_event_domain_group_0* event_domain_group) noexcept
	{
		const co_allocation_callbacks_0 allocation_callbacks = self->allocation_callbacks;
		HINSTANCE instance = NULL;
		if (GetModuleHandleEx(
			GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
			reinterpret_cast<LPCWSTR>(&create_event_domain_group_0),
			&instance
		) == 0) {
			DWORD last_error = GetLastError();
			(void)last_error;
			return CO_RESULT_0_ERROR_SYSTEM;
		}
		void* event_loop_memory = impl::allocate_0(allocation_callbacks, sizeof(message_pump_0_event_loop_0_impl_0), alignof(message_pump_0_event_loop_0_impl_0));
		if (event_loop_memory == nullptr) {
			return CO_RESULT_0_ERROR_OUT_OF_MEMORY;
		}
		// TODO: exception handling
		std::wstringstream class_name_str; // TODO: use allocation callbacks as allocator
		class_name_str << L"message_pump_0_event_loop_0_impl_0_";
		class_name_str << std::hex << std::setfill(L'0') << std::setw(sizeof(std::uintptr_t) * 2) << reinterpret_cast<std::uintptr_t>(event_loop_memory);
		std::wstring class_name = class_name_str.str();
		WNDCLASSEX wnd_class = {
			.cbSize = sizeof(wnd_class),
			.style = 0,
			.lpfnWndProc = &message_pump_0_event_loop_0_impl_0::window_procedure,
			.cbClsExtra = 0,
			.cbWndExtra = 0,
			.hInstance = instance,
			.hIcon = NULL,
			.hCursor = NULL,
			.hbrBackground = NULL,
			.lpszMenuName = nullptr,
			.lpszClassName = class_name.c_str(),
			.hIconSm = NULL,
		};
		ATOM window_class = RegisterClassEx(&wnd_class);
		if (window_class == 0) {
			DWORD last_error = GetLastError();
			(void)last_error;
			impl::deallocate_0(allocation_callbacks, event_loop_memory, sizeof(message_pump_0_event_domain_group_0_impl_0), alignof(message_pump_0_event_domain_group_0_impl_0));
			return CO_RESULT_0_ERROR_SYSTEM;
		}
		co_event_loop_0 event_loop = CO_EVENT_LOOP_0_CAST(::new (event_loop_memory) message_pump_0_event_loop_0_impl_0(
			allocation_callbacks,
			window_class
		));
		*event_domain_group = CO_EVENT_DOMAIN_GROUP_0_CAST(impl::new_object_0<message_pump_0_event_domain_group_0_impl_0>(
			allocation_callbacks,
			allocation_callbacks,
			event_loop
		));
		if (!*event_domain_group) {
			impl::delete_object_0(allocation_callbacks, event_loop);
			return CO_RESULT_0_ERROR_OUT_OF_MEMORY;
		}
		return CO_RESULT_0_SUCCESS;
	}

private:
	static const co_event_domain_group_factory_0_vtable event_domain_group_factory_vtable;
	const co_allocation_callbacks_0 allocation_callbacks;
	const co_structure_type_0 member_create_info_structure_types[2];
	//const co_structure_type_0 handled_structure_types[0];
	const co_service_type_0 provided_service_types[2];
};

const co_event_domain_group_factory_0_vtable message_pump_0_event_domain_group_factory_0_impl_0::event_domain_group_factory_vtable = {
	.api_version = CO_API_VERSION_0,
	.destroy = reinterpret_cast<co_event_domain_group_factory_0_destroy_fn>(&message_pump_0_event_domain_group_factory_0_impl_0::destroy),
	.get_member_create_info_structure_types_0 = reinterpret_cast<co_event_domain_group_factory_0_get_member_create_info_structure_types_0_fn>(&message_pump_0_event_domain_group_factory_0_impl_0::get_member_create_info_structure_types_0),
	.get_handled_structure_types_0 = reinterpret_cast<co_event_domain_group_factory_0_get_handled_structure_types_0_fn>(&message_pump_0_event_domain_group_factory_0_impl_0::get_handled_structure_types_0),
	.get_provided_service_types_0 = reinterpret_cast<co_event_domain_group_factory_0_get_provided_service_types_0_fn>(&message_pump_0_event_domain_group_factory_0_impl_0::get_provided_service_types_0),
	.get_member_count_range_0 = reinterpret_cast<co_event_domain_group_factory_0_get_member_count_range_0_fn>(&message_pump_0_event_domain_group_factory_0_impl_0::get_member_count_range_0),
	.get_rank_0 = reinterpret_cast<co_event_domain_group_factory_0_get_rank_0_fn>(&message_pump_0_event_domain_group_factory_0_impl_0::get_rank_0),
	.create_event_domain_group_0 = reinterpret_cast<co_event_domain_group_factory_0_create_event_domain_group_0_fn>(&message_pump_0_event_domain_group_factory_0_impl_0::create_event_domain_group_0),
};

} // namespace

} // namespace co::win32

#pragma endregion
