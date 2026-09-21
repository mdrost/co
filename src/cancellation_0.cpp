#include <co.hpp>

#include <co/impl.hpp>
#include "instance_0.hpp"

#include <atomic>
#include <cassert>
#include <condition_variable>
#include <mutex>
#include <thread>

namespace co
{

const structure_type_0 structure_type_0_cancellation_0_create_info_0 = []() { return static_cast<structure_type_0>(reinterpret_cast<uintptr_t>(&structure_type_0_cancellation_0_create_info_0)); }();

} // namespace co

const co_structure_type_0 CO_STRUCTURE_TYPE_0_CANCELLATION_0_CREATE_INFO_0 = []() { return static_cast<co_structure_type_0>(reinterpret_cast<uintptr_t>(&co::structure_type_0_cancellation_0_create_info_0)); }();

// The only implementation of cancellations, so the handle type is the implementation itself.
struct co_cancellation_0_t final
{
public:

	explicit co_cancellation_0_t(co_instance_0 instance, const co_allocation_callbacks_0& allocation_callbacks) noexcept
		: instance(instance)
		, allocation_callbacks{
			.structure_type = allocation_callbacks.structure_type,
			.next_structure = nullptr,
			.allocate_fn = allocation_callbacks.allocate_fn,
			.deallocate_fn = allocation_callbacks.deallocate_fn,
		}
	{
	}

	~co_cancellation_0_t()
	{
		assert(head == nullptr && running == nullptr && "a cancellation may be destroyed only once every registration has been unregistered");
	}

	void destroy() noexcept
	{
		co::impl::delete_object_0(allocation_callbacks, this);
	}

	void request_0() noexcept
	{
		std::unique_lock lock(mutex);
		if (requested.load(std::memory_order_relaxed)) {
			return;
		}
		requested.store(true, std::memory_order_release);
		requesting_thread = std::this_thread::get_id();
		// Callbacks run without the lock held, so that they may unregister any registration, including their own.
		while (head != nullptr) {
			co_cancellation_0_registration_0* registration = head;
			unlink(registration);
			running = registration;
			lock.unlock();
			registration->callback_fn(registration->data);
			lock.lock();
			running = nullptr;
			done.notify_all();
		}
		requesting_thread = std::thread::id();
	}

	bool is_requested_0() const noexcept
	{
		return requested.load(std::memory_order_acquire);
	}

	void reset_0() noexcept
	{
		std::lock_guard lock(mutex);
		assert(head == nullptr && running == nullptr && "a cancellation may be reset only once every registration has been unregistered");
		requested.store(false, std::memory_order_relaxed);
	}

	bool register_0(co_cancellation_0_registration_0* registration) noexcept
	{
		assert(registration->callback_fn != nullptr);
		std::lock_guard lock(mutex);
		owner(registration) = nullptr;
		if (requested.load(std::memory_order_relaxed)) {
			return false;
		}
		prev(registration) = nullptr;
		next(registration) = head;
		if (head != nullptr) {
			prev(head) = registration;
		}
		head = registration;
		owner(registration) = this;
		return true;
	}

	void unregister_0(co_cancellation_0_registration_0* registration) noexcept
	{
		std::unique_lock lock(mutex);
		if (owner(registration) != nullptr) {
			assert(owner(registration) == this);
			unlink(registration);
			return;
		}
		// The callback may be running; wait for it to return unless it is unregistering itself (or another callback of
		// this cancellation is unregistering it), which would deadlock.
		if (requesting_thread != std::this_thread::get_id()) {
			done.wait(lock, [this, registration]() { return running != registration; });
		}
	}

private:

	// Private members of a registration: the links of the list of the registrations not yet called, and the cancellation
	// while the registration is in that list.
	static co_cancellation_0_registration_0*& prev(co_cancellation_0_registration_0* registration) noexcept
	{
		return reinterpret_cast<co_cancellation_0_registration_0*&>(registration->reserved[0]);
	}

	static co_cancellation_0_registration_0*& next(co_cancellation_0_registration_0* registration) noexcept
	{
		return reinterpret_cast<co_cancellation_0_registration_0*&>(registration->reserved[1]);
	}

	static void*& owner(co_cancellation_0_registration_0* registration) noexcept
	{
		return registration->reserved[2];
	}

	void unlink(co_cancellation_0_registration_0* registration) noexcept
	{
		if (prev(registration) != nullptr) {
			next(prev(registration)) = next(registration);
		}
		else {
			head = next(registration);
		}
		if (next(registration) != nullptr) {
			prev(next(registration)) = prev(registration);
		}
		owner(registration) = nullptr;
	}

	// First, so that exported functions can find the per-instance dispatch table of future layers from the handle alone
	// (see docs/design/extensibility.md).
	co_instance_0 instance;
	co_allocation_callbacks_0 allocation_callbacks;
	std::mutex mutex;
	std::condition_variable done;
	std::atomic<bool> requested = false;
	co_cancellation_0_registration_0* head = nullptr;
	co_cancellation_0_registration_0* running = nullptr;
	std::thread::id requesting_thread;
};

// TODO: move to instance_0.cpp
co_result_0 co_instance_0_create_cancellation_0(co_instance_0 instance, const co_cancellation_0_create_info_0* create_info, co_cancellation_0* cancellation) noexcept
{
	co_result_0 result = co::detail::check_next_chain(instance, create_info->next_structure);
	if (result != CO_RESULT_0_SUCCESS) {
		return result;
	}
	const co_allocation_callbacks_0* allocation_callbacks = nullptr;
	result = co::detail::find_allocation_callbacks(instance, create_info->next_structure, &allocation_callbacks);
	if (result != CO_RESULT_0_SUCCESS) {
		return result;
	}
	co_cancellation_0 created = co::impl::new_object_0<co_cancellation_0_t>(*allocation_callbacks, instance, *allocation_callbacks);
	if (created == nullptr) {
		co::detail::report(instance, CO_DEBUG_SEVERITY_0_ERROR, "co_instance_0_create_cancellation_0: cannot allocate the cancellation", nullptr, CO_RESULT_0_ERROR_OUT_OF_MEMORY);
		return CO_RESULT_0_ERROR_OUT_OF_MEMORY;
	}
	*cancellation = created;
	return CO_RESULT_0_SUCCESS;
}

void co_cancellation_0_destroy(co_cancellation_0 cancellation) noexcept
{
	if (cancellation == nullptr) {
		return;
	}
	cancellation->destroy();
}

void co_cancellation_0_request_0(co_cancellation_0 cancellation) noexcept
{
	cancellation->request_0();
}

bool co_cancellation_0_is_requested_0(co_cancellation_0 cancellation) noexcept
{
	return cancellation->is_requested_0();
}

void co_cancellation_0_reset_0(co_cancellation_0 cancellation) noexcept
{
	cancellation->reset_0();
}

bool co_cancellation_0_register_0(co_cancellation_0 cancellation, co_cancellation_0_registration_0* registration) noexcept
{
	return cancellation->register_0(registration);
}

void co_cancellation_0_unregister_0(co_cancellation_0 cancellation, co_cancellation_0_registration_0* registration) noexcept
{
	cancellation->unregister_0(registration);
}
