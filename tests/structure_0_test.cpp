#include <co.hpp>
#include <co/dynamic_thread_pool_0.hpp>

#include <gtest/gtest.h>

#include <cstddef>
#include <type_traits>

static_assert(sizeof(co_in_structure_0) == sizeof(co::in_structure_0));
static_assert(offsetof(co_in_structure_0, structure_type) == offsetof(co::in_structure_0, structure_type));
static_assert(offsetof(co_in_structure_0, next_structure) == offsetof(co::in_structure_0, next_structure));

namespace
{

template <class T>
concept castable_to_c_header = requires (const T* structure) { co::detail::in_structure_cast<co_in_structure_0>(structure); };

template <class T>
concept castable_to_cpp_header = requires (const T* structure) { co::in_structure_0_cast(structure); };

struct no_header final
{
	int value;
};

struct swapped_header final
{
	const co_in_structure_0* next_structure;
	co_structure_type_0 structure_type;
};

struct untyped_next final
{
	co_structure_type_0 structure_type;
	const void* next_structure;
};

struct next_of_other_header final
{
	co_structure_type_0 structure_type;
	const co::in_structure_0* next_structure;
};

// Input structures are accepted, whatever follows their header.
static_assert(castable_to_c_header<co_dynamic_thread_pool_0_create_info_0>);
static_assert(castable_to_c_header<co_in_structure_0>);
static_assert(castable_to_cpp_header<co::dynamic_thread_pool_0_create_info_0>);
static_assert(castable_to_cpp_header<co::in_structure_0>);

// Anything else is rejected.
static_assert(!castable_to_c_header<int>);
static_assert(!castable_to_c_header<no_header>);
static_assert(!castable_to_c_header<swapped_header>);
static_assert(!castable_to_c_header<untyped_next>);
static_assert(!castable_to_c_header<next_of_other_header>);
static_assert(!castable_to_c_header<co::dynamic_thread_pool_0_create_info_0>);
static_assert(!castable_to_cpp_header<co_dynamic_thread_pool_0_create_info_0>);
static_assert(!castable_to_cpp_header<next_of_other_header>);

// The casts keep the constness of the structure.
static_assert(std::is_same_v<decltype(CO_IN_STRUCTURE_0_CAST(std::declval<co_dynamic_thread_pool_0_create_info_0*>())), co_in_structure_0*>);
static_assert(std::is_same_v<decltype(CO_IN_STRUCTURE_0_CAST(std::declval<const co_dynamic_thread_pool_0_create_info_0*>())), const co_in_structure_0*>);
static_assert(std::is_same_v<decltype(co::in_structure_0_cast(std::declval<co::dynamic_thread_pool_0_create_info_0*>())), co::in_structure_0*>);
static_assert(std::is_same_v<decltype(co::in_structure_0_cast(std::declval<const co::dynamic_thread_pool_0_create_info_0*>())), const co::in_structure_0*>);
static_assert(std::is_same_v<decltype(co::in_structure_0_cast(std::declval<co::dynamic_thread_pool_0_create_info_0&>())), co::in_structure_0&>);
static_assert(std::is_same_v<decltype(co::in_structure_0_cast(std::declval<const co::dynamic_thread_pool_0_create_info_0&>())), const co::in_structure_0&>);
static_assert(std::is_same_v<decltype(co::in_structure_0_cast(std::declval<co::dynamic_thread_pool_0_create_info_0>())), const co::in_structure_0&>);

} // namespace

TEST(c_in_structure_0, cast_links_structure_into_chain)
{
	const co_dynamic_thread_pool_0_create_info_0 dynamic = {
		.structure_type = CO_STRUCTURE_TYPE_0_DYNAMIC_THREAD_POOL_0_CREATE_INFO_0,
		.next_structure = nullptr,
	};
	const co_in_structure_0* header = CO_IN_STRUCTURE_0_CAST(&dynamic);
	EXPECT_EQ(static_cast<const void*>(header), static_cast<const void*>(&dynamic));
	EXPECT_EQ(header->structure_type, CO_STRUCTURE_TYPE_0_DYNAMIC_THREAD_POOL_0_CREATE_INFO_0);
	EXPECT_EQ(header->next_structure, nullptr);
}

TEST(cpp_in_structure_0, cast_links_structure_into_chain)
{
	const co::dynamic_thread_pool_0_create_info_0 dynamic = {
		.structure_type = co::structure_type_0_dynamic_thread_pool_0_create_info_0,
		.next_structure = nullptr,
	};
	const co::in_structure_0* header = co::in_structure_0_cast(&dynamic);
	EXPECT_EQ(static_cast<const void*>(header), static_cast<const void*>(&dynamic));
	EXPECT_EQ(header->structure_type, co::structure_type_0_dynamic_thread_pool_0_create_info_0);
}
