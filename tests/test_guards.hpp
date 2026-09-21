#pragma once

// Scope guards destroying C API objects when leaving the scope, also when an ASSERT_* fails.
// A guard holding null does nothing, so it may be created right after a create call that is expected to fail.

// Guards are destroyed in reverse declaration order, so guard an object after the objects it uses.
// guard() creates the guard of a C handle or of a C++ wrapper: auto strand_guard = co_test::guard(strand);

#include <co.hpp>
#include <co/strand_0.hpp>

#include <memory>

namespace co_test
{

struct destroyer final
{
	void operator()(co_instance_0 instance) const noexcept { co_instance_0_destroy(instance); }
	void operator()(co_runtime_0 runtime) const noexcept { co_runtime_0_destroy(runtime); }
	void operator()(co_event_domain_0 event_domain) const noexcept { co_event_domain_0_destroy(event_domain); }
	void operator()(co_event_loop_0 event_loop) const noexcept { co_event_domain_0_destroy(&event_loop->event_domain); }
	void operator()(co_event_facility_0 event_facility) const noexcept { co_event_domain_0_destroy(&event_facility->event_domain); }
	void operator()(co_service_0 service) const noexcept { co_service_0_destroy(service); }
	void operator()(co_executor_0 executor) const noexcept { co_executor_0_destroy(executor); }
	void operator()(co_strand_0_executor_0 strand) const noexcept { co_strand_0_executor_0_destroy(strand); }
};

using instance_0_guard = std::unique_ptr<co_instance_0_t, destroyer>;
using runtime_0_guard = std::unique_ptr<co_runtime_0_t, destroyer>;
using event_domain_0_guard = std::unique_ptr<co_event_domain_0_t, destroyer>;
using event_loop_0_guard = std::unique_ptr<co_event_loop_0_t, destroyer>;
using event_facility_0_guard = std::unique_ptr<co_event_facility_0_t, destroyer>;
using service_0_guard = std::unique_ptr<co_service_0_t, destroyer>;
using executor_0_guard = std::unique_ptr<co_executor_0_t, destroyer>;
using strand_0_executor_0_guard = std::unique_ptr<co_strand_0_executor_0_t, destroyer>;

[[nodiscard]] inline instance_0_guard guard(co_instance_0 instance) noexcept { return instance_0_guard(instance); }
[[nodiscard]] inline runtime_0_guard guard(co_runtime_0 runtime) noexcept { return runtime_0_guard(runtime); }
[[nodiscard]] inline event_domain_0_guard guard(co_event_domain_0 event_domain) noexcept { return event_domain_0_guard(event_domain); }
[[nodiscard]] inline event_loop_0_guard guard(co_event_loop_0 event_loop) noexcept { return event_loop_0_guard(event_loop); }
[[nodiscard]] inline event_facility_0_guard guard(co_event_facility_0 event_facility) noexcept { return event_facility_0_guard(event_facility); }
[[nodiscard]] inline service_0_guard guard(co_service_0 service) noexcept { return service_0_guard(service); }
[[nodiscard]] inline executor_0_guard guard(co_executor_0 executor) noexcept { return executor_0_guard(executor); }
[[nodiscard]] inline strand_0_executor_0_guard guard(co_strand_0_executor_0 strand) noexcept { return strand_0_executor_0_guard(strand); }

[[nodiscard]] inline instance_0_guard guard(co::instance_0 instance) noexcept { return guard(static_cast<co_instance_0>(instance)); }
[[nodiscard]] inline runtime_0_guard guard(co::runtime_0 runtime) noexcept { return guard(static_cast<co_runtime_0>(runtime)); }
[[nodiscard]] inline event_domain_0_guard guard(co::event_domain_0 event_domain) noexcept { return guard(static_cast<co_event_domain_0>(event_domain)); }
[[nodiscard]] inline event_loop_0_guard guard(co::event_loop_0 event_loop) noexcept { return guard(static_cast<co_event_loop_0>(event_loop)); }
[[nodiscard]] inline event_facility_0_guard guard(co::event_facility_0 event_facility) noexcept { return guard(static_cast<co_event_facility_0>(event_facility)); }
[[nodiscard]] inline service_0_guard guard(co::service_0 service) noexcept { return guard(static_cast<co_service_0>(service)); }
[[nodiscard]] inline executor_0_guard guard(co::executor_0 executor) noexcept { return guard(static_cast<co_executor_0>(executor)); }
[[nodiscard]] inline strand_0_executor_0_guard guard(co::strand_0_executor_0 strand) noexcept { return guard(static_cast<co_strand_0_executor_0>(strand)); }

} // namespace co_test
