#include "co.hpp"

#include <cstddef>
#include <type_traits>

// Rule: Make sure functions in this file are defined in the same order as in co.h file.

namespace co
{

const structure_type_0 structure_type_0_executor_0_create_info_0 = []() { return static_cast<structure_type_0>(reinterpret_cast<uintptr_t>(&structure_type_0_executor_0_create_info_0)); }();

} // namespace co

const co_structure_type_0 CO_STRUCTURE_TYPE_0_EXECUTOR_0_CREATE_INFO_0 = []() { return static_cast<co_structure_type_0>(reinterpret_cast<uintptr_t>(&co::structure_type_0_executor_0_create_info_0)); }();

static_assert(std::is_standard_layout_v<co_executor_0_t>);
static_assert(offsetof(co_executor_0_vtable, api_version) == 0);

void co_executor_0_destroy(co_executor_0 executor) noexcept
{
	if (executor == nullptr) {
		return;
	}
	executor->vtable->destroy(executor);
}

co_result_0 co_executor_0_post_0(co_executor_0 executor, co_executor_handler_0 handler) noexcept
{
	return executor->vtable->post_0(executor, handler);
}
