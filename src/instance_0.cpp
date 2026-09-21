#include "instance_0.hpp"

#include <co.hpp>
#include <co/impl.hpp>

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <limits>
#include <memory>
#include <new>
#include <span>
#include <type_traits>

// Rule: Make sure functions in this file are defined in the same order as in co.h file.

namespace co
{

const structure_type_0 structure_type_0_instance_0_create_info_0 = []() { return static_cast<structure_type_0>(reinterpret_cast<uintptr_t>(&structure_type_0_instance_0_create_info_0)); }();
const structure_type_0 structure_type_0_hint_info_0 = []() { return static_cast<structure_type_0>(reinterpret_cast<uintptr_t>(&structure_type_0_hint_info_0)); }();
const structure_type_0 structure_type_0_allocation_callbacks_0 = []() { return static_cast<structure_type_0>(reinterpret_cast<uintptr_t>(&structure_type_0_allocation_callbacks_0)); }();
const structure_type_0 structure_type_0_debug_callback_0 = []() { return static_cast<structure_type_0>(reinterpret_cast<uintptr_t>(&structure_type_0_debug_callback_0)); }();
const structure_type_0 structure_type_0_event_domain_group_factory_source_0 = []() { return static_cast<structure_type_0>(reinterpret_cast<uintptr_t>(&structure_type_0_event_domain_group_factory_source_0)); }();

} // namespace co

const co_structure_type_0 CO_STRUCTURE_TYPE_0_INSTANCE_0_CREATE_INFO_0 = []() { return static_cast<co_structure_type_0>(reinterpret_cast<uintptr_t>(&co::structure_type_0_instance_0_create_info_0)); }();
const co_structure_type_0 CO_STRUCTURE_TYPE_0_HINT_INFO_0 = []() { return static_cast<co_structure_type_0>(reinterpret_cast<uintptr_t>(&co::structure_type_0_hint_info_0)); }();
const co_structure_type_0 CO_STRUCTURE_TYPE_0_ALLOCATION_CALLBACKS_0 = []() { return static_cast<co_structure_type_0>(reinterpret_cast<uintptr_t>(&co::structure_type_0_allocation_callbacks_0)); }();
const co_structure_type_0 CO_STRUCTURE_TYPE_0_DEBUG_CALLBACK_0 = []() { return static_cast<co_structure_type_0>(reinterpret_cast<uintptr_t>(&co::structure_type_0_debug_callback_0)); }();
const co_structure_type_0 CO_STRUCTURE_TYPE_0_EVENT_DOMAIN_GROUP_FACTORY_SOURCE_0 = []() { return static_cast<co_structure_type_0>(reinterpret_cast<uintptr_t>(&co::structure_type_0_event_domain_group_factory_source_0)); }();
const co_structure_type_0 CO_STRUCTURE_TYPE_0_EVENT_DOMAIN_GROUP_FACTORY_0_ENTRY_POINT_INFO_0 = []() { return static_cast<co_structure_type_0>(reinterpret_cast<uintptr_t>(&CO_STRUCTURE_TYPE_0_EVENT_DOMAIN_GROUP_FACTORY_0_ENTRY_POINT_INFO_0)); }();

static_assert(std::is_standard_layout_v<co_instance_0_create_info_0>);
static_assert(offsetof(co_instance_0_create_info_0, structure_type) == offsetof(co_in_structure_0, structure_type));
static_assert(offsetof(co_instance_0_create_info_0, next_structure) == offsetof(co_in_structure_0, next_structure));
static_assert(std::is_standard_layout_v<co::instance_0_create_info_0>);
static_assert(offsetof(co::instance_0_create_info_0, structure_type) == offsetof(co::in_structure_0, structure_type));
static_assert(offsetof(co::instance_0_create_info_0, next_structure) == offsetof(co::in_structure_0, next_structure));
static_assert(sizeof(co_instance_0_create_info_0) == sizeof(co::instance_0_create_info_0));
static_assert(offsetof(co_instance_0_create_info_0, structure_type) == offsetof(co::instance_0_create_info_0, structure_type));
static_assert(offsetof(co_instance_0_create_info_0, next_structure) == offsetof(co::instance_0_create_info_0, next_structure));
static_assert(std::is_standard_layout_v<co_hint_info_0>);
static_assert(offsetof(co_hint_info_0, structure_type) == offsetof(co_in_structure_0, structure_type));
static_assert(offsetof(co_hint_info_0, next_structure) == offsetof(co_in_structure_0, next_structure));
static_assert(sizeof(co_hint_info_0) == sizeof(co::hint_info_0));
static_assert(offsetof(co_hint_info_0, hint) == offsetof(co::hint_info_0, hint));
static_assert(offsetof(co_instance_0_create_info_0, api_version) == offsetof(co::instance_0_create_info_0, api_version));

static_assert(std::is_standard_layout_v<co::allocation_callbacks_0>);
static_assert(sizeof(co_allocation_callbacks_0) == sizeof(co::allocation_callbacks_0));
static_assert(offsetof(co_allocation_callbacks_0, structure_type) == offsetof(co::allocation_callbacks_0, structure_type));
static_assert(offsetof(co_allocation_callbacks_0, next_structure) == offsetof(co::allocation_callbacks_0, next_structure));
static_assert(offsetof(co_allocation_callbacks_0, allocate_fn) == offsetof(co::allocation_callbacks_0, allocate_fn));
static_assert(offsetof(co_allocation_callbacks_0, deallocate_fn) == offsetof(co::allocation_callbacks_0, deallocate_fn));
static_assert(offsetof(co_allocation_callbacks_0, data) == offsetof(co::allocation_callbacks_0, data));

static_assert(std::is_standard_layout_v<co::debug_message_0>);
static_assert(sizeof(co_debug_message_0) == sizeof(co::debug_message_0));
static_assert(offsetof(co_debug_message_0, severity) == offsetof(co::debug_message_0, severity));
static_assert(offsetof(co_debug_message_0, result) == offsetof(co::debug_message_0, result));
static_assert(offsetof(co_debug_message_0, message) == offsetof(co::debug_message_0, message));
static_assert(offsetof(co_debug_message_0, object) == offsetof(co::debug_message_0, object));
static_assert(offsetof(co_debug_message_0, system_error) == offsetof(co::debug_message_0, system_error));

static_assert(std::is_standard_layout_v<co::debug_callback_0>);
static_assert(sizeof(co_debug_callback_0) == sizeof(co::debug_callback_0));
static_assert(offsetof(co_debug_callback_0, structure_type) == offsetof(co::debug_callback_0, structure_type));
static_assert(offsetof(co_debug_callback_0, next_structure) == offsetof(co::debug_callback_0, next_structure));
static_assert(offsetof(co_debug_callback_0, min_severity) == offsetof(co::debug_callback_0, min_severity));
static_assert(offsetof(co_debug_callback_0, callback_fn) == offsetof(co::debug_callback_0, callback_fn));
static_assert(offsetof(co_debug_callback_0, data) == offsetof(co::debug_callback_0, data));

static_assert(std::is_standard_layout_v<co::event_domain_group_factory_source_0>);
static_assert(sizeof(co_event_domain_group_factory_source_0) == sizeof(co::event_domain_group_factory_source_0));
static_assert(offsetof(co_event_domain_group_factory_source_0, structure_type) == offsetof(co::event_domain_group_factory_source_0, structure_type));
static_assert(offsetof(co_event_domain_group_factory_source_0, next_structure) == offsetof(co::event_domain_group_factory_source_0, next_structure));
static_assert(offsetof(co_event_domain_group_factory_source_0, entry_point_fn) == offsetof(co::event_domain_group_factory_source_0, entry_point_fn));
static_assert(offsetof(co_event_domain_group_factory_source_0, data) == offsetof(co::event_domain_group_factory_source_0, data));

namespace co::detail
{

namespace
{

void* default_allocate(void* /* data */, std::size_t size, std::size_t alignment) noexcept
{
	return ::operator new(size, std::align_val_t(alignment), std::nothrow);
}

void default_deallocate(void* /* data */, void* memory, std::size_t size, std::size_t alignment) noexcept
{
	::operator delete(memory, size, std::align_val_t(alignment));
}

bool align_up(std::size_t value, std::size_t alignment, std::size_t* result) noexcept
{
	if (value > std::numeric_limits<std::size_t>::max() - (alignment - 1)) {
		return false;
	}
	*result = (value + alignment - 1) & ~(alignment - 1);
	return true;
}

} // namespace

const co_allocation_callbacks_0 default_allocation_callbacks_0 = {
	.structure_type = CO_STRUCTURE_TYPE_0_ALLOCATION_CALLBACKS_0,
	.next_structure = nullptr,
	.allocate_fn = &default_allocate,
	.deallocate_fn = &default_deallocate,
	.data = nullptr,
};

bool is_compatible_api_version(co_version_0 api_version) noexcept
{
	constexpr co_version_0 library_version = CO_API_VERSION_0;
	if (CO_VERSION_0_EPOCH(api_version) != CO_VERSION_0_EPOCH(library_version) || api_version > library_version) {
		return false;
	}
	const uint16_t library_components[] = {CO_VERSION_0_MAJOR(library_version), CO_VERSION_0_MINOR(library_version), CO_VERSION_0_PATCH(library_version)};
	const uint16_t components[] = {CO_VERSION_0_MAJOR(api_version), CO_VERSION_0_MINOR(api_version), CO_VERSION_0_PATCH(api_version)};
	for (std::size_t i = 0; i < std::size(components); ++i) {
		if (components[i] != library_components[i]) {
			return false;
		}
		if (library_components[i] != 0) {
			break;
		}
	}
	return true;
}

void report(const debug_callbacks& callbacks, co_debug_severity_0 severity, co_result_0 result, const char* message, const void* object, std::int64_t system_error) noexcept
{
	const co_debug_message_0 debug_message = {
		.severity = severity,
		.result = result,
		.message = message,
		.object = object,
		.system_error = system_error,
	};
	for (const co_debug_callback_0& callback : std::span(callbacks.callbacks, callbacks.count)) {
		if (severity >= callback.min_severity) {
			callback.callback_fn(callback.data, &debug_message);
		}
	}
}

void report(co_instance_0 instance, co_debug_severity_0 severity, co_result_0 result, const char* message, const void* object, std::int64_t system_error) noexcept
{
	report(instance->debug_callbacks, severity, result, message, object, system_error);
}

co_result_0 find_allocation_callbacks(co_instance_0 instance, const co_in_structure_0* next_structure, const co_allocation_callbacks_0** allocation_callbacks) noexcept
{
	const co_allocation_callbacks_0* found = nullptr;
	for (const co_in_structure_0* entry = next_structure; entry != nullptr; entry = entry->next_structure) {
		if (entry->structure_type != CO_STRUCTURE_TYPE_0_ALLOCATION_CALLBACKS_0) {
			continue;
		}
		if (found != nullptr) {
			report(instance, CO_DEBUG_SEVERITY_0_ERROR, CO_RESULT_0_ERROR_INVALID_ARGUMENT, "the next chain holds several allocation callbacks", nullptr);
			return CO_RESULT_0_ERROR_INVALID_ARGUMENT;
		}
		found = reinterpret_cast<const co_allocation_callbacks_0*>(entry);
		if (found->allocate_fn == nullptr || found->deallocate_fn == nullptr) {
			report(instance, CO_DEBUG_SEVERITY_0_ERROR, CO_RESULT_0_ERROR_INVALID_ARGUMENT, "allocation callbacks without allocate_fn or deallocate_fn", nullptr);
			return CO_RESULT_0_ERROR_INVALID_ARGUMENT;
		}
	}
	*allocation_callbacks = found != nullptr ? found : &instance->allocation_callbacks;
	return CO_RESULT_0_SUCCESS;
}

co_result_0 check_next_chain(co_instance_0 instance, const co_in_structure_0* next_structure) noexcept
{
	for (const co_in_structure_0* entry = next_structure; entry != nullptr; entry = entry->next_structure) {
		if (entry->structure_type != CO_STRUCTURE_TYPE_0_ALLOCATION_CALLBACKS_0 && entry->structure_type != CO_STRUCTURE_TYPE_0_HINT_INFO_0) {
			report(instance, CO_DEBUG_SEVERITY_0_ERROR, CO_RESULT_0_ERROR_NOT_SUPPORTED, "unsupported entry in the next chain of the create info", nullptr);
			return CO_RESULT_0_ERROR_NOT_SUPPORTED;
		}
	}
	return CO_RESULT_0_SUCCESS;
}

} // namespace co::detail

namespace
{

using namespace co::detail;

// Sequential layout of the parts of a block.
struct block_layout final
{
	std::size_t size = 0;
	std::size_t alignment = 1;
	bool overflow = false;

	// Returns the offset of count elements of element_size bytes aligned to element_alignment.
	std::size_t reserve(std::size_t element_size, std::size_t element_alignment, std::size_t count) noexcept
	{
		std::size_t offset = 0;
		if (overflow || !align_up(size, element_alignment, &offset) || (count != 0 && element_size > (std::numeric_limits<std::size_t>::max() - offset) / count)) {
			overflow = true;
			return 0;
		}
		size = offset + element_size * count;
		alignment = std::max(alignment, element_alignment);
		return offset;
	}
};

// Reports a message of instance creation to the debug callbacks of the next chain.
void report_chain(const co_in_structure_0* next_structure, co_debug_severity_0 severity, co_result_0 result, const char* message) noexcept
{
	for (const co_in_structure_0* entry = next_structure; entry != nullptr; entry = entry->next_structure) {
		if (entry->structure_type != CO_STRUCTURE_TYPE_0_DEBUG_CALLBACK_0) {
			continue;
		}
		const co_debug_callback_0* callback = reinterpret_cast<const co_debug_callback_0*>(entry);
		if (callback->callback_fn != nullptr) {
			report(debug_callbacks{.callbacks = callback, .count = 1}, severity, result, message, nullptr);
		}
	}
}

// Calls fn(data, entry_point_fn) for the built-in entry point, then for each source in chain order.
template <class Fn>
co_result_0 for_each_source(const co_in_structure_0* chain, Fn&& fn) noexcept
{
	if (co_result_0 result = fn(nullptr, &builtin_event_domain_group_factories_0); result != CO_RESULT_0_SUCCESS) {
		return result;
	}
	for (const co_in_structure_0* entry = chain; entry != nullptr; entry = entry->next_structure) {
		if (entry->structure_type != CO_STRUCTURE_TYPE_0_EVENT_DOMAIN_GROUP_FACTORY_SOURCE_0) {
			continue;
		}
		const co_event_domain_group_factory_source_0* source = reinterpret_cast<const co_event_domain_group_factory_source_0*>(entry);
		if (co_result_0 result = fn(source->data, source->entry_point_fn); result != CO_RESULT_0_SUCCESS) {
			return result;
		}
	}
	return CO_RESULT_0_SUCCESS;
}

void destroy_factories(co_event_domain_group_factory_0* factories, std::size_t count) noexcept
{
	while (count != 0) {
		--count;
		factories[count]->vtable->destroy(factories[count]);
	}
}

// Validates a factory and stores the answers of its getters.
co_result_0 describe_factory(co_instance_0 instance, co_event_domain_group_factory_0 factory, registered_class* registered) noexcept
{
	const co_event_domain_group_factory_0_vtable* vtable = factory->vtable;
	if (vtable != nullptr && !is_compatible_api_version(vtable->api_version)) {
		report(instance, CO_DEBUG_SEVERITY_0_ERROR, CO_RESULT_0_ERROR_INCOMPATIBLE_VERSION, "the api_version of a factory is not compatible with the library", factory);
		return CO_RESULT_0_ERROR_INCOMPATIBLE_VERSION;
	}
	if (vtable == nullptr
		|| vtable->get_member_count_range_0 == nullptr || vtable->get_rank_0 == nullptr || vtable->create_event_domain_group_0 == nullptr || vtable->destroy == nullptr) {
		report(instance, CO_DEBUG_SEVERITY_0_ERROR, CO_RESULT_0_ERROR_INVALID_ARGUMENT, "a factory lacks a required method", factory);
		return CO_RESULT_0_ERROR_INVALID_ARGUMENT;
	}

	registered_class described = {.factory = factory};
	vtable->get_member_create_info_structure_types_0(factory, &described.member_create_info_structure_types, &described.member_create_info_structure_type_count);
	vtable->get_handled_structure_types_0(factory, &described.handled_structure_types, &described.handled_structure_type_count);
	vtable->get_provided_service_types_0(factory, &described.provided_service_types, &described.provided_service_type_count);
	vtable->get_member_count_range_0(factory, &described.min_member_count, &described.max_member_count);
	vtable->get_rank_0(factory, &described.rank);

	const auto valid_types = [](const co_structure_type_0* types, std::size_t count) noexcept {
		return count == 0 || (types != nullptr && std::find(types, types + count, co_structure_type_0{}) == types + count);
	};
	if (described.member_create_info_structure_type_count == 0 || !valid_types(described.member_create_info_structure_types, described.member_create_info_structure_type_count)
		|| !valid_types(described.handled_structure_types, described.handled_structure_type_count)
		|| (described.provided_service_type_count != 0 && described.provided_service_types == nullptr)
		|| described.min_member_count == 0 || described.min_member_count > described.max_member_count) {
		report(instance, CO_DEBUG_SEVERITY_0_ERROR, CO_RESULT_0_ERROR_INVALID_ARGUMENT, "the getters of a factory returned inconsistent answers", factory);
		return CO_RESULT_0_ERROR_INVALID_ARGUMENT;
	}
	*registered = described;
	return CO_RESULT_0_SUCCESS;
}

// Calls the entry points of every source and registers the factories they create.
co_result_0 register_classes(co_instance_0 instance, const co_in_structure_0* chain) noexcept
{
	// Entry points see the instance allocation callbacks followed by the instance debug callbacks.
	co_allocation_callbacks_0 allocation_callbacks = instance->allocation_callbacks;
	co_debug_callback_0* debug_callbacks = const_cast<co_debug_callback_0*>(instance->debug_callbacks.callbacks);
	for (std::size_t i = 0; i < instance->debug_callbacks.count; ++i) {
		debug_callbacks[i].next_structure = i + 1 < instance->debug_callbacks.count ? CO_IN_STRUCTURE_0_CAST(&debug_callbacks[i + 1]) : nullptr;
	}
	allocation_callbacks.next_structure = instance->debug_callbacks.count != 0 ? CO_IN_STRUCTURE_0_CAST(&debug_callbacks[0]) : nullptr;
	const co_event_domain_group_factory_0_entry_point_info_0 info = {
		.structure_type = CO_STRUCTURE_TYPE_0_EVENT_DOMAIN_GROUP_FACTORY_0_ENTRY_POINT_INFO_0,
		.next_structure = CO_IN_STRUCTURE_0_CAST(&allocation_callbacks),
		.api_version = instance->api_version,
	};

	std::size_t total = 0;
	co_result_0 result = for_each_source(chain, [&](void* data, co_event_domain_group_factory_0_entry_point_0_fn entry_point_fn) noexcept {
		std::size_t count = 0;
		if (co_result_0 counted = entry_point_fn(data, &info, &count, nullptr); counted != CO_RESULT_0_SUCCESS) {
			report(instance, CO_DEBUG_SEVERITY_0_ERROR, counted, "an entry point failed to count its factories", nullptr);
			return counted;
		}
		if (count > std::numeric_limits<std::size_t>::max() - total) {
			return CO_RESULT_0_ERROR_OUT_OF_MEMORY;
		}
		total += count;
		return CO_RESULT_0_SUCCESS;
	});
	if (result != CO_RESULT_0_SUCCESS || total == 0) {
		return result;
	}

	block_layout layout;
	const std::size_t classes_offset = layout.reserve(sizeof(registered_class), alignof(registered_class), total);
	const std::size_t factories_offset = layout.reserve(sizeof(co_event_domain_group_factory_0), alignof(co_event_domain_group_factory_0), total);
	void* block = layout.overflow ? nullptr : co::impl::allocate_0(instance->allocation_callbacks, layout.size, layout.alignment);
	if (block == nullptr) {
		report(instance, CO_DEBUG_SEVERITY_0_ERROR, CO_RESULT_0_ERROR_OUT_OF_MEMORY, "cannot allocate the registered classes", nullptr);
		return CO_RESULT_0_ERROR_OUT_OF_MEMORY;
	}
	std::byte* bytes = static_cast<std::byte*>(block);
	registered_class* classes = reinterpret_cast<registered_class*>(bytes + classes_offset);
	co_event_domain_group_factory_0* factories = reinterpret_cast<co_event_domain_group_factory_0*>(bytes + factories_offset);

	std::size_t created = 0;
	result = for_each_source(chain, [&](void* data, co_event_domain_group_factory_0_entry_point_0_fn entry_point_fn) noexcept {
		std::size_t count = total - created;
		if (co_result_0 entry_result = entry_point_fn(data, &info, &count, factories + created); entry_result != CO_RESULT_0_SUCCESS) {
			report(instance, CO_DEBUG_SEVERITY_0_ERROR, entry_result, "an entry point failed to create its factories", nullptr);
			return entry_result;
		}
		assert(count <= total - created && "an entry point created more factories than requested");
		created += count;
		return CO_RESULT_0_SUCCESS;
	});
	for (std::size_t i = 0; result == CO_RESULT_0_SUCCESS && i < created; ++i) {
		if (factories[i] == nullptr) {
			report(instance, CO_DEBUG_SEVERITY_0_ERROR, CO_RESULT_0_ERROR_INVALID_ARGUMENT, "an entry point created a null factory", nullptr);
			result = CO_RESULT_0_ERROR_INVALID_ARGUMENT;
			break;
		}
		result = describe_factory(instance, factories[i], ::new (&classes[i]) registered_class{});
	}
	if (result != CO_RESULT_0_SUCCESS) {
		destroy_factories(factories, created);
		co::impl::deallocate_0(instance->allocation_callbacks, block, layout.size, layout.alignment);
		return result;
	}

	instance->classes_block = block;
	instance->classes_block_size = layout.size;
	instance->classes_block_alignment = layout.alignment;
	instance->classes = classes;
	instance->class_count = created;
	return CO_RESULT_0_SUCCESS;
}

void destroy_instance(co_instance_0 instance) noexcept
{
	for (std::size_t i = instance->class_count; i != 0; --i) {
		co_event_domain_group_factory_0 factory = instance->classes[i - 1].factory;
		factory->vtable->destroy(factory);
	}
	if (instance->classes_block != nullptr) {
		co::impl::deallocate_0(instance->allocation_callbacks, instance->classes_block, instance->classes_block_size, instance->classes_block_alignment);
	}
	const co_allocation_callbacks_0 allocation_callbacks = instance->allocation_callbacks;
	const std::size_t block_size = instance->block_size;
	const std::size_t block_alignment = instance->block_alignment;
	std::destroy_at(instance);
	co::impl::deallocate_0(allocation_callbacks, instance, block_size, block_alignment);
}

} // namespace

co_result_0 co_instance_0_create_0(const co_instance_0_create_info_0* create_info, co_instance_0* instance_) noexcept
{
	const co_in_structure_0* chain = create_info->next_structure;
	if (instance_ == nullptr || create_info->structure_type != CO_STRUCTURE_TYPE_0_INSTANCE_0_CREATE_INFO_0) {
		report_chain(chain, CO_DEBUG_SEVERITY_0_ERROR, CO_RESULT_0_ERROR_INVALID_ARGUMENT, "invalid instance create info or null instance");
		return CO_RESULT_0_ERROR_INVALID_ARGUMENT;
	}
	if (!is_compatible_api_version(create_info->api_version)) {
		report_chain(chain, CO_DEBUG_SEVERITY_0_ERROR, CO_RESULT_0_ERROR_INCOMPATIBLE_VERSION, "the api_version of the instance create info is not compatible with the library");
		return CO_RESULT_0_ERROR_INCOMPATIBLE_VERSION;
	}

	const co_allocation_callbacks_0* allocation_callbacks = nullptr;
	std::size_t debug_callback_count = 0;
	for (const co_in_structure_0* entry = chain; entry != nullptr; entry = entry->next_structure) {
		if (entry->structure_type == CO_STRUCTURE_TYPE_0_ALLOCATION_CALLBACKS_0) {
			const co_allocation_callbacks_0* callbacks = reinterpret_cast<const co_allocation_callbacks_0*>(entry);
			if (allocation_callbacks != nullptr || callbacks->allocate_fn == nullptr || callbacks->deallocate_fn == nullptr) {
				report_chain(chain, CO_DEBUG_SEVERITY_0_ERROR, CO_RESULT_0_ERROR_INVALID_ARGUMENT, "several or incomplete allocation callbacks in the instance create info");
				return CO_RESULT_0_ERROR_INVALID_ARGUMENT;
			}
			allocation_callbacks = callbacks;
		}
		else if (entry->structure_type == CO_STRUCTURE_TYPE_0_DEBUG_CALLBACK_0) {
			if (reinterpret_cast<const co_debug_callback_0*>(entry)->callback_fn == nullptr) {
				report_chain(chain, CO_DEBUG_SEVERITY_0_ERROR, CO_RESULT_0_ERROR_INVALID_ARGUMENT, "debug callback without callback_fn");
				return CO_RESULT_0_ERROR_INVALID_ARGUMENT;
			}
			++debug_callback_count;
		}
		else if (entry->structure_type == CO_STRUCTURE_TYPE_0_EVENT_DOMAIN_GROUP_FACTORY_SOURCE_0) {
			if (reinterpret_cast<const co_event_domain_group_factory_source_0*>(entry)->entry_point_fn == nullptr) {
				report_chain(chain, CO_DEBUG_SEVERITY_0_ERROR, CO_RESULT_0_ERROR_INVALID_ARGUMENT, "factory source without entry_point_fn");
				return CO_RESULT_0_ERROR_INVALID_ARGUMENT;
			}
		}
		else {
			report_chain(chain, CO_DEBUG_SEVERITY_0_ERROR, CO_RESULT_0_ERROR_NOT_SUPPORTED, "unsupported entry in the next chain of the instance create info");
			return CO_RESULT_0_ERROR_NOT_SUPPORTED;
		}
	}
	if (allocation_callbacks == nullptr) {
		allocation_callbacks = &default_allocation_callbacks_0;
	}

	block_layout layout;
	layout.reserve(sizeof(co_instance_0_t), alignof(co_instance_0_t), 1);
	const std::size_t debug_callbacks_offset = layout.reserve(sizeof(co_debug_callback_0), alignof(co_debug_callback_0), debug_callback_count);
	void* block = layout.overflow ? nullptr : co::impl::allocate_0(*allocation_callbacks, layout.size, layout.alignment);
	if (block == nullptr) {
		report_chain(chain, CO_DEBUG_SEVERITY_0_ERROR, CO_RESULT_0_ERROR_OUT_OF_MEMORY, "cannot allocate the instance");
		return CO_RESULT_0_ERROR_OUT_OF_MEMORY;
	}
	std::byte* bytes = static_cast<std::byte*>(block);

	co_instance_0_t* instance = ::new (block) co_instance_0_t{};
	instance->block_size = layout.size;
	instance->block_alignment = layout.alignment;
	instance->api_version = create_info->api_version;
	instance->allocation_callbacks = *allocation_callbacks;
	instance->allocation_callbacks.next_structure = nullptr;

	co_debug_callback_0* debug_callbacks = reinterpret_cast<co_debug_callback_0*>(bytes + debug_callbacks_offset);
	for (const co_in_structure_0* entry = chain; entry != nullptr; entry = entry->next_structure) {
		if (entry->structure_type == CO_STRUCTURE_TYPE_0_DEBUG_CALLBACK_0) {
			co_debug_callback_0* callback = ::new (&debug_callbacks[instance->debug_callbacks.count++]) co_debug_callback_0(*reinterpret_cast<const co_debug_callback_0*>(entry));
			callback->next_structure = nullptr;
		}
	}
	instance->debug_callbacks.callbacks = debug_callbacks;

	if (co_result_0 result = register_classes(instance, chain); result != CO_RESULT_0_SUCCESS) {
		destroy_instance(instance);
		return result;
	}

	*instance_ = instance;
	return CO_RESULT_0_SUCCESS;
}

void co_instance_0_destroy(co_instance_0 instance) noexcept
{
	if (instance == nullptr) {
		return;
	}
	destroy_instance(instance);
}
