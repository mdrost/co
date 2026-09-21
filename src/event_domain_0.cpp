#include "instance_0.hpp"

#include <co.hpp>

#include <cstddef>
#include <type_traits>

static_assert(std::is_standard_layout_v<co_event_domain_0_t>);
static_assert(offsetof(co_event_domain_0_vtable, api_version) == 0);

co_result_0 co_instance_0_create_event_domain_0(co_instance_0 instance, const co_event_domain_0_create_info_0* create_info, co_event_domain_0* event_domain) noexcept
{
	co_result_0 result;
	if (event_domain == nullptr) {
		return CO_RESULT_0_ERROR_INVALID_ARGUMENT;
	}
	co_event_domain_group_0 group = nullptr;
	result = co::detail::create_event_domain_group(instance, create_info, 1, &group);
	if (result != CO_RESULT_0_SUCCESS) {
		return result;
	}
	group->vtable->get_event_domain_0(group, 0, event_domain);
	// TODO: why we don't destroy group?
	return CO_RESULT_0_SUCCESS;
}

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
