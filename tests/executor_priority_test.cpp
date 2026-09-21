#include <co/executor_priority_0.hpp>

#include <gtest/gtest.h>

#include <cstddef>

static_assert(co::detail::in_structure_of<co_executor_priority_info_0, co_in_structure_0>);
static_assert(co::detail::in_structure_of<co::executor_priority_info_0, co::in_structure_0>);
static_assert(sizeof(co_executor_priority_info_0) == sizeof(co::executor_priority_info_0));
static_assert(offsetof(co_executor_priority_info_0, priority) == offsetof(co::executor_priority_info_0, priority));

static_assert(CO_EXECUTOR_PRIORITY_0_LOWEST == -CO_EXECUTOR_PRIORITY_0_HIGHEST);
static_assert(CO_EXECUTOR_PRIORITY_0_LOW == -CO_EXECUTOR_PRIORITY_0_HIGH);
static_assert(CO_EXECUTOR_PRIORITY_0_LOWEST < CO_EXECUTOR_PRIORITY_0_LOW);
static_assert(CO_EXECUTOR_PRIORITY_0_LOW < CO_EXECUTOR_PRIORITY_0_NORMAL);
static_assert(CO_EXECUTOR_PRIORITY_0_NORMAL < CO_EXECUTOR_PRIORITY_0_HIGH);
static_assert(CO_EXECUTOR_PRIORITY_0_HIGH < CO_EXECUTOR_PRIORITY_0_HIGHEST);
static_assert(co::executor_priority_0_high == CO_EXECUTOR_PRIORITY_0_HIGH);

TEST(c_executor_priority_info_0, links_into_executor_create_info_as_hint)
{
	const co_executor_priority_info_0 priority = {
		.structure_type = CO_STRUCTURE_TYPE_0_EXECUTOR_PRIORITY_INFO_0,
		.next_structure = NULL,
		.priority = CO_EXECUTOR_PRIORITY_0_HIGH,
	};
	const co_hint_info_0 priority_hint = {
		.structure_type = CO_STRUCTURE_TYPE_0_HINT_INFO_0,
		.next_structure = NULL,
		.hint = CO_IN_STRUCTURE_0_CAST(&priority),
	};
	const co_executor_0_create_info_0 executor = {
		.structure_type = CO_STRUCTURE_TYPE_0_EXECUTOR_0_CREATE_INFO_0,
		.next_structure = CO_IN_STRUCTURE_0_CAST(&priority_hint),
	};
	EXPECT_EQ(co_in_structure_0_find_0(executor.next_structure, CO_STRUCTURE_TYPE_0_EXECUTOR_PRIORITY_INFO_0), nullptr);
	const co_in_structure_0* hint = co_in_structure_0_find_0(executor.next_structure, CO_STRUCTURE_TYPE_0_HINT_INFO_0);
	ASSERT_NE(hint, nullptr);
	EXPECT_EQ(reinterpret_cast<const co_hint_info_0*>(hint)->hint->structure_type, CO_STRUCTURE_TYPE_0_EXECUTOR_PRIORITY_INFO_0);
	EXPECT_NE(CO_STRUCTURE_TYPE_0_EXECUTOR_PRIORITY_INFO_0, CO_STRUCTURE_TYPE_0_HINT_INFO_0);
}
