#include "strand_0.hpp"

#include "instance_0.hpp"

#include <co/impl.hpp>

#include <cassert>
#include <deque>
#include <exception>
#include <mutex>
#include <new>

// Rule: Make sure functions in this file are defined in the same order as in strand_0.h file.

namespace co
{

const structure_type_0 structure_type_0_strand_0_executor_0_create_info_0 = []() { return static_cast<structure_type_0>(reinterpret_cast<uintptr_t>(&structure_type_0_strand_0_executor_0_create_info_0)); }();

namespace
{

class strand_0_executor_0_impl final : public co_executor_0_t
{
public:

	strand_0_executor_0_impl(const co_allocation_callbacks_0& allocation_callbacks, co_executor_0 inner_executor) noexcept
		: m_allocation_callbacks(allocation_callbacks)
		, m_inner_executor(inner_executor)
	{
		m_allocation_callbacks.next_structure = nullptr;
		co_executor_0_init(this, &vtable);
	}

	~strand_0_executor_0_impl()
	{
		assert(m_handlers.empty() && "a strand may be destroyed only once all of its handlers have been invoked");
		assert((!m_scheduled || m_destroyed != nullptr) && "a strand may be destroyed only while it is idle or from within its handler");
		if (m_destroyed != nullptr) {
			// Destroyed from within the running handler; tells the job not to touch the strand anymore.
			*m_destroyed = true;
		}
	}

	static const co_executor_0_vtable vtable;

private:

	static strand_0_executor_0_impl* from(co_executor_0 self) noexcept
	{
		return static_cast<strand_0_executor_0_impl*>(self);
	}

	static void destroy(co_executor_0 self) noexcept
	{
		strand_0_executor_0_impl* strand = from(self);
		impl::delete_object_0(strand->m_allocation_callbacks, strand);
	}

	static co_result_0 post_0(co_executor_0 self, co_executor_handler_0 handler) noexcept
	{
		strand_0_executor_0_impl* strand = from(self);
		return strand->post(handler);
	}

	co_result_0 post(co_executor_handler_0 handler) noexcept
	{
		std::lock_guard lock(m_mutex);
		try {
			m_handlers.push_back(handler);
		}
		catch (const std::bad_alloc&) {
			return CO_RESULT_0_ERROR_OUT_OF_MEMORY;
		}
		if (m_scheduled) {
			return CO_RESULT_0_SUCCESS;
		}
		// A rejecting inner executor neither invokes nor keeps the job, so it is safe to post while holding the lock.
		co_result_0 scheduled = schedule();
		if (scheduled != CO_RESULT_0_SUCCESS) {
			// The strand had no job, so its queue held only this handler; it stays with the caller.
			m_handlers.pop_back();
			return scheduled;
		}
		m_scheduled = true;
		return CO_RESULT_0_SUCCESS;
	}

	co_result_0 schedule() noexcept
	{
		return co_executor_0_post_0(m_inner_executor, co_executor_handler_0{.invoke_fn = &strand_0_executor_0_impl::run, .data = this});
	}

	// The job: invokes exactly one handler of the strand, then posts the job for the next one.
	static void run(void* data) noexcept
	{
		strand_0_executor_0_impl* strand = static_cast<strand_0_executor_0_impl*>(data);
		bool destroyed = false;
		co_executor_handler_0 handler = [strand, &destroyed]() {
			std::lock_guard lock(strand->m_mutex);
			assert(!strand->m_handlers.empty());
			co_executor_handler_0 handler = strand->m_handlers.front();
			strand->m_handlers.pop_front();
			strand->m_destroyed = &destroyed;
			return handler;
		}();
		handler.invoke_fn(handler.data);
		if (destroyed) {
			return;
		}
		std::lock_guard lock(strand->m_mutex);
		strand->m_destroyed = nullptr;
		if (strand->m_handlers.empty()) {
			strand->m_scheduled = false;
			return;
		}
		if (strand->schedule() != CO_RESULT_0_SUCCESS) {
			// Queued handlers were accepted by post_0() and must be invoked, but there is no context left to invoke them in.
			std::terminate();
		}
	}

private:
	co_allocation_callbacks_0 m_allocation_callbacks;
	co_executor_0 m_inner_executor = nullptr;
	std::mutex m_mutex;
	std::deque<co_executor_handler_0> m_handlers;
	bool m_scheduled = false; // a job of the strand is posted to, or running on, the inner executor
	bool* m_destroyed = nullptr; // while a handler runs: flag of the running job, set when the strand is destroyed
};

const co_executor_0_vtable strand_0_executor_0_impl::vtable = {
	.api_version = CO_API_VERSION_0,
	.destroy = &strand_0_executor_0_impl::destroy,
	.post_0 = &strand_0_executor_0_impl::post_0,
};

} // namespace

} // namespace co

const co_structure_type_0 CO_STRUCTURE_TYPE_0_STRAND_0_EXECUTOR_0_CREATE_INFO_0 = []() { return static_cast<co_structure_type_0>(reinterpret_cast<uintptr_t>(&co::structure_type_0_strand_0_executor_0_create_info_0)); }();

co_result_0 co_strand_0_executor_0_create_0(co_instance_0 instance, const co_strand_0_executor_0_create_info_0* create_info, co_strand_0_executor_0* strand_executor) noexcept
{
	using namespace co::detail;

	if (strand_executor == nullptr || create_info->structure_type != CO_STRUCTURE_TYPE_0_STRAND_0_EXECUTOR_0_CREATE_INFO_0 || create_info->inner_executor == nullptr) {
		report(instance, CO_DEBUG_SEVERITY_0_ERROR, CO_RESULT_0_ERROR_INVALID_ARGUMENT, "co_strand_0_executor_0_create_0: invalid create info or null strand", nullptr);
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
	co::strand_0_executor_0_impl* strand = co::impl::new_object_0<co::strand_0_executor_0_impl>(*allocation_callbacks, *allocation_callbacks, create_info->inner_executor);
	if (strand == nullptr) {
		report(instance, CO_DEBUG_SEVERITY_0_ERROR, CO_RESULT_0_ERROR_OUT_OF_MEMORY, "co_strand_0_executor_0_create_0: cannot allocate the strand", nullptr);
		return CO_RESULT_0_ERROR_OUT_OF_MEMORY;
	}
	*strand_executor = CO_STRAND_0_EXECUTOR_0_CAST(static_cast<co_executor_0>(strand));
	return CO_RESULT_0_SUCCESS;
}
