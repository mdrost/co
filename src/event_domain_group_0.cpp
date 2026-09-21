#include "instance_0.hpp"

#include "co.hpp"

#include <algorithm>
#include <cstddef>
#include <span>
#include <type_traits>

// Rule: Make sure functions in this file are defined in the same order as in co.h file.

static_assert(std::is_standard_layout_v<co_event_domain_group_0_t>);
static_assert(offsetof(co_event_domain_group_0_vtable, api_version) == 0);

namespace co
{

const structure_type_0 structure_type_0_event_domain_group_0_create_info_0 = []() { return static_cast<structure_type_0>(reinterpret_cast<uintptr_t>(&structure_type_0_event_domain_group_0_create_info_0)); }();

} // namespace co

const co_structure_type_0 CO_STRUCTURE_TYPE_0_EVENT_DOMAIN_GROUP_0_CREATE_INFO_0 = []() { return static_cast<co_structure_type_0>(reinterpret_cast<uintptr_t>(&co::structure_type_0_event_domain_group_0_create_info_0)); }();

namespace co::detail
{

namespace
{

// The members of an event domain create info read by the library.
struct generic_create_info final
{
	const co_service_type_0* required_services;
	std::size_t required_service_count;
};

bool contains(const co_structure_type_0* types, std::size_t count, co_structure_type_0 type) noexcept
{
	return std::find(types, types + count, type) != types + count;
}

bool contains(const co_service_type_0* services, std::size_t count, co_service_type_0 service) noexcept
{
	return std::find(services, services + count, service) != services + count;
}

bool is_candidate(const registered_class& registered, co_structure_type_0 member_create_info_structure_type, const generic_create_info& generic, const co_in_structure_0* chain, std::size_t member_count) noexcept
{
	if (!contains(registered.member_create_info_structure_types, registered.member_create_info_structure_type_count, member_create_info_structure_type)) {
		return false;
	}
	if (member_count < registered.min_member_count || member_count > registered.max_member_count) {
		return false;
	}
	for (std::size_t i = 0; i < generic.required_service_count; ++i) {
		if (!contains(registered.provided_service_types, registered.provided_service_type_count, generic.required_services[i])) {
			return false;
		}
	}
	for (const co_in_structure_0* entry = chain; entry != nullptr; entry = entry->next_structure) {
		if (entry->structure_type != CO_STRUCTURE_TYPE_0_ALLOCATION_CALLBACKS_0 && entry->structure_type != CO_STRUCTURE_TYPE_0_HINT_INFO_0 && !contains(registered.handled_structure_types, registered.handled_structure_type_count, entry->structure_type)) {
			return false;
		}
	}
	return true;
}

// Whether a precedes b in the order the candidates are tried: higher rank first, then later registered first.
bool precedes(const registered_class* a, const registered_class* b) noexcept
{
	return a->rank != b->rank ? a->rank > b->rank : a > b;
}

// The first candidate tried after previous, or the first one when previous is null.
const registered_class* select_class(co_instance_0 instance, co_structure_type_0 member_create_info_structure_type, const generic_create_info& generic, const co_in_structure_0* chain, std::size_t member_count, const registered_class* previous) noexcept
{
	const registered_class* selected = nullptr;
	for (const registered_class& registered : std::span(instance->classes, instance->class_count)) {
		if ((previous == nullptr || precedes(previous, &registered)) && (selected == nullptr || precedes(&registered, selected)) && is_candidate(registered, member_create_info_structure_type, generic, chain, member_count)) {
			selected = &registered;
		}
	}
	return selected;
}

co_result_0 check_group(co_instance_0 instance, co_event_domain_group_0 group, std::size_t member_count, const generic_create_info& generic) noexcept
{
	co_result_0 result;
	for (std::size_t domain_event_index = 0; domain_event_index < member_count; ++domain_event_index) {
		co_event_domain_0 member;
		group->vtable->get_event_domain_0(group, domain_event_index, &member);
		// TODO: do we really need to check api version?
		if (!is_compatible_api_version(member->vtable->api_version)) {
			report(instance, CO_DEBUG_SEVERITY_0_ERROR, CO_RESULT_0_ERROR_INVALID_ARGUMENT, "a group created an invalid event domain", group);
			return CO_RESULT_0_ERROR_INVALID_ARGUMENT;
		}
		for (std::size_t i = 0; i < generic.required_service_count; ++i) {
			co_service_0 service = nullptr;
			if (member->vtable->get_service_0(member, generic.required_services[i], &service) != CO_RESULT_0_SUCCESS || service == nullptr) {
				report(instance, CO_DEBUG_SEVERITY_0_ERROR, CO_RESULT_0_ERROR_SERVICE_NOT_FOUND, "an event domain does not provide a service its factory declared", member);
				return CO_RESULT_0_ERROR_SERVICE_NOT_FOUND;
			}
		}
	}
	return CO_RESULT_0_SUCCESS;
}

} // namespace

co_result_0 builtin_event_domain_group_factories_0(void* /* data */, const co_event_domain_group_factory_0_entry_point_info_0* /* info */, std::size_t* factory_count, co_event_domain_group_factory_0* /* factories */) noexcept
{
	// TODO: create the factories of the built-in classes as they are ported to event domain groups.
	*factory_count = 0;
	return CO_RESULT_0_SUCCESS;
}

co_result_0 create_event_domain_group(co_instance_0 instance, const co_event_domain_0_create_info_0* member_create_info, std::size_t member_count, co_event_domain_group_0* group_) noexcept
{
	// The member create info is passed on unchanged: the allocation callbacks are given to the class in the next chain of
	// the group create info.
	const generic_create_info generic = {.required_services = member_create_info->required_services, .required_service_count = member_create_info->required_service_count};
	if (generic.required_service_count != 0 && generic.required_services == nullptr) {
		report(instance, CO_DEBUG_SEVERITY_0_ERROR, CO_RESULT_0_ERROR_INVALID_ARGUMENT, "required_services is null", nullptr);
		return CO_RESULT_0_ERROR_INVALID_ARGUMENT;
	}

	const co_allocation_callbacks_0* allocation_callbacks = nullptr;
	if (co_result_0 result = find_allocation_callbacks(instance, member_create_info->next_structure, &allocation_callbacks); result != CO_RESULT_0_SUCCESS) {
		return result;
	}
	co_allocation_callbacks_0 group_callbacks = *allocation_callbacks;
	group_callbacks.next_structure = nullptr;

	const co_event_domain_group_0_create_info_0 group_create_info = {
		.structure_type = CO_STRUCTURE_TYPE_0_EVENT_DOMAIN_GROUP_0_CREATE_INFO_0,
		.next_structure = CO_IN_STRUCTURE_0_CAST(&group_callbacks),
		.event_domain_create_info = member_create_info,
		.event_domain_count = member_count,
	};
	// A class declining the values of the create infos with CO_RESULT_0_ERROR_NOT_SUPPORTED passes the request on to the
	// next candidate. Any other failure ends the selection.
	co_event_domain_group_factory_0 factory = nullptr;
	co_event_domain_group_0 group = nullptr;
	for (const registered_class* selected = nullptr;;) {
		selected = select_class(instance, member_create_info->structure_type, generic, member_create_info->next_structure, member_count, selected);
		if (selected == nullptr) {
			report(instance, CO_DEBUG_SEVERITY_0_ERROR, CO_RESULT_0_ERROR_NOT_SUPPORTED, "no registered class matches the event domain create info", nullptr);
			return CO_RESULT_0_ERROR_NOT_SUPPORTED;
		}
		factory = selected->factory;
		co_result_0 result = factory->vtable->create_event_domain_group_0(factory, &group_create_info, &group);
		if (result == CO_RESULT_0_SUCCESS) {
			break;
		}
		if (result != CO_RESULT_0_ERROR_NOT_SUPPORTED) {
			report(instance, CO_DEBUG_SEVERITY_0_ERROR, result, "the chosen class failed to create the event domain group", factory);
			return result;
		}
		report(instance, CO_DEBUG_SEVERITY_0_INFO, result, "a candidate class declined the event domain create info; trying the next one", factory);
		group = nullptr;
	}
	if (group == nullptr || group->vtable == nullptr || !is_compatible_api_version(group->vtable->api_version) || group->vtable->get_event_domain_0 == nullptr || group->vtable->destroy == nullptr) {
		report(instance, CO_DEBUG_SEVERITY_0_ERROR, CO_RESULT_0_ERROR_INVALID_ARGUMENT, "the chosen class created an invalid event domain group", factory);
		return CO_RESULT_0_ERROR_INVALID_ARGUMENT;
	}
	if (co_result_0 result = check_group(instance, group, member_count, generic); result != CO_RESULT_0_SUCCESS) {
		group->vtable->destroy(group);
		return result;
	}
	*group_ = group;
	return CO_RESULT_0_SUCCESS;
}

} // namespace co::detail
