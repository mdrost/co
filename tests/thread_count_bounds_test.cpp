#include <co/dynamic_thread_pool_0.hpp>
#include <co/thread_count_bounds_0.hpp>

#include <gtest/gtest.h>

#include <cstddef>

static_assert(co::detail::in_structure_of<co_thread_count_bounds_info_0, co_in_structure_0>);
static_assert(co::detail::in_structure_of<co::thread_count_bounds_info_0, co::in_structure_0>);
static_assert(sizeof(co_thread_count_bounds_info_0) == sizeof(co::thread_count_bounds_info_0));
static_assert(offsetof(co_thread_count_bounds_info_0, minimum_thread_count) == offsetof(co::thread_count_bounds_info_0, minimum_thread_count));
static_assert(offsetof(co_thread_count_bounds_info_0, maximum_thread_count) == offsetof(co::thread_count_bounds_info_0, maximum_thread_count));

TEST(c_thread_count_bounds_info_0, links_into_chain_with_dynamic_thread_pool_create_info)
{
	const co_thread_count_bounds_info_0 bounds = {
		.structure_type = CO_STRUCTURE_TYPE_0_THREAD_COUNT_BOUNDS_INFO_0,
		.next_structure = NULL,
		.minimum_thread_count = 1,
		.maximum_thread_count = 8,
	};
	const co_dynamic_thread_pool_0_create_info_0 dynamic_thread_pool = {
		.structure_type = CO_STRUCTURE_TYPE_0_DYNAMIC_THREAD_POOL_0_CREATE_INFO_0,
		.next_structure = CO_IN_STRUCTURE_0_CAST(&bounds),
	};
	const co_in_structure_0* next = dynamic_thread_pool.next_structure;
	ASSERT_NE(next, nullptr);
	EXPECT_EQ(next->structure_type, CO_STRUCTURE_TYPE_0_THREAD_COUNT_BOUNDS_INFO_0);
	EXPECT_NE(CO_STRUCTURE_TYPE_0_THREAD_COUNT_BOUNDS_INFO_0, CO_STRUCTURE_TYPE_0_DYNAMIC_THREAD_POOL_0_CREATE_INFO_0);
}
