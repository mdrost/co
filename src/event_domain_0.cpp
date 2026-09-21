#include "instance_0.hpp"

#include <co.hpp>

#include <cstddef>
#include <type_traits>

static_assert(std::is_standard_layout_v<co_event_domain_0_t>);
static_assert(offsetof(co_event_domain_0_vtable, api_version) == 0);

void co_event_domain_0_destroy(co_event_domain_0 event_domain) noexcept
{
	if (event_domain == nullptr) {
		return;
	}
	event_domain->vtable->destroy(event_domain);
}

co_result_0 co_event_domain_0_get_service_0(co_event_domain_0 event_domain, co_service_type_0 service_type, co_service_0* service) noexcept
{
	return event_domain->vtable->get_service_0(event_domain, service_type, service);
}
