#include "executor_priority_0.hpp"

#include <cstddef>

namespace co
{

const structure_type_0 structure_type_0_executor_priority_info_0 = []() { return static_cast<structure_type_0>(reinterpret_cast<uintptr_t>(&structure_type_0_executor_priority_info_0)); }();

} // namespace co

const co_structure_type_0 CO_STRUCTURE_TYPE_0_EXECUTOR_PRIORITY_INFO_0 = []() { return static_cast<co_structure_type_0>(reinterpret_cast<uintptr_t>(&co::structure_type_0_executor_priority_info_0)); }();

static_assert(co::detail::in_structure_of<co_executor_priority_info_0, co_in_structure_0>);
static_assert(co::detail::in_structure_of<co::executor_priority_info_0, co::in_structure_0>);
static_assert(sizeof(co_executor_priority_info_0) == sizeof(co::executor_priority_info_0));
static_assert(offsetof(co_executor_priority_info_0, priority) == offsetof(co::executor_priority_info_0, priority));
