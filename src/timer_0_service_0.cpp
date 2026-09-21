#include "timer_0_service_0.hpp"

namespace co
{

const service_type_0 service_type_0_timer_0_service_0 = []() { return static_cast<service_type_0>(reinterpret_cast<uintptr_t>(&service_type_0_timer_0_service_0)); }();

} // namespace co

const co_service_type_0 CO_SERVICE_TYPE_0_TIMER_0_SERVICE_0 = []() { return static_cast<co_service_type_0>(reinterpret_cast<uintptr_t>(&co::service_type_0_timer_0_service_0)); }();

co_result_0 co_timer_0_service_0_create_timer_0(co_timer_0_service_0 timer_service, co_timer_0* timer) noexcept
{
	return timer_service->vtable->create_timer_0(timer_service, timer);
}
