#include "time_watcher_0.hpp"

#include <cstddef>
#include <type_traits>

// Rule: Make sure functions in this file are defined in the same order as in time_watcher_0.h file.

static_assert(std::is_standard_layout_v<co_time_watcher_0_t>);
static_assert(offsetof(co_time_watcher_0_vtable, api_version) == 0);

void co_time_watcher_0_destroy(co_time_watcher_0 time_watcher) noexcept
{
	if (time_watcher == nullptr) {
		return;
	}
	time_watcher->vtable->destroy(time_watcher);
}

void co_time_watcher_0_get_generation_0(co_time_watcher_0 time_watcher, size_t* generation) noexcept
{
	time_watcher->vtable->get_generation_0(time_watcher, generation);
}

co_result_0 co_time_watcher_0_wait_change_0(co_time_watcher_0 time_watcher, size_t after_generation, co_cancellation_0 cancellation, co_executor_0 executor, co_time_watcher_0_wait_change_0_completion_fn completion_fn, void* data) noexcept
{
	return time_watcher->vtable->wait_change_0(time_watcher, after_generation, cancellation, executor, completion_fn, data);
}

co_result_0 co_time_watcher_0_cancel_0(co_time_watcher_0 time_watcher) noexcept
{
	return time_watcher->vtable->cancel_0(time_watcher);
}
