#include <co.hpp>

#include "test_guards.hpp"

#include <gtest/gtest.h>

#include <algorithm>
#include <array>
#include <cstddef>
#include <new>
#include <span>
#include <string>
#include <vector>

// Registration of event domain group classes through factory sources in the instance create info, and creation of
// standalone event domains of the chosen class.

namespace
{

const int timer_service_tag = 0;
const int socket_service_tag = 0;
const int tag_a_tag = 0;
const int tag_b_tag = 0;

co_service_type_0 service_type(const int& tag) noexcept
{
	return static_cast<co_service_type_0>(reinterpret_cast<uintptr_t>(&tag));
}

co_structure_type_0 structure_type(const int& tag) noexcept
{
	return static_cast<co_structure_type_0>(reinterpret_cast<uintptr_t>(&tag));
}

struct test_log final
{
	std::vector<std::string> events;
	// Allocation callbacks found in the member create info of the last created group.
	std::size_t group_allocation_callbacks_count = 0;
	void* group_allocation_data = nullptr;
	// Allocation callbacks found in the info of the last entry point call.
	std::size_t entry_point_allocation_callbacks_count = 0;
	co_version_0 entry_point_api_version = 0;
	// Hint wrapped in the member create info of the last created group.
	const co_in_structure_0* member_hint = nullptr;
};

struct class_config final
{
	std::string name;
	std::vector<co_structure_type_0> member_create_info_structure_types = {CO_STRUCTURE_TYPE_0_EVENT_LOOP_0_CREATE_INFO_0};
	std::vector<co_structure_type_0> handled_structure_types;
	std::vector<co_service_type_0> provided_service_types;
	std::size_t min_member_count = 1;
	std::size_t max_member_count = 1;
	std::uintptr_t rank = 0;
	// Members provide none of the declared services.
	bool members_lack_services = false;
	// When not success, create_event_domain_group_0 fails with it.
	co_result_0 create_result = CO_RESULT_0_SUCCESS;
};

std::size_t count_allocation_callbacks(const co_in_structure_0* chain, void** data) noexcept
{
	std::size_t count = 0;
	for (; chain != nullptr; chain = chain->next_structure) {
		if (chain->structure_type == CO_STRUCTURE_TYPE_0_ALLOCATION_CALLBACKS_0) {
			*data = reinterpret_cast<const co_allocation_callbacks_0*>(chain)->data;
			++count;
		}
	}
	return count;
}

template <class T, class... Args>
T* allocate_object(const co_allocation_callbacks_0& callbacks, Args&&... args) noexcept
{
	void* memory = callbacks.allocate_fn(callbacks.data, sizeof(T), alignof(T));
	return memory != nullptr ? ::new (memory) T(std::forward<Args>(args)...) : nullptr;
}

template <class T>
void deallocate_object(const co_allocation_callbacks_0& callbacks, T* object) noexcept
{
	const co_allocation_callbacks_0 copy = callbacks;
	object->~T();
	copy.deallocate_fn(copy.data, object, sizeof(T), alignof(T));
}

struct test_factory;

// Used for both kinds: a co_event_loop_0_t starts with the co_event_domain_0_t that is the whole co_event_facility_0_t.
struct test_member final : co_event_loop_0_t
{
	const test_factory* factory = nullptr;
	co_event_domain_group_0 group = nullptr;
	int runs = 0;
	int stops = 0;
};

struct test_group final : co_event_domain_group_0_t
{
	test_factory* factory = nullptr;
	co_allocation_callbacks_0 allocation_callbacks = {};
	std::array<test_member, 4> members;
};

struct test_factory final : co_event_domain_group_factory_0_t
{
	const class_config* config = nullptr;
	test_log* log = nullptr;
	co_allocation_callbacks_0 allocation_callbacks = {};
};

void member_destroy(test_member* self) noexcept
{
	co_event_domain_group_0 group = static_cast<test_member*>(CO_EVENT_LOOP_0_CAST(self))->group;
	group->vtable->destroy(group);
}

co_result_0 member_get_service(test_member* self, co_service_type_0 type, co_service_0* service) noexcept
{
	const class_config& config = *self->factory->config;
	if (config.members_lack_services || std::ranges::find(config.provided_service_types, type) == config.provided_service_types.end()) {
		return CO_RESULT_0_ERROR_SERVICE_NOT_FOUND;
	}
	*service = reinterpret_cast<co_service_0>(self); // TODO: wtf?
	return CO_RESULT_0_SUCCESS;
}

co_result_0 member_run(test_member* self) noexcept
{
	++self->runs;
	return CO_RESULT_0_SUCCESS;
}

co_result_0 member_stop(test_member* self) noexcept
{
	++self->stops;
	return CO_RESULT_0_SUCCESS;
}

const co_event_domain_0_vtable member_event_domain_vtable = {
	.api_version = CO_API_VERSION_0,
	.destroy = reinterpret_cast<co_event_domain_0_destroy_fn>(&member_destroy),
	.get_service_0 = reinterpret_cast<co_event_domain_0_get_service_0_fn>(&member_get_service),
};

const co_event_loop_0_vtable member_event_loop_vtable = {
	.run_0 = reinterpret_cast<co_event_loop_0_run_0_fn>(&member_run),
	.stop_0 = reinterpret_cast<co_event_loop_0_stop_0_fn>(&member_stop),
};

void group_destroy(test_group* self) noexcept
{
	self->factory->log->events.push_back("destroy group " + self->factory->config->name);
	deallocate_object(self->allocation_callbacks, self);
}

void group_get_event_domain(test_group* self, std::size_t domain_event_index, co_event_domain_0* event_domain) noexcept
{
	*event_domain = &self->members[domain_event_index].event_domain;
}

const co_event_domain_group_0_vtable group_vtable = {
	.api_version = CO_API_VERSION_0,
	.destroy = reinterpret_cast<co_event_domain_group_0_destroy_fn>(&group_destroy),
	.get_event_domain_0 = reinterpret_cast<co_event_domain_group_0_get_event_domain_0_fn>(&group_get_event_domain),
};

void factory_destroy(test_factory* self) noexcept
{
	self->log->events.push_back("destroy factory " + self->config->name);
	deallocate_object(self->allocation_callbacks, self);
}

void factory_get_member_create_info_structure_types(test_factory* self, const co_structure_type_0** member_create_info_structure_types, size_t* member_create_info_structure_type_count) noexcept
{
	*member_create_info_structure_types = self->config->member_create_info_structure_types.data();
	*member_create_info_structure_type_count = self->config->member_create_info_structure_types.size();
}

void factory_get_handled_structure_types(test_factory* self, const co_structure_type_0** handled_structure_types, size_t* handled_structure_type_count) noexcept
{
	*handled_structure_types = self->config->handled_structure_types.data();
	*handled_structure_type_count = self->config->handled_structure_types.size();
}

void factory_get_provided_service_types(test_factory* self, const co_service_type_0** provided_service_types, size_t* provided_service_type_count) noexcept
{
	*provided_service_types = self->config->provided_service_types.data();
	*provided_service_type_count = self->config->provided_service_types.size();
}

void factory_get_member_count_range(test_factory* self, size_t* min_count, size_t* max_count) noexcept
{
	*min_count = self->config->min_member_count;
	*max_count = self->config->max_member_count;
}

void factory_get_rank(test_factory* self, size_t* rank) noexcept
{
	*rank = self->config->rank;
}

co_result_0 factory_create_group(test_factory* self, const co_event_domain_group_0_create_info_0* create_info, co_event_domain_group_0* group_) noexcept
{
	test_log& log = *self->log;
	log.group_allocation_callbacks_count = count_allocation_callbacks(create_info->next_structure, &log.group_allocation_data);
	const co_hint_info_0* hint = reinterpret_cast<const co_hint_info_0*>(co_in_structure_0_find_0(create_info->event_domain_create_info->next_structure, CO_STRUCTURE_TYPE_0_HINT_INFO_0));
	log.member_hint = hint != nullptr ? hint->hint : nullptr;
	const co_allocation_callbacks_0* callbacks = reinterpret_cast<const co_allocation_callbacks_0*>(co_in_structure_0_find_0(create_info->next_structure, CO_STRUCTURE_TYPE_0_ALLOCATION_CALLBACKS_0));
	if (callbacks == nullptr || create_info->event_domain_count > 4) {
		return CO_RESULT_0_ERROR_INVALID_ARGUMENT;
	}
	if (self->config->create_result != CO_RESULT_0_SUCCESS) {
		log.events.push_back("decline " + self->config->name);
		return self->config->create_result;
	}
	test_group* group = allocate_object<test_group>(*callbacks);
	if (group == nullptr) {
		return CO_RESULT_0_ERROR_OUT_OF_MEMORY;
	}
	co_event_domain_group_0_init(group, &group_vtable);
	group->factory = self;
	group->allocation_callbacks = *callbacks;
	group->allocation_callbacks.next_structure = nullptr;
	for (std::size_t i = 0; i < create_info->event_domain_count; ++i) {
		co_event_loop_0_init(&group->members[i], &member_event_domain_vtable, &member_event_loop_vtable);
		group->members[i].factory = self;
		group->members[i].group = group;
	}
	log.events.push_back("create group " + self->config->name);
	*group_ = group;
	return CO_RESULT_0_SUCCESS;
}

const co_event_domain_group_factory_0_vtable factory_vtable = {
	.api_version = CO_API_VERSION_0,
	.destroy = reinterpret_cast<co_event_domain_group_factory_0_destroy_fn>(&factory_destroy),
	.get_member_create_info_structure_types_0 = reinterpret_cast<co_event_domain_group_factory_0_get_member_create_info_structure_types_0_fn>(&factory_get_member_create_info_structure_types),
	.get_handled_structure_types_0 = reinterpret_cast<co_event_domain_group_factory_0_get_handled_structure_types_0_fn>(&factory_get_handled_structure_types),
	.get_provided_service_types_0 = reinterpret_cast<co_event_domain_group_factory_0_get_provided_service_types_0_fn>(&factory_get_provided_service_types),
	.get_member_count_range_0 = reinterpret_cast<co_event_domain_group_factory_0_get_member_count_range_0_fn>(&factory_get_member_count_range),
	.get_rank_0 = reinterpret_cast<co_event_domain_group_factory_0_get_rank_0_fn>(&factory_get_rank),
	.create_event_domain_group_0 = reinterpret_cast<co_event_domain_group_factory_0_create_event_domain_group_0_fn>(&factory_create_group),
};

// A source of factories: the data of a co_event_domain_group_factory_source_0.
struct test_source final
{
	std::vector<const class_config*> classes;
	test_log* log = nullptr;
	// Result of the call creating the factories.
	co_result_0 result = CO_RESULT_0_SUCCESS;
};

co_result_0 test_entry_point(void* data, const co_event_domain_group_factory_0_entry_point_info_0* info, std::size_t* factory_count, co_event_domain_group_factory_0* factories) noexcept
{
	test_source* source = static_cast<test_source*>(data);
	if (factories == nullptr) {
		*factory_count = source->classes.size();
		return CO_RESULT_0_SUCCESS;
	}
	if (source->result != CO_RESULT_0_SUCCESS) {
		return source->result;
	}
	void* unused = nullptr;
	source->log->entry_point_allocation_callbacks_count = count_allocation_callbacks(info->next_structure, &unused);
	source->log->entry_point_api_version = info->api_version;
	const co_allocation_callbacks_0* callbacks = reinterpret_cast<const co_allocation_callbacks_0*>(co_in_structure_0_find_0(info->next_structure, CO_STRUCTURE_TYPE_0_ALLOCATION_CALLBACKS_0));

	const std::size_t count = std::min(*factory_count, source->classes.size());
	for (std::size_t i = 0; i < count; ++i) {
		test_factory* factory = allocate_object<test_factory>(*callbacks);
		if (factory == nullptr) {
			for (std::size_t j = i; j != 0; --j) {
				factories[j - 1]->vtable->destroy(factories[j - 1]);
			}
			return CO_RESULT_0_ERROR_OUT_OF_MEMORY;
		}
		co_event_domain_group_factory_0_init(factory, &factory_vtable);
		factory->config = source->classes[i];
		factory->log = source->log;
		factory->allocation_callbacks = *callbacks;
		factory->allocation_callbacks.next_structure = nullptr;
		factories[i] = factory;
	}
	*factory_count = count;
	return CO_RESULT_0_SUCCESS;
}

// Creates an instance registering the classes of sources, in order, after the built-in ones.
co_result_0 create_instance(std::span<test_source* const> sources, co_instance_0* instance, const co_in_structure_0* next_structure = nullptr)
{
	std::vector<co_event_domain_group_factory_source_0> entries(sources.size());
	const co_in_structure_0* chain = next_structure;
	for (std::size_t i = sources.size(); i != 0; --i) {
		entries[i - 1] = {
			.structure_type = CO_STRUCTURE_TYPE_0_EVENT_DOMAIN_GROUP_FACTORY_SOURCE_0,
			.next_structure = chain,
			.entry_point_fn = &test_entry_point,
			.data = sources[i - 1],
		};
		chain = CO_IN_STRUCTURE_0_CAST(&entries[i - 1]);
	}
	const co_instance_0_create_info_0 create_info = {
		.structure_type = CO_STRUCTURE_TYPE_0_INSTANCE_0_CREATE_INFO_0,
		.next_structure = chain,
		.api_version = CO_API_VERSION_0,
	};
	return co_instance_0_create_0(&create_info, instance);
}

co_event_loop_0_create_info_0 event_loop_create_info(const co_in_structure_0* next_structure, std::span<const co_service_type_0> required_services = {})
{
	return co_event_loop_0_create_info_0{
		.structure_type = CO_STRUCTURE_TYPE_0_EVENT_LOOP_0_CREATE_INFO_0,
		.next_structure = next_structure,
		.required_services = required_services.data(),
		.required_service_count = required_services.size(),
	};
}

// Name of the class of a standalone event domain created by a test factory.
std::string class_of(co_event_domain_0 event_domain)
{
	return static_cast<test_member*>(CO_EVENT_LOOP_0_CAST(event_domain))->factory->config->name;
}

struct counting_allocator final
{
	int allocations = 0;
	int deallocations = 0;
};

void* counting_allocate(void* data, std::size_t size, std::size_t alignment) noexcept
{
	++static_cast<counting_allocator*>(data)->allocations;
	return ::operator new(size, std::align_val_t(alignment), std::nothrow);
}

void counting_deallocate(void* data, void* memory, std::size_t size, std::size_t alignment) noexcept
{
	++static_cast<counting_allocator*>(data)->deallocations;
	::operator delete(memory, size, std::align_val_t(alignment));
}

co_allocation_callbacks_0 counting_callbacks(counting_allocator* allocator, const co_in_structure_0* next_structure = nullptr) noexcept
{
	return co_allocation_callbacks_0{
		.structure_type = CO_STRUCTURE_TYPE_0_ALLOCATION_CALLBACKS_0,
		.next_structure = next_structure,
		.allocate_fn = &counting_allocate,
		.deallocate_fn = &counting_deallocate,
		.data = allocator,
	};
}

} // namespace

TEST(c_event_domain_group_factory_0, instance_owns_factories_and_destroys_them_in_reverse_order)
{
	test_log log;
	const class_config a = {.name = "a"};
	const class_config b = {.name = "b"};
	const class_config c = {.name = "c"};
	test_source first = {.classes = {&a, &b}, .log = &log};
	test_source second = {.classes = {&c}, .log = &log};
	test_source* const sources[] = {&first, &second};
	counting_allocator allocator;
	const co_allocation_callbacks_0 callbacks = counting_callbacks(&allocator);

	co_instance_0 instance = nullptr;
	ASSERT_EQ(create_instance(sources, &instance, CO_IN_STRUCTURE_0_CAST(&callbacks)), CO_RESULT_0_SUCCESS);
	auto instance_guard = co_test::guard(instance);
	EXPECT_EQ(log.entry_point_allocation_callbacks_count, 1u);
	EXPECT_EQ(log.entry_point_api_version, CO_API_VERSION_0);
	EXPECT_TRUE(log.events.empty());

	instance_guard.reset();
	EXPECT_EQ(log.events, (std::vector<std::string>{"destroy factory c", "destroy factory b", "destroy factory a"}));
	EXPECT_EQ(allocator.allocations, allocator.deallocations);
}

TEST(c_event_domain_group_factory_0, failing_entry_point_fails_instance_creation)
{
	test_log log;
	const class_config a = {.name = "a"};
	const class_config b = {.name = "b"};
	test_source first = {.classes = {&a}, .log = &log};
	test_source failing = {.classes = {&b}, .log = &log, .result = CO_RESULT_0_ERROR_SYSTEM};
	test_source* const sources[] = {&first, &failing};
	counting_allocator allocator;
	const co_allocation_callbacks_0 callbacks = counting_callbacks(&allocator);

	co_instance_0 instance = nullptr;
	EXPECT_EQ(create_instance(sources, &instance, CO_IN_STRUCTURE_0_CAST(&callbacks)), CO_RESULT_0_ERROR_SYSTEM);
	auto instance_guard = co_test::guard(instance);
	EXPECT_EQ(instance, nullptr);
	EXPECT_EQ(log.events, (std::vector<std::string>{"destroy factory a"}));
	EXPECT_EQ(allocator.allocations, allocator.deallocations);
}

TEST(c_event_domain_group_factory_0, inconsistent_factory_fails_instance_creation)
{
	test_log log;
	const class_config a = {.name = "a"};
	const class_config broken = {.name = "broken", .min_member_count = 2, .max_member_count = 1};
	test_source source = {.classes = {&a, &broken}, .log = &log};
	test_source* const sources[] = {&source};

	co_instance_0 instance = nullptr;
	EXPECT_EQ(create_instance(sources, &instance), CO_RESULT_0_ERROR_INVALID_ARGUMENT);
	auto instance_guard = co_test::guard(instance);
	EXPECT_EQ(instance, nullptr);
	EXPECT_EQ(log.events, (std::vector<std::string>{"destroy factory broken", "destroy factory a"}));
}

TEST(c_instance_0_create_event_domain, fails_without_candidate)
{
	test_log log;
	const class_config facility = {.name = "facility", .member_create_info_structure_types = {CO_STRUCTURE_TYPE_0_EVENT_FACILITY_0_CREATE_INFO_0}};
	test_source source = {.classes = {&facility}, .log = &log};
	test_source* const sources[] = {&source};
	co_instance_0 instance = nullptr;
	ASSERT_EQ(create_instance(sources, &instance), CO_RESULT_0_SUCCESS);
	auto instance_guard = co_test::guard(instance);

	const co_event_loop_0_create_info_0 create_info = event_loop_create_info(nullptr);
	co_event_domain_0 event_domain = nullptr;
	EXPECT_EQ(co_instance_0_create_event_domain_0(instance, CO_EVENT_DOMAIN_0_CREATE_INFO_0_CAST(&create_info), &event_domain), CO_RESULT_0_ERROR_NOT_SUPPORTED);
	auto event_domain_guard = co_test::guard(event_domain);
	EXPECT_EQ(event_domain, nullptr);
}

TEST(c_instance_0_create_event_domain, chooses_highest_rank_then_last_registered)
{
	test_log log;
	const class_config low = {.name = "low", .rank = 1};
	const class_config high = {.name = "high", .rank = 2};
	const class_config high_later = {.name = "high_later", .rank = 2};
	const class_config lowest_last = {.name = "lowest_last", .rank = 0};
	test_source first = {.classes = {&low, &high}, .log = &log};
	test_source second = {.classes = {&high_later, &lowest_last}, .log = &log};
	test_source* const sources[] = {&first, &second};
	co_instance_0 instance = nullptr;
	ASSERT_EQ(create_instance(sources, &instance), CO_RESULT_0_SUCCESS);
	auto instance_guard = co_test::guard(instance);

	const co_event_loop_0_create_info_0 create_info = event_loop_create_info(nullptr);
	co_event_domain_0 event_domain = nullptr;
	ASSERT_EQ(co_instance_0_create_event_domain_0(instance, CO_EVENT_DOMAIN_0_CREATE_INFO_0_CAST(&create_info), &event_domain), CO_RESULT_0_SUCCESS);
	auto event_domain_guard = co_test::guard(event_domain);
	EXPECT_EQ(class_of(event_domain), "high_later");
}

TEST(c_instance_0_create_event_domain, tries_next_candidate_when_class_declines)
{
	test_log log;
	const class_config low = {.name = "low", .rank = 1};
	const class_config high = {.name = "high", .rank = 2};
	const class_config high_later = {.name = "high_later", .rank = 2, .create_result = CO_RESULT_0_ERROR_NOT_SUPPORTED};
	const class_config highest = {.name = "highest", .rank = 3, .create_result = CO_RESULT_0_ERROR_NOT_SUPPORTED};
	test_source source = {.classes = {&low, &high, &highest, &high_later}, .log = &log};
	test_source* const sources[] = {&source};
	co_instance_0 instance = nullptr;
	ASSERT_EQ(create_instance(sources, &instance), CO_RESULT_0_SUCCESS);
	auto instance_guard = co_test::guard(instance);

	const co_event_loop_0_create_info_0 create_info = event_loop_create_info(nullptr);
	co_event_domain_0 event_domain = nullptr;
	ASSERT_EQ(co_instance_0_create_event_domain_0(instance, CO_EVENT_DOMAIN_0_CREATE_INFO_0_CAST(&create_info), &event_domain), CO_RESULT_0_SUCCESS);
	auto event_domain_guard = co_test::guard(event_domain);
	EXPECT_EQ(class_of(event_domain), "high");
	EXPECT_EQ(log.events, (std::vector<std::string>{"decline highest", "decline high_later", "create group high"}));
}

TEST(c_instance_0_create_event_domain, fails_when_every_candidate_declines)
{
	test_log log;
	const class_config first = {.name = "first", .create_result = CO_RESULT_0_ERROR_NOT_SUPPORTED};
	const class_config second = {.name = "second", .create_result = CO_RESULT_0_ERROR_NOT_SUPPORTED};
	test_source source = {.classes = {&first, &second}, .log = &log};
	test_source* const sources[] = {&source};
	co_instance_0 instance = nullptr;
	ASSERT_EQ(create_instance(sources, &instance), CO_RESULT_0_SUCCESS);
	auto instance_guard = co_test::guard(instance);

	const co_event_loop_0_create_info_0 create_info = event_loop_create_info(nullptr);
	co_event_domain_0 event_domain = nullptr;
	EXPECT_EQ(co_instance_0_create_event_domain_0(instance, CO_EVENT_DOMAIN_0_CREATE_INFO_0_CAST(&create_info), &event_domain), CO_RESULT_0_ERROR_NOT_SUPPORTED);
	auto event_domain_guard = co_test::guard(event_domain);
	EXPECT_EQ(event_domain, nullptr);
	EXPECT_EQ(log.events, (std::vector<std::string>{"decline second", "decline first"}));
}

TEST(c_instance_0_create_event_domain, other_failure_ends_selection)
{
	test_log log;
	const class_config fallback = {.name = "fallback"};
	const class_config failing = {.name = "failing", .create_result = CO_RESULT_0_ERROR_OUT_OF_MEMORY};
	test_source source = {.classes = {&fallback, &failing}, .log = &log};
	test_source* const sources[] = {&source};
	co_instance_0 instance = nullptr;
	ASSERT_EQ(create_instance(sources, &instance), CO_RESULT_0_SUCCESS);
	auto instance_guard = co_test::guard(instance);

	const co_event_loop_0_create_info_0 create_info = event_loop_create_info(nullptr);
	co_event_domain_0 event_domain = nullptr;
	EXPECT_EQ(co_instance_0_create_event_domain_0(instance, CO_EVENT_DOMAIN_0_CREATE_INFO_0_CAST(&create_info), &event_domain), CO_RESULT_0_ERROR_OUT_OF_MEMORY);
	auto event_domain_guard = co_test::guard(event_domain);
	EXPECT_EQ(event_domain, nullptr);
	EXPECT_EQ(log.events, (std::vector<std::string>{"decline failing"}));
}

TEST(c_instance_0_create_event_domain, requires_every_chain_entry_to_be_handled)
{
	test_log log;
	const class_config generic = {.name = "generic", .rank = 5};
	const class_config tagged = {.name = "tagged", .handled_structure_types = {structure_type(tag_a_tag)}};
	test_source source = {.classes = {&generic, &tagged}, .log = &log};
	test_source* const sources[] = {&source};
	co_instance_0 instance = nullptr;
	ASSERT_EQ(create_instance(sources, &instance), CO_RESULT_0_SUCCESS);
	auto instance_guard = co_test::guard(instance);

	const co_in_structure_0 tag_a = {.structure_type = structure_type(tag_a_tag), .next_structure = nullptr};
	const co_event_loop_0_create_info_0 tagged_info = event_loop_create_info(&tag_a);
	co_event_domain_0 event_domain = nullptr;
	ASSERT_EQ(co_instance_0_create_event_domain_0(instance, CO_EVENT_DOMAIN_0_CREATE_INFO_0_CAST(&tagged_info), &event_domain), CO_RESULT_0_SUCCESS);
	auto event_domain_guard = co_test::guard(event_domain);
	EXPECT_EQ(class_of(event_domain), "tagged");

	const co_in_structure_0 tag_b = {.structure_type = structure_type(tag_b_tag), .next_structure = nullptr};
	const co_event_loop_0_create_info_0 unhandled_info = event_loop_create_info(&tag_b);
	co_event_domain_0 unhandled = nullptr;
	EXPECT_EQ(co_instance_0_create_event_domain_0(instance, CO_EVENT_DOMAIN_0_CREATE_INFO_0_CAST(&unhandled_info), &unhandled), CO_RESULT_0_ERROR_NOT_SUPPORTED);
	auto unhandled_guard = co_test::guard(unhandled);
}

TEST(c_instance_0_create_event_domain, ignores_order_of_chain_entries)
{
	test_log log;
	const class_config only_a = {.name = "only_a", .handled_structure_types = {structure_type(tag_a_tag)}, .rank = 5};
	const class_config both = {.name = "both", .handled_structure_types = {structure_type(tag_b_tag), structure_type(tag_a_tag)}};
	test_source source = {.classes = {&only_a, &both}, .log = &log};
	test_source* const sources[] = {&source};
	co_instance_0 instance = nullptr;
	ASSERT_EQ(create_instance(sources, &instance), CO_RESULT_0_SUCCESS);
	auto instance_guard = co_test::guard(instance);

	const co_in_structure_0 a_last = {.structure_type = structure_type(tag_a_tag), .next_structure = nullptr};
	const co_in_structure_0 b_first = {.structure_type = structure_type(tag_b_tag), .next_structure = &a_last};
	const co_event_loop_0_create_info_0 b_then_a = event_loop_create_info(&b_first);
	co_event_domain_0 first = nullptr;
	ASSERT_EQ(co_instance_0_create_event_domain_0(instance, CO_EVENT_DOMAIN_0_CREATE_INFO_0_CAST(&b_then_a), &first), CO_RESULT_0_SUCCESS);
	auto first_guard = co_test::guard(first);
	EXPECT_EQ(class_of(first), "both");

	const co_in_structure_0 b_last = {.structure_type = structure_type(tag_b_tag), .next_structure = nullptr};
	const co_in_structure_0 a_first = {.structure_type = structure_type(tag_a_tag), .next_structure = &b_last};
	const co_event_loop_0_create_info_0 a_then_b = event_loop_create_info(&a_first);
	co_event_domain_0 second = nullptr;
	ASSERT_EQ(co_instance_0_create_event_domain_0(instance, CO_EVENT_DOMAIN_0_CREATE_INFO_0_CAST(&a_then_b), &second), CO_RESULT_0_SUCCESS);
	auto second_guard = co_test::guard(second);
	EXPECT_EQ(class_of(second), "both");
}

TEST(c_instance_0_create_event_domain, ignores_hints_in_matching_and_passes_them_to_the_class)
{
	test_log log;
	const class_config generic = {.name = "generic", .rank = 5};
	const class_config tagged = {.name = "tagged", .handled_structure_types = {structure_type(tag_a_tag)}};
	test_source source = {.classes = {&generic, &tagged}, .log = &log};
	test_source* const sources[] = {&source};
	co_instance_0 instance = nullptr;
	ASSERT_EQ(create_instance(sources, &instance), CO_RESULT_0_SUCCESS);
	auto instance_guard = co_test::guard(instance);

	const co_in_structure_0 tag_a = {.structure_type = structure_type(tag_a_tag), .next_structure = nullptr};
	const co_hint_info_0 hint = {.structure_type = CO_STRUCTURE_TYPE_0_HINT_INFO_0, .next_structure = nullptr, .hint = &tag_a};
	const co_event_loop_0_create_info_0 create_info = event_loop_create_info(CO_IN_STRUCTURE_0_CAST(&hint));
	co_event_domain_0 event_domain = nullptr;
	ASSERT_EQ(co_instance_0_create_event_domain_0(instance, CO_EVENT_DOMAIN_0_CREATE_INFO_0_CAST(&create_info), &event_domain), CO_RESULT_0_SUCCESS);
	auto event_domain_guard = co_test::guard(event_domain);
	EXPECT_EQ(class_of(event_domain), "generic");
	EXPECT_EQ(log.member_hint, &tag_a);

	const co_in_structure_0 tag_b = {.structure_type = structure_type(tag_b_tag), .next_structure = nullptr};
	const co_hint_info_0 unknown_hint = {.structure_type = CO_STRUCTURE_TYPE_0_HINT_INFO_0, .next_structure = nullptr, .hint = &tag_b};
	const co_in_structure_0 required_a = {.structure_type = structure_type(tag_a_tag), .next_structure = CO_IN_STRUCTURE_0_CAST(&unknown_hint)};
	const co_event_loop_0_create_info_0 required_info = event_loop_create_info(&required_a);
	co_event_domain_0 required = nullptr;
	ASSERT_EQ(co_instance_0_create_event_domain_0(instance, CO_EVENT_DOMAIN_0_CREATE_INFO_0_CAST(&required_info), &required), CO_RESULT_0_SUCCESS);
	auto required_guard = co_test::guard(required);
	EXPECT_EQ(class_of(required), "tagged");
}

TEST(c_instance_0_create_event_domain, requires_every_required_service)
{
	test_log log;
	const class_config plain = {.name = "plain", .rank = 5};
	const class_config with_timer = {.name = "with_timer", .provided_service_types = {service_type(timer_service_tag)}};
	test_source source = {.classes = {&plain, &with_timer}, .log = &log};
	test_source* const sources[] = {&source};
	co_instance_0 instance = nullptr;
	ASSERT_EQ(create_instance(sources, &instance), CO_RESULT_0_SUCCESS);
	auto instance_guard = co_test::guard(instance);

	const co_service_type_0 timer[] = {service_type(timer_service_tag)};
	const co_event_loop_0_create_info_0 timer_info = event_loop_create_info(nullptr, timer);
	co_event_domain_0 event_domain = nullptr;
	ASSERT_EQ(co_instance_0_create_event_domain_0(instance, CO_EVENT_DOMAIN_0_CREATE_INFO_0_CAST(&timer_info), &event_domain), CO_RESULT_0_SUCCESS);
	auto event_domain_guard = co_test::guard(event_domain);
	EXPECT_EQ(class_of(event_domain), "with_timer");
	co_service_0 service = nullptr;
	EXPECT_EQ(co_event_domain_0_get_service_0(event_domain, service_type(timer_service_tag), &service), CO_RESULT_0_SUCCESS);
	EXPECT_NE(service, nullptr);

	const co_service_type_0 socket[] = {service_type(socket_service_tag)};
	const co_event_loop_0_create_info_0 socket_info = event_loop_create_info(nullptr, socket);
	co_event_domain_0 unsupported = nullptr;
	EXPECT_EQ(co_instance_0_create_event_domain_0(instance, CO_EVENT_DOMAIN_0_CREATE_INFO_0_CAST(&socket_info), &unsupported), CO_RESULT_0_ERROR_NOT_SUPPORTED);
	auto unsupported_guard = co_test::guard(unsupported);
}

TEST(c_instance_0_create_event_domain, verifies_services_after_creation)
{
	test_log log;
	const class_config liar = {.name = "liar", .provided_service_types = {service_type(timer_service_tag)}, .members_lack_services = true};
	test_source source = {.classes = {&liar}, .log = &log};
	test_source* const sources[] = {&source};
	co_instance_0 instance = nullptr;
	ASSERT_EQ(create_instance(sources, &instance), CO_RESULT_0_SUCCESS);
	auto instance_guard = co_test::guard(instance);

	const co_service_type_0 timer[] = {service_type(timer_service_tag)};
	const co_event_loop_0_create_info_0 create_info = event_loop_create_info(nullptr, timer);
	co_event_domain_0 event_domain = nullptr;
	EXPECT_EQ(co_instance_0_create_event_domain_0(instance, CO_EVENT_DOMAIN_0_CREATE_INFO_0_CAST(&create_info), &event_domain), CO_RESULT_0_ERROR_SERVICE_NOT_FOUND);
	auto event_domain_guard = co_test::guard(event_domain);
	EXPECT_EQ(event_domain, nullptr);
	EXPECT_EQ(log.events, (std::vector<std::string>{"create group liar", "destroy group liar"}));
}

TEST(c_instance_0_create_event_domain, hands_exactly_one_allocation_callbacks_to_the_class)
{
	test_log log;
	const class_config a = {.name = "a"};
	test_source source = {.classes = {&a}, .log = &log};
	test_source* const sources[] = {&source};
	counting_allocator instance_allocator;
	const co_allocation_callbacks_0 instance_callbacks = counting_callbacks(&instance_allocator);
	co_instance_0 instance = nullptr;
	ASSERT_EQ(create_instance(sources, &instance, CO_IN_STRUCTURE_0_CAST(&instance_callbacks)), CO_RESULT_0_SUCCESS);
	auto instance_guard = co_test::guard(instance);

	const co_event_loop_0_create_info_0 default_info = event_loop_create_info(nullptr);
	co_event_domain_0 event_domain = nullptr;
	ASSERT_EQ(co_instance_0_create_event_domain_0(instance, CO_EVENT_DOMAIN_0_CREATE_INFO_0_CAST(&default_info), &event_domain), CO_RESULT_0_SUCCESS);
	auto event_domain_guard = co_test::guard(event_domain);
	EXPECT_EQ(log.group_allocation_callbacks_count, 1u);
	EXPECT_EQ(log.group_allocation_data, &instance_allocator);
	EXPECT_EQ(default_info.next_structure, nullptr);
	event_domain_guard.reset();

	counting_allocator object_allocator;
	const co_allocation_callbacks_0 object_callbacks = counting_callbacks(&object_allocator);
	const co_event_loop_0_create_info_0 object_info = event_loop_create_info(CO_IN_STRUCTURE_0_CAST(&object_callbacks));
	ASSERT_EQ(co_instance_0_create_event_domain_0(instance, CO_EVENT_DOMAIN_0_CREATE_INFO_0_CAST(&object_info), &event_domain), CO_RESULT_0_SUCCESS);
	event_domain_guard.reset(event_domain);
	EXPECT_EQ(log.group_allocation_callbacks_count, 1u);
	EXPECT_EQ(log.group_allocation_data, &object_allocator);
	EXPECT_EQ(object_allocator.allocations, 1);
	event_domain_guard.reset();
	EXPECT_EQ(object_allocator.deallocations, 1);
}

TEST(c_instance_0_create_event_domain, standalone_event_loop_dispatches_and_destroys_its_group)
{
	test_log log;
	const class_config a = {.name = "a", .max_member_count = 4};
	test_source source = {.classes = {&a}, .log = &log};
	test_source* const sources[] = {&source};
	co_instance_0 instance = nullptr;
	ASSERT_EQ(create_instance(sources, &instance), CO_RESULT_0_SUCCESS);
	auto instance_guard = co_test::guard(instance);

	const co_event_loop_0_create_info_0 create_info = event_loop_create_info(nullptr);
	co_event_domain_0 event_domain = nullptr;
	ASSERT_EQ(co_instance_0_create_event_domain_0(instance, CO_EVENT_DOMAIN_0_CREATE_INFO_0_CAST(&create_info), &event_domain), CO_RESULT_0_SUCCESS);
	auto event_domain_guard = co_test::guard(event_domain);
	co_event_loop_0 event_loop = CO_EVENT_LOOP_0_CAST(event_domain);
	EXPECT_EQ(CO_EVENT_DOMAIN_0_CAST(event_loop), event_domain);
	EXPECT_EQ(CO_EVENT_LOOP_0_CAST(nullptr), nullptr);
	EXPECT_EQ(co_event_loop_0_run_0(event_loop), CO_RESULT_0_SUCCESS);
	EXPECT_EQ(co_event_loop_0_stop_0(event_loop), CO_RESULT_0_SUCCESS);
	EXPECT_EQ(static_cast<test_member*>(event_loop)->runs, 1);
	EXPECT_EQ(static_cast<test_member*>(event_loop)->stops, 1);

	event_domain_guard.reset();
	EXPECT_EQ(log.events, (std::vector<std::string>{"create group a", "destroy group a"}));
}

TEST(cpp_instance_0_create_event_domain, creates_event_loop)
{
	test_log log;
	const class_config a = {.name = "a"};
	test_source source = {.classes = {&a}, .log = &log};
	test_source* const sources[] = {&source};
	co_instance_0 c_instance = nullptr;
	ASSERT_EQ(create_instance(sources, &c_instance), CO_RESULT_0_SUCCESS);
	auto instance_guard = co_test::guard(c_instance);
	co::instance_0 instance(c_instance);

	const co::event_loop_0_create_info_0 create_info = {
		.structure_type = co::structure_type_0_event_loop_0_create_info_0,
		.next_structure = nullptr,
		.required_services = nullptr,
		.required_service_count = 0,
	};
	co::event_loop_0 event_loop;
	ASSERT_EQ(instance.create_event_domain_0(co::event_domain_0_create_info_0_cast(create_info), co::out_handle(event_loop)), co::result_0::success);
	auto event_loop_guard = co_test::guard(event_loop);
	ASSERT_NE(event_loop, nullptr);
	EXPECT_EQ(event_loop.run_0(), co::result_0::success);

	co::event_domain_0 event_domain = event_loop;
	EXPECT_EQ(co::event_loop_0_cast(static_cast<co_event_domain_0>(event_domain)), event_loop);
	EXPECT_EQ(co::event_domain_0_cast(event_loop), event_domain);
	EXPECT_EQ(co::event_domain_0_cast(static_cast<co_event_loop_0>(event_loop)), event_domain);
	EXPECT_EQ(co::handle_cast<co_event_loop_0>(event_domain), static_cast<co_event_loop_0>(event_loop));
	EXPECT_EQ(co::event_loop_0_cast(nullptr), nullptr);
	static_assert(!co::handle_castable_to<co::event_loop_0, co::event_facility_0>);
	static_assert(!co::handle_castable_to<co::event_loop_0, co::executor_0>);
	static_assert(!co::handle_castable_to<co::event_loop_0, co_service_0>);

	// out_handle also adapts a C output parameter, and to a C handle target.
	co_event_loop_0_create_info_0 c_create_info = {
		.structure_type = CO_STRUCTURE_TYPE_0_EVENT_LOOP_0_CREATE_INFO_0,
		.next_structure = nullptr,
		.required_services = nullptr,
		.required_service_count = 0,
	};
	co_event_loop_0 c_event_loop = nullptr;
	ASSERT_EQ(co_instance_0_create_event_domain_0(c_instance, CO_EVENT_DOMAIN_0_CREATE_INFO_0_CAST(&c_create_info), co::out_handle(c_event_loop)), CO_RESULT_0_SUCCESS);
	auto c_event_loop_guard = co_test::guard(c_event_loop);
	EXPECT_NE(c_event_loop, nullptr);

	// The target is left unchanged when the function fails before writing its output.
	co::event_loop_0 unchanged = event_loop;
	const co::event_loop_0_create_info_0 unhandled_info = {
		.structure_type = co::structure_type_0_event_loop_0_create_info_0,
		.next_structure = co::in_structure_0_cast(&create_info),
		.required_services = nullptr,
		.required_service_count = 0,
	};
	EXPECT_NE(instance.create_event_domain_0(co::event_domain_0_create_info_0_cast(unhandled_info), co::out_handle(unchanged)), co::result_0::success);
	EXPECT_EQ(unchanged, event_loop);
}
