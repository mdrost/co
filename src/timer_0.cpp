#include "timer_0.hpp"

#include <cstddef>
#include <type_traits>

// Rule: Make sure functions in this file are defined in the same order as in timer_0.h file.

static_assert(std::is_standard_layout_v<co_timer_0_t>);
static_assert(offsetof(co_timer_0_vtable, api_version) == 0);

void co_timer_0_destroy(co_timer_0 timer) noexcept
{
	if (timer == nullptr) {
		return;
	}
	timer->vtable->destroy(timer);
}

co_result_0 co_timer_0_start_0(co_timer_0 timer, uint64_t due_ns, co_executor_0 executor, co_timer_0_start_0_completion_fn completion_fn, void* data) noexcept
{
	return timer->vtable->start_0(timer, due_ns, executor, completion_fn, data);
}

co_result_0 co_timer_0_cancel_0(co_timer_0 timer) noexcept
{
	return timer->vtable->cancel_0(timer);
}
