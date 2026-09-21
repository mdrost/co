#include "timer_0.hpp"

#include <cstddef>
#include <type_traits>

// Rule: Make sure functions in this file are defined in the same order as in timer_0.h file.

namespace co
{

const structure_type_0 structure_type_0_timer_0_create_info_0 = []() { return static_cast<structure_type_0>(reinterpret_cast<uintptr_t>(&structure_type_0_timer_0_create_info_0)); }();

} // namespace co

const co_structure_type_0 CO_STRUCTURE_TYPE_0_TIMER_0_CREATE_INFO_0 = []() { return static_cast<co_structure_type_0>(reinterpret_cast<uintptr_t>(&co::structure_type_0_timer_0_create_info_0)); }();

static_assert(co::detail::in_structure_of<co_timer_0_create_info_0, co_in_structure_0>);
static_assert(co::detail::in_structure_of<co::timer_0_create_info_0, co::in_structure_0>);
static_assert(sizeof(co_timer_0_create_info_0) == sizeof(co::timer_0_create_info_0));
static_assert(offsetof(co_timer_0_create_info_0, clock) == offsetof(co::timer_0_create_info_0, clock));

static_assert(std::is_standard_layout_v<co_timer_0_t>);
static_assert(offsetof(co_timer_0_vtable, api_version) == 0);

void co_timer_0_destroy(co_timer_0 timer) noexcept
{
	if (timer == nullptr) {
		return;
	}
	timer->vtable->destroy(timer);
}

void co_timer_0_get_clock_0(co_timer_0 timer, co_clock_0* clock) noexcept
{
	timer->vtable->get_clock_0(timer, clock);
}

co_result_0 co_timer_0_expiry_at_0(co_timer_0 timer, int64_t time_ns) noexcept
{
	return timer->vtable->expiry_at_0(timer, time_ns);
}

co_result_0 co_timer_0_expiry_after_0(co_timer_0 timer, uint64_t duration_ns) noexcept
{
	return timer->vtable->expiry_after_0(timer, duration_ns);
}

co_result_0 co_timer_0_wait_0(co_timer_0 timer, co_cancellation_0 cancellation, co_executor_0 executor, co_timer_0_wait_0_completion_fn completion_fn, void* data) noexcept
{
	return timer->vtable->wait_0(timer, cancellation, executor, completion_fn, data);
}

co_result_0 co_timer_0_cancel_0(co_timer_0 timer) noexcept
{
	return timer->vtable->cancel_0(timer);
}

co_result_0 co_timer_0_expiry_at_calendar_0(co_timer_0 timer, const co_calendar_time_0* calendar_time) noexcept
{
	co_clock_0 clock;
	timer->vtable->get_clock_0(timer, &clock);
	int64_t time_ns;
	const co_result_0 result = co_clock_0_time_from_calendar_0(clock, calendar_time, &time_ns);
	if (result != CO_RESULT_0_SUCCESS) {
		return result;
	}
	return timer->vtable->expiry_at_0(timer, time_ns);
}
