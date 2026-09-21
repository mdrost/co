#include <co.hpp>

#include <gtest/gtest.h>

#include <cstddef>
#include <type_traits>

static_assert(std::is_standard_layout_v<co_runtime_0_create_info_0>);
static_assert(offsetof(co_runtime_0_create_info_0, structure_type) == offsetof(co_in_structure_0, structure_type));
static_assert(offsetof(co_runtime_0_create_info_0, next_structure) == offsetof(co_in_structure_0, next_structure));
static_assert(std::is_standard_layout_v<co::runtime_0_create_info_0>);
static_assert(offsetof(co::runtime_0_create_info_0, structure_type) == offsetof(co::in_structure_0, structure_type));
static_assert(offsetof(co::runtime_0_create_info_0, next_structure) == offsetof(co::in_structure_0, next_structure));
static_assert(sizeof(co_runtime_0_create_info_0) == sizeof(co::runtime_0_create_info_0));
static_assert(offsetof(co_runtime_0_create_info_0, structure_type) == offsetof(co::runtime_0_create_info_0, structure_type));
static_assert(offsetof(co_runtime_0_create_info_0, next_structure) == offsetof(co::runtime_0_create_info_0, next_structure));

static_assert(std::is_standard_layout_v<co_event_domain_group_0_create_info_0>);
static_assert(offsetof(co_event_domain_group_0_create_info_0, structure_type) == offsetof(co_in_structure_0, structure_type));
static_assert(offsetof(co_event_domain_group_0_create_info_0, next_structure) == offsetof(co_in_structure_0, next_structure));
static_assert(sizeof(co_event_domain_group_0_create_info_0) == sizeof(co::event_domain_group_0_create_info_0));
static_assert(offsetof(co_event_domain_group_0_create_info_0, event_domain_create_info) == offsetof(co::event_domain_group_0_create_info_0, event_domain_create_info));
static_assert(offsetof(co_event_domain_group_0_create_info_0, event_domain_count) == offsetof(co::event_domain_group_0_create_info_0, event_domain_count));
static_assert(std::is_standard_layout_v<co_executor_0_create_info_0>);
static_assert(offsetof(co_executor_0_create_info_0, structure_type) == offsetof(co_in_structure_0, structure_type));
static_assert(offsetof(co_executor_0_create_info_0, next_structure) == offsetof(co_in_structure_0, next_structure));
static_assert(sizeof(co_executor_0_create_info_0) == sizeof(co::executor_0_create_info_0));

static_assert(std::is_standard_layout_v<co_runtime_0_event_domain_group_info_0>);
static_assert(sizeof(co_runtime_0_event_domain_group_info_0) == sizeof(co::runtime_0_event_domain_group_info_0));
static_assert(offsetof(co_runtime_0_event_domain_group_info_0, id) == offsetof(co::runtime_0_event_domain_group_info_0, id));
static_assert(offsetof(co_runtime_0_event_domain_group_info_0, create_info) == offsetof(co::runtime_0_event_domain_group_info_0, create_info));

static_assert(std::is_standard_layout_v<co_runtime_0_executor_info_0>);
static_assert(sizeof(co_runtime_0_executor_info_0) == sizeof(co::runtime_0_executor_info_0));
static_assert(offsetof(co_runtime_0_executor_info_0, id) == offsetof(co::runtime_0_executor_info_0, id));
static_assert(offsetof(co_runtime_0_executor_info_0, event_domain_group_id) == offsetof(co::runtime_0_executor_info_0, event_domain_group_id));
static_assert(offsetof(co_runtime_0_executor_info_0, create_info) == offsetof(co::runtime_0_executor_info_0, create_info));

static_assert(offsetof(co_runtime_0_create_info_0, event_domain_groups) == offsetof(co::runtime_0_create_info_0, event_domain_groups));
static_assert(offsetof(co_runtime_0_create_info_0, executors) == offsetof(co::runtime_0_create_info_0, executors));
