#include "fixed_thread_pool_0.hpp"

#include <cstddef>

namespace co
{

const structure_type_0 structure_type_0_fixed_thread_pool_0_create_info_0 = []() { return static_cast<structure_type_0>(reinterpret_cast<uintptr_t>(&structure_type_0_fixed_thread_pool_0_create_info_0)); }();

} // namespace co

const co_structure_type_0 CO_STRUCTURE_TYPE_0_FIXED_THREAD_POOL_0_CREATE_INFO_0 = []() { return static_cast<co_structure_type_0>(reinterpret_cast<uintptr_t>(&co::structure_type_0_fixed_thread_pool_0_create_info_0)); }();

static_assert(co::detail::in_structure_of<co_fixed_thread_pool_0_create_info_0, co_in_structure_0>);
static_assert(co::detail::in_structure_of<co::fixed_thread_pool_0_create_info_0, co::in_structure_0>);
static_assert(sizeof(co_fixed_thread_pool_0_create_info_0) == sizeof(co::fixed_thread_pool_0_create_info_0));
static_assert(offsetof(co_fixed_thread_pool_0_create_info_0, thread_count) == offsetof(co::fixed_thread_pool_0_create_info_0, thread_count));
