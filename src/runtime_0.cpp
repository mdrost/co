#include "co.hpp"

#include <new>
#include <type_traits>

// Rule: Make sure functions in this file are defined in the same order as in co.h file.

// The runtime has a single implementation, so its handle type has no members and runtime_0_impl derives from it at
// no cost.
struct co_runtime_0_t
{
};

namespace co
{

namespace
{

class runtime_0_impl final : public co_runtime_0_t
{
public:

	explicit runtime_0_impl(co_instance_0 instance) noexcept
		: m_instance(instance)
	{
	}

	static runtime_0_impl* from(co_runtime_0 self) noexcept
	{
		return static_cast<runtime_0_impl*>(self);
	}

private:

	co_instance_0 m_instance;
};

// A standard-layout class shares its address with its base subobject, so the empty base co_runtime_0_t takes no
// space and a co_runtime_0 points at the runtime_0_impl.
static_assert(std::is_empty_v<co_runtime_0_t>);
static_assert(std::is_standard_layout_v<runtime_0_impl>);

} // namespace

const structure_type_0 structure_type_0_runtime_0_create_info_0 = []() { return static_cast<structure_type_0>(reinterpret_cast<uintptr_t>(&structure_type_0_runtime_0_create_info_0)); }();

} // namespace co

const co_structure_type_0 CO_STRUCTURE_TYPE_0_RUNTIME_0_CREATE_INFO_0 = []() { return static_cast<co_structure_type_0>(reinterpret_cast<uintptr_t>(&co::structure_type_0_runtime_0_create_info_0)); }();

co_result_0 co_instance_0_create_runtime_0(co_instance_0 instance, const co_runtime_0_create_info_0* /* create_info */, co_runtime_0* runtime) noexcept
{
	// TODO: create the topology described by the create info
	if (instance == nullptr || runtime == nullptr) {
		return CO_RESULT_0_ERROR_INVALID_ARGUMENT;
	}
	co::runtime_0_impl* runtime_impl = new (std::nothrow) co::runtime_0_impl(instance);
	if (runtime == nullptr) {
		return CO_RESULT_0_ERROR_OUT_OF_MEMORY;
	}
	*runtime = runtime_impl;
	return CO_RESULT_0_SUCCESS;
}

void co_runtime_0_destroy(co_runtime_0 runtime) noexcept
{
	delete co::runtime_0_impl::from(runtime);
}

void co_runtime_0_get_event_domain_0(co_runtime_0 /* runtime */, size_t /* group_id */, size_t /* domain_event_index */, co_event_domain_0* /* event_domain */) noexcept
{
	// TODO: look up the event domain created for the group
}

void co_runtime_0_get_executor_0(co_runtime_0 /* runtime */, size_t /* executor_id */, co_executor_0* /* executor */) noexcept
{
	// TODO: look up the executor
}