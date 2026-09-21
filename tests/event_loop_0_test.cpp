#include <co.hpp>

#include <gtest/gtest.h>

#include <cstddef>
#include <type_traits>

static_assert(std::is_standard_layout_v<co_event_loop_0_create_info_0>);
static_assert(offsetof(co_event_loop_0_create_info_0, structure_type) == offsetof(co_in_structure_0, structure_type));
static_assert(offsetof(co_event_loop_0_create_info_0, next_structure) == offsetof(co_in_structure_0, next_structure));
static_assert(std::is_standard_layout_v<co::event_loop_0_create_info_0>);
static_assert(offsetof(co::event_loop_0_create_info_0, structure_type) == offsetof(co::in_structure_0, structure_type));
static_assert(offsetof(co::event_loop_0_create_info_0, next_structure) == offsetof(co::in_structure_0, next_structure));
static_assert(sizeof(co_event_loop_0_create_info_0) == sizeof(co::event_loop_0_create_info_0));
static_assert(offsetof(co_event_loop_0_create_info_0, structure_type) == offsetof(co::event_loop_0_create_info_0, structure_type));
static_assert(offsetof(co_event_loop_0_create_info_0, next_structure) == offsetof(co::event_loop_0_create_info_0, next_structure));
static_assert(offsetof(co_event_loop_0_create_info_0, required_services) == offsetof(co::event_loop_0_create_info_0, required_services));
static_assert(offsetof(co_event_loop_0_create_info_0, required_service_count) == offsetof(co::event_loop_0_create_info_0, required_service_count));
