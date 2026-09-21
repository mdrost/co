#include <co.hpp>

#include <gtest/gtest.h>

#include <cstddef>
#include <type_traits>

static_assert(std::is_standard_layout_v<co_executor_0_create_info_0>);
static_assert(offsetof(co_executor_0_create_info_0, structure_type) == offsetof(co_in_structure_0, structure_type));
static_assert(offsetof(co_executor_0_create_info_0, next_structure) == offsetof(co_in_structure_0, next_structure));
static_assert(std::is_standard_layout_v<co::executor_0_create_info_0>);
static_assert(offsetof(co::executor_0_create_info_0, structure_type) == offsetof(co::in_structure_0, structure_type));
static_assert(offsetof(co::executor_0_create_info_0, next_structure) == offsetof(co::in_structure_0, next_structure));
static_assert(sizeof(co_executor_0_create_info_0) == sizeof(co::executor_0_create_info_0));
static_assert(offsetof(co_executor_0_create_info_0, structure_type) == offsetof(co::executor_0_create_info_0, structure_type));
static_assert(offsetof(co_executor_0_create_info_0, next_structure) == offsetof(co::executor_0_create_info_0, next_structure));

static_assert(std::is_standard_layout_v<co_executor_0_t>);
static_assert(offsetof(co_executor_0_vtable, api_version) == 0);

namespace
{

// An executor class implemented in plain C style: the object starts with a co_executor_0_t.
struct counting_executor final
{
	co_executor_0_t base;
	int posted;
	int* destroyed;
};

co_result_0 counting_post(co_executor_0 self, co_executor_handler_0 handler) noexcept
{
	counting_executor* executor = reinterpret_cast<counting_executor*>(self);
	++executor->posted;
	handler.invoke_fn(handler.data);
	return CO_RESULT_0_SUCCESS;
}

void counting_destroy(co_executor_0 self) noexcept
{
	counting_executor* executor = reinterpret_cast<counting_executor*>(self);
	++*executor->destroyed;
	delete executor;
}

const co_executor_0_vtable counting_vtable = {
	.api_version = CO_API_VERSION_0,
	.destroy = &counting_destroy,
	.post_0 = &counting_post,
};

void count_invoke(void* data) noexcept
{
	++*static_cast<int*>(data);
}

} // namespace

TEST(c_executor_0, init_sets_vtable)
{
	co_executor_0_t executor{};
	co_executor_0_init(&executor, &counting_vtable);
	EXPECT_EQ(executor.vtable, &counting_vtable);
	EXPECT_EQ(executor.vtable->api_version, CO_API_VERSION_0);
}

TEST(c_executor_0, dispatches_post_0_and_destroy_through_vtable)
{
	int destroyed = 0;
	counting_executor* object = new counting_executor{.base = {}, .posted = 0, .destroyed = &destroyed};
	co_executor_0_init(&object->base, &counting_vtable);
	co_executor_0 executor = &object->base;

	int invoked = 0;
	EXPECT_EQ(co_executor_0_post_0(executor, co_executor_handler_0{.invoke_fn = &count_invoke, .data = &invoked}), CO_RESULT_0_SUCCESS);
	EXPECT_EQ(invoked, 1);
	EXPECT_EQ(object->posted, 1);

	co_executor_0_destroy(executor);
	EXPECT_EQ(destroyed, 1);
}

TEST(c_executor_0, destroy_ignores_null)
{
	co_executor_0_destroy(nullptr);
}
