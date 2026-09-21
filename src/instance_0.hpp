#pragma once

// Internal state of instances.

#include <co.h>

#include <cstddef>
#include <cstdint>

namespace co::detail
{

// Allocation callbacks using the global operator new and delete.
extern const co_allocation_callbacks_0 default_allocation_callbacks_0;

// Whether a component written against api_version works with this library; see co_instance_0_create_info_0.
bool is_compatible_api_version(co_version_0 api_version) noexcept;

struct debug_callbacks final
{
	const co_debug_callback_0* callbacks;
	std::size_t count;
};

void report(const debug_callbacks& callbacks, co_debug_severity_0 severity, co_result_0 result, const char* message, const void* object, std::int64_t system_error = 0) noexcept;

// Returns the allocation callbacks in the next chain of an object create info, or those of the instance.
// Fails with CO_RESULT_0_ERROR_INVALID_ARGUMENT when the chain holds several.
co_result_0 find_allocation_callbacks(co_instance_0 instance, const co_in_structure_0* next_structure, const co_allocation_callbacks_0** allocation_callbacks) noexcept;

// Fails with CO_RESULT_0_ERROR_NOT_SUPPORTED when the next chain holds an entry other than allocation callbacks and
// hints.
co_result_0 check_next_chain(co_instance_0 instance, const co_in_structure_0* next_structure) noexcept;

void report(co_instance_0 instance, co_debug_severity_0 severity, co_result_0 result, const char* message, const void* object, std::int64_t system_error = 0) noexcept;

// A registered event domain group class: its factory and the answers of its getters.
struct registered_class final
{
	co_event_domain_group_factory_0 factory;
	const co_structure_type_0* member_create_info_structure_types;
	std::size_t member_create_info_structure_type_count;
	const co_structure_type_0* handled_structure_types;
	std::size_t handled_structure_type_count;
	const co_service_type_0* provided_service_types;
	std::size_t provided_service_type_count;
	std::size_t min_member_count;
	std::size_t max_member_count;
	std::uintptr_t rank;
};

// Entry point of the built-in event domain group classes.
co_result_0 builtin_event_domain_group_factories_0(void* data, const co_event_domain_group_factory_0_entry_point_info_0* info, std::size_t* factory_count, co_event_domain_group_factory_0* factories) noexcept;

// Creates a group of member_count members from the event domain create info member_create_info, of the class chosen as
// described for co_instance_0_create_event_domain_0(), and verifies that every member provides every required
// service.
co_result_0 create_event_domain_group(co_instance_0 instance, const co_event_domain_0_create_info_0* member_create_info, std::size_t member_count, co_event_domain_group_0* group) noexcept;

} // namespace co::detail

// The instance is allocated in a single block also holding the copies of its debug callbacks. The factories and the
// registered classes are allocated in a second block.
struct co_instance_0_t final
{
	std::size_t block_size;
	std::size_t block_alignment;
	co_version_0 api_version;
	co_allocation_callbacks_0 allocation_callbacks;
	co::detail::debug_callbacks debug_callbacks;

	void* classes_block;
	std::size_t classes_block_size;
	std::size_t classes_block_alignment;
	// In registration order.
	co::detail::registered_class* classes;
	std::size_t class_count;
};
