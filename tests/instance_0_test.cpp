#ifdef _WIN32
# define WIN32_LEAN_AND_MEAN
# define NOGDICAPMASKS
# define NOVIRTUALKEYCODES
# define NOWINMESSAGES
# define NOWINSTYLES
# define NOSYSMETRICS
# define NOMENUS
# define NOICONS
# define NOKEYSTATES
# define NOSYSCOMMANDS
# define NORASTEROPS
# define NOSHOWWINDOW
# define NOATOM
# define NOCLIPBOARD
# define NOCOLOR
# define NOCTLMGR
# define NODRAWTEXT
# define NOGDI
# define NOKERNEL
# define NOUSER
# define NONLS
# define NOMB
# define NOMEMMGR
# define NOMETAFILE
# define NOMINMAX
# define NOMSG
# define NOOPENFILE
# define NOSCROLL
# define NOSERVICE
# define NOSOUND
# define NOTEXTMETRIC
# define NOWH
# define NOWINOFFSETS
# define NOCOMM
# define NOKANJI
# define NOHELP
# define NOPROFILER
# define NODEFERWINDOWPOS
# define NOMCX
# define NOIME // ?
# define NORESOURCE // ?
# define WIN32_NO_STATUS
# include <Windows.h>
//# undef WIN32_NO_STATUS
//# include <winternl.h>
//# include <ntstatus.h>
#endif

#include <co.hpp>
#include <co/dynamic_thread_pool_0.h>
#include <co/platform_thread_pool_0.h>
#include <co/strand_0.h>
#include <co/timer_0_service_0.h>
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <co/win32.h>
#endif

#include "test_executors.hpp"
#include "test_guards.hpp"

#include <gtest/gtest.h>

#include <cstddef>
#include <cstring>
#include <new>

namespace
{

const int unknown_structure_type_tag = 0;

co_structure_type_0 unknown_structure_type() noexcept
{
	return static_cast<co_structure_type_0>(reinterpret_cast<uintptr_t>(&unknown_structure_type_tag));
}

co_instance_0_create_info_0 instance_create_info(const co_in_structure_0* next_structure, co_version_0 api_version = CO_API_VERSION_0)
{
	return co_instance_0_create_info_0{
		.structure_type = CO_STRUCTURE_TYPE_0_INSTANCE_0_CREATE_INFO_0,
		.next_structure = next_structure,
		.api_version = api_version,
	};
}

} // namespace

TEST(c_instance_0, rejects_newer_patch_version)
{
	co_instance_0_create_info_0 create_info = instance_create_info(nullptr, CO_API_VERSION_0 + 1);
	co_instance_0 instance = nullptr;
	EXPECT_EQ(co_instance_0_create_0(&create_info, &instance), CO_RESULT_0_ERROR_INCOMPATIBLE_VERSION);
	auto instance_guard = co_test::guard(instance);
	EXPECT_EQ(instance, nullptr);
}

TEST(c_instance_0, rejects_newer_minor_version)
{
	co_version_0 newer = CO_VERSION_0(CO_VERSION_0_EPOCH(CO_API_VERSION_0), CO_VERSION_0_MAJOR(CO_API_VERSION_0), CO_VERSION_0_MINOR(CO_API_VERSION_0) + 1, 0);
	co_instance_0_create_info_0 create_info = instance_create_info(nullptr, newer);
	co_instance_0 instance = nullptr;
	EXPECT_EQ(co_instance_0_create_0(&create_info, &instance), CO_RESULT_0_ERROR_INCOMPATIBLE_VERSION);
	auto instance_guard = co_test::guard(instance);
	EXPECT_EQ(instance, nullptr);
}

TEST(c_instance_0, rejects_newer_major_version)
{
	co_version_0 newer = CO_VERSION_0(CO_VERSION_0_EPOCH(CO_API_VERSION_0), CO_VERSION_0_MAJOR(CO_API_VERSION_0) + 1, 0, 0);
	co_instance_0_create_info_0 create_info = instance_create_info(nullptr, newer);
	co_instance_0 instance = nullptr;
	EXPECT_EQ(co_instance_0_create_0(&create_info, &instance), CO_RESULT_0_ERROR_INCOMPATIBLE_VERSION);
	auto instance_guard = co_test::guard(instance);
	EXPECT_EQ(instance, nullptr);
}

TEST(c_instance_0, rejects_other_epoch)
{
	co_version_0 other = CO_VERSION_0(CO_VERSION_0_EPOCH(CO_API_VERSION_0) + 1, 0, 0, 0);
	co_instance_0_create_info_0 create_info = instance_create_info(nullptr, other);
	co_instance_0 instance = nullptr;
	EXPECT_EQ(co_instance_0_create_0(&create_info, &instance), CO_RESULT_0_ERROR_INCOMPATIBLE_VERSION);
	auto instance_guard = co_test::guard(instance);
}

namespace
{

struct counting_allocator final
{
	int allocations = 0;
	int deallocations = 0;
};

void* counting_allocate(void* data, size_t size, size_t alignment) noexcept
{
	++static_cast<counting_allocator*>(data)->allocations;
	return ::operator new(size, std::align_val_t(alignment), std::nothrow);
}

void counting_deallocate(void* data, void* memory, size_t size, size_t alignment) noexcept
{
	++static_cast<counting_allocator*>(data)->deallocations;
	::operator delete(memory, size, std::align_val_t(alignment));
}

struct debug_log final
{
	int errors = 0;
};

void count_errors(void* data, const co_debug_message_0* message) noexcept
{
	if (message->severity == CO_DEBUG_SEVERITY_0_ERROR) {
		++static_cast<debug_log*>(data)->errors;
	}
}

} // namespace

TEST(c_instance_0, uses_allocation_callbacks_for_instance_and_objects)
{
	counting_allocator instance_allocator;
	counting_allocator object_allocator;
	const co_allocation_callbacks_0 instance_callbacks = {
		.structure_type = CO_STRUCTURE_TYPE_0_ALLOCATION_CALLBACKS_0,
		.next_structure = nullptr,
		.allocate_fn = &counting_allocate,
		.deallocate_fn = &counting_deallocate,
		.data = &instance_allocator,
	};
	co_instance_0_create_info_0 create_info = instance_create_info(CO_IN_STRUCTURE_0_CAST(&instance_callbacks));
	co_instance_0 instance = nullptr;
	ASSERT_EQ(co_instance_0_create_0(&create_info, &instance), CO_RESULT_0_SUCCESS);
	auto instance_guard = co_test::guard(instance);
	EXPECT_EQ(instance_allocator.allocations, 1);

	co_test::manual_executor_0 inner;
	co_strand_0_executor_0_create_info_0 strand_create_info = {
		.structure_type = CO_STRUCTURE_TYPE_0_STRAND_0_EXECUTOR_0_CREATE_INFO_0,
		.next_structure = nullptr,
		.inner_executor = &inner,
	};
	co_strand_0_executor_0 strand = nullptr;
	ASSERT_EQ(co_strand_0_executor_0_create_0(instance, &strand_create_info, &strand), CO_RESULT_0_SUCCESS);
	auto strand_guard = co_test::guard(strand);
	EXPECT_EQ(instance_allocator.allocations, 2);
	strand_guard.reset();
	EXPECT_EQ(instance_allocator.deallocations, 1);

	const co_allocation_callbacks_0 object_callbacks = {
		.structure_type = CO_STRUCTURE_TYPE_0_ALLOCATION_CALLBACKS_0,
		.next_structure = nullptr,
		.allocate_fn = &counting_allocate,
		.deallocate_fn = &counting_deallocate,
		.data = &object_allocator,
	};
	strand_create_info.next_structure = CO_IN_STRUCTURE_0_CAST(&object_callbacks);
	ASSERT_EQ(co_strand_0_executor_0_create_0(instance, &strand_create_info, &strand), CO_RESULT_0_SUCCESS);
	strand_guard.reset(strand);
	EXPECT_EQ(object_allocator.allocations, 1);
	EXPECT_EQ(instance_allocator.allocations, 2);
	strand_guard.reset();
	EXPECT_EQ(object_allocator.deallocations, 1);

	instance_guard.reset();
	EXPECT_EQ(instance_allocator.deallocations, 2);
}

TEST(c_instance_0, rejects_several_allocation_callbacks)
{
	debug_log log;
	counting_allocator allocator;
	const co_debug_callback_0 debug_callback = {
		.structure_type = CO_STRUCTURE_TYPE_0_DEBUG_CALLBACK_0,
		.next_structure = nullptr,
		.min_severity = CO_DEBUG_SEVERITY_0_WARNING,
		.callback_fn = &count_errors,
		.data = &log,
	};
	const co_allocation_callbacks_0 second = {
		.structure_type = CO_STRUCTURE_TYPE_0_ALLOCATION_CALLBACKS_0,
		.next_structure = CO_IN_STRUCTURE_0_CAST(&debug_callback),
		.allocate_fn = &counting_allocate,
		.deallocate_fn = &counting_deallocate,
		.data = &allocator,
	};
	const co_allocation_callbacks_0 first = {
		.structure_type = CO_STRUCTURE_TYPE_0_ALLOCATION_CALLBACKS_0,
		.next_structure = CO_IN_STRUCTURE_0_CAST(&second),
		.allocate_fn = &counting_allocate,
		.deallocate_fn = &counting_deallocate,
		.data = &allocator,
	};
	co_instance_0_create_info_0 create_info = instance_create_info(CO_IN_STRUCTURE_0_CAST(&first));
	co_instance_0 instance = nullptr;
	EXPECT_EQ(co_instance_0_create_0(&create_info, &instance), CO_RESULT_0_ERROR_INVALID_ARGUMENT);
	auto instance_guard = co_test::guard(instance);
	EXPECT_EQ(instance, nullptr);
	EXPECT_EQ(log.errors, 1);
	EXPECT_EQ(allocator.allocations, 0);
}

TEST(c_instance_0, rejects_unknown_next_chain_entry)
{
	const co_in_structure_0 unknown = {.structure_type = unknown_structure_type(), .next_structure = nullptr};
	co_instance_0_create_info_0 create_info = instance_create_info(&unknown);
	co_instance_0 instance = nullptr;
	EXPECT_EQ(co_instance_0_create_0(&create_info, &instance), CO_RESULT_0_ERROR_NOT_SUPPORTED);
	auto instance_guard = co_test::guard(instance);
	EXPECT_EQ(instance, nullptr);
}

TEST(c_instance_0, creates_runtime)
{
	co_instance_0_create_info_0 create_info = instance_create_info(nullptr);
	co_instance_0 instance = nullptr;
	ASSERT_EQ(co_instance_0_create_0(&create_info, &instance), CO_RESULT_0_SUCCESS);
	auto instance_guard = co_test::guard(instance);

	co_runtime_0_create_info_0 runtime_create_info = {};
	runtime_create_info.structure_type = CO_STRUCTURE_TYPE_0_RUNTIME_0_CREATE_INFO_0;
	co_runtime_0 runtime = nullptr;
	EXPECT_EQ(co_instance_0_create_runtime_0(instance, &runtime_create_info, &runtime), CO_RESULT_0_SUCCESS);
	auto runtime_guard = co_test::guard(runtime);
	EXPECT_NE(runtime, nullptr);
}

#ifdef _WIN32
TEST(c_event_facility_0_class_0, win32_thread_pool_adapter_is_platform_thread_pool_adapter)
{
	const co_event_domain_group_factory_source_0 win32_classes = {
		.structure_type = CO_STRUCTURE_TYPE_0_EVENT_DOMAIN_GROUP_FACTORY_SOURCE_0,
		.next_structure = nullptr,
		.entry_point_fn = &co_win32_event_domain_group_factory_0_entry_point_0,
		.data = nullptr,
	};
	co_instance_0_create_info_0 create_info = instance_create_info(CO_IN_STRUCTURE_0_CAST(&win32_classes));
	co_instance_0 instance = nullptr;
	ASSERT_EQ(co_instance_0_create_0(&create_info, &instance), CO_RESULT_0_SUCCESS);
	auto instance_guard = co_test::guard(instance);

	const co_service_type_0 required_services[] = {CO_SERVICE_TYPE_0_TIMER_0_SERVICE_0};
	const co_platform_thread_pool_0_create_info_0 platform_thread_pool = {
		.structure_type = CO_STRUCTURE_TYPE_0_PLATFORM_THREAD_POOL_0_CREATE_INFO_0,
		.next_structure = nullptr,
	};
	const co_event_facility_0_create_info_0 event_facility_create_info = {
		.structure_type = CO_STRUCTURE_TYPE_0_EVENT_FACILITY_0_CREATE_INFO_0,
		.next_structure = CO_IN_STRUCTURE_0_CAST(&platform_thread_pool),
		.required_services = required_services,
		.required_service_count = std::size(required_services),
	};
	co_event_domain_0 event_domain = nullptr;
	ASSERT_EQ(co_instance_0_create_event_domain_0(instance, CO_EVENT_DOMAIN_0_CREATE_INFO_0_CAST(&event_facility_create_info), &event_domain), CO_RESULT_0_SUCCESS);
	auto event_domain_guard = co_test::guard(event_domain);
	co_event_facility_0 event_facility = CO_EVENT_FACILITY_0_CAST(event_domain);
	co_service_0 service = nullptr;
	EXPECT_EQ(co_event_facility_0_get_service_0(event_facility, CO_SERVICE_TYPE_0_TIMER_0_SERVICE_0, &service), CO_RESULT_0_SUCCESS);
	EXPECT_NE(service, nullptr);
}
#endif
