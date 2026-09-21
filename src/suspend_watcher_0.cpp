#include "suspend_watcher_0.hpp"

#include <cstddef>
#include <type_traits>

// Rule: Make sure functions in this file are defined in the same order as in suspend_watcher_0.h file.

static_assert(std::is_standard_layout_v<co_suspend_watcher_0_t>);
static_assert(offsetof(co_suspend_watcher_0_vtable, api_version) == 0);

void co_suspend_watcher_0_destroy(co_suspend_watcher_0 suspend_watcher) noexcept
{
	if (suspend_watcher == nullptr) {
		return;
	}
	suspend_watcher->vtable->destroy(suspend_watcher);
}

void co_suspend_watcher_0_get_suspend_generation_0(co_suspend_watcher_0 suspend_watcher, size_t* generation) noexcept
{
	suspend_watcher->vtable->get_suspend_generation_0(suspend_watcher, generation);
}

void co_suspend_watcher_0_get_resume_generation_0(co_suspend_watcher_0 suspend_watcher, size_t* generation) noexcept
{
	suspend_watcher->vtable->get_resume_generation_0(suspend_watcher, generation);
}

co_result_0 co_suspend_watcher_0_wait_suspend_0(co_suspend_watcher_0 suspend_watcher, size_t after_generation, co_cancellation_0 cancellation, co_executor_0 executor, co_suspend_watcher_0_wait_suspend_0_completion_fn completion_fn, void* data) noexcept
{
	return suspend_watcher->vtable->wait_suspend_0(suspend_watcher, after_generation, cancellation, executor, completion_fn, data);
}

co_result_0 co_suspend_watcher_0_wait_resume_0(co_suspend_watcher_0 suspend_watcher, size_t after_generation, co_cancellation_0 cancellation, co_executor_0 executor, co_suspend_watcher_0_wait_resume_0_completion_fn completion_fn, void* data) noexcept
{
	return suspend_watcher->vtable->wait_resume_0(suspend_watcher, after_generation, cancellation, executor, completion_fn, data);
}

co_result_0 co_suspend_watcher_0_cancel_0(co_suspend_watcher_0 suspend_watcher) noexcept
{
	return suspend_watcher->vtable->cancel_0(suspend_watcher);
}
