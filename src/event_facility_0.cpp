#include "co.hpp"

#include <cstddef>
#include <type_traits>

// Rule: Make sure functions in this file are defined in the same order as in co.h file.

static_assert(std::is_standard_layout_v<co_event_facility_0_t>);
static_assert(offsetof(co_event_facility_0_t, event_domain) == 0);
static_assert(co::detail::event_domain_create_info_of<co_event_facility_0_create_info_0, co_event_domain_0_create_info_0>);
static_assert(co::detail::event_domain_create_info_of<co::event_facility_0_create_info_0, co::event_domain_0_create_info_0>);

namespace co
{

const structure_type_0 structure_type_0_event_facility_0_create_info_0 = []() { return static_cast<structure_type_0>(reinterpret_cast<uintptr_t>(&structure_type_0_event_facility_0_create_info_0)); }();

} // namespace co

const co_structure_type_0 CO_STRUCTURE_TYPE_0_EVENT_FACILITY_0_CREATE_INFO_0 = []() { return static_cast<co_structure_type_0>(reinterpret_cast<uintptr_t>(&co::structure_type_0_event_facility_0_create_info_0)); }();
