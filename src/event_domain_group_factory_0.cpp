#include "co.hpp"

// Rule: Make sure functions in this file are defined in the same order as in co.h file.

static_assert(std::is_standard_layout_v<co_event_domain_group_factory_0_t>);
static_assert(offsetof(co_event_domain_group_factory_0_vtable, api_version) == 0);
static_assert(std::is_standard_layout_v<co_event_domain_group_factory_0_entry_point_info_0>);
static_assert(offsetof(co_event_domain_group_factory_0_entry_point_info_0, structure_type) == offsetof(co_in_structure_0, structure_type));
static_assert(offsetof(co_event_domain_group_factory_0_entry_point_info_0, next_structure) == offsetof(co_in_structure_0, next_structure));

void co_event_domain_group_factory_0_destroy(co_event_domain_group_factory_0 event_domain_group_factory) noexcept
{
	event_domain_group_factory->vtable->destroy(event_domain_group_factory);
}

void co_event_domain_group_factory_0_get_member_create_info_structure_types_0(co_event_domain_group_factory_0 event_domain_group_factory, const co_structure_type_0** member_create_info_structure_types, size_t* member_create_info_structure_type_count) noexcept
{
	event_domain_group_factory->vtable->get_member_create_info_structure_types_0(event_domain_group_factory, member_create_info_structure_types, member_create_info_structure_type_count);
}

void co_event_domain_group_factory_0_get_handled_structure_types_0(co_event_domain_group_factory_0 event_domain_group_factory, const co_structure_type_0** handled_structure_types, size_t* handled_structure_type_count) noexcept
{
	event_domain_group_factory->vtable->get_handled_structure_types_0(event_domain_group_factory, handled_structure_types, handled_structure_type_count);
}

void co_event_domain_group_factory_0_get_provided_service_types_0(co_event_domain_group_factory_0 event_domain_group_factory, const co_service_type_0** provided_service_types, size_t* provided_service_type_count) noexcept
{
	event_domain_group_factory->vtable->get_provided_service_types_0(event_domain_group_factory, provided_service_types, provided_service_type_count);
}

void co_event_domain_group_factory_0_get_member_count_range_0(co_event_domain_group_factory_0 event_domain_group_factory, size_t* min_count, size_t* max_count) noexcept
{
	event_domain_group_factory->vtable->get_member_count_range_0(event_domain_group_factory, min_count, max_count);
}

void co_event_domain_group_factory_0_get_rank_0(co_event_domain_group_factory_0 event_domain_group_factory, size_t* rank) noexcept
{
	event_domain_group_factory->vtable->get_rank_0(event_domain_group_factory, rank);
}

co_result_0 co_event_domain_group_factory_0_create_event_domain_group_0(co_event_domain_group_factory_0 event_domain_group_factory, const co_event_domain_group_0_create_info_0* create_info, co_event_domain_group_0* group) noexcept
{
	return event_domain_group_factory->vtable->create_event_domain_group_0(event_domain_group_factory, create_info, group);
}
