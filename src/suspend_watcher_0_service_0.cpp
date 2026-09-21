#include "suspend_watcher_0_service_0.hpp"

namespace co
{

const service_type_0 service_type_0_suspend_watcher_0_service_0 = []() { return static_cast<service_type_0>(reinterpret_cast<uintptr_t>(&service_type_0_suspend_watcher_0_service_0)); }();

} // namespace co

const co_service_type_0 CO_SERVICE_TYPE_0_SUSPEND_WATCHER_0_SERVICE_0 = []() { return static_cast<co_service_type_0>(reinterpret_cast<uintptr_t>(&co::service_type_0_suspend_watcher_0_service_0)); }();

co_result_0 co_suspend_watcher_0_service_0_create_suspend_watcher_0(co_suspend_watcher_0_service_0 suspend_watcher_service, co_suspend_watcher_0* suspend_watcher) noexcept
{
	return suspend_watcher_service->vtable->create_suspend_watcher_0(suspend_watcher_service, suspend_watcher);
}
