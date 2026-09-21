#include "thread_count_bounds_0.hpp"

#include <cstddef>

namespace co
{

const structure_type_0 structure_type_0_thread_count_bounds_info_0 = []() { return static_cast<structure_type_0>(reinterpret_cast<uintptr_t>(&structure_type_0_thread_count_bounds_info_0)); }();

} // namespace co

const co_structure_type_0 CO_STRUCTURE_TYPE_0_THREAD_COUNT_BOUNDS_INFO_0 = []() { return static_cast<co_structure_type_0>(reinterpret_cast<uintptr_t>(&co::structure_type_0_thread_count_bounds_info_0)); }();

static_assert(co::detail::in_structure_of<co_thread_count_bounds_info_0, co_in_structure_0>);
static_assert(co::detail::in_structure_of<co::thread_count_bounds_info_0, co::in_structure_0>);
static_assert(sizeof(co_thread_count_bounds_info_0) == sizeof(co::thread_count_bounds_info_0));
static_assert(offsetof(co_thread_count_bounds_info_0, minimum_thread_count) == offsetof(co::thread_count_bounds_info_0, minimum_thread_count));
static_assert(offsetof(co_thread_count_bounds_info_0, maximum_thread_count) == offsetof(co::thread_count_bounds_info_0, maximum_thread_count));
