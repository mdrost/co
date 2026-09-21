// Compiles the public C headers as C11 and checks that the C typedefs of the enumerations keep their C++ sizes.

#include <co.h>
#include <co/clock_0.h>
#include <co/dynamic_thread_pool_0.h>
#include <co/executor_priority_0.h>
#include <co/file_0_service_0.h>
#include <co/fixed_thread_pool_0.h>
#include <co/platform_thread_pool_0.h>
#include <co/socket_0_service_0.h>
#include <co/strand_0.h>
#include <co/suspend_watcher_0.h>
#include <co/suspend_watcher_0_service_0.h>
#include <co/thread_count_bounds_0.h>
#include <co/timer_0.h>
#include <co/timer_0_service_0.h>
#include <co/time_watcher_0.h>
#include <co/time_watcher_0_service_0.h>
#ifdef _WIN32
# include <windows.h>
# include <co/win32.h>
#endif

#include <stdio.h>

_Static_assert(sizeof(co_result_0) == sizeof(intptr_t), "co_result_0 has the size of intptr_t");
_Static_assert(sizeof(co_structure_type_0) == sizeof(uintptr_t), "co_structure_type_0 has the size of uintptr_t");
_Static_assert(sizeof(co_service_type_0) == sizeof(uintptr_t), "co_service_type_0 has the size of uintptr_t");
_Static_assert(sizeof(co_debug_severity_0) == sizeof(uintptr_t), "co_debug_severity_0 has the size of uintptr_t");

int main(void)
{
	const co_platform_thread_pool_0_create_info_0 platform = {
		.structure_type = CO_STRUCTURE_TYPE_0_PLATFORM_THREAD_POOL_0_CREATE_INFO_0,
	};
	const co_event_facility_0_create_info_0 event_facility_create_info = {
		.structure_type = CO_STRUCTURE_TYPE_0_EVENT_FACILITY_0_CREATE_INFO_0,
		.next_structure = CO_IN_STRUCTURE_0_CAST(&platform),
	};
	const co_event_domain_0_create_info_0* event_domain_create_info = CO_EVENT_DOMAIN_0_CREATE_INFO_0_CAST(&event_facility_create_info);
	if (event_domain_create_info->next_structure != CO_IN_STRUCTURE_0_CAST(&platform)) {
		fprintf(stderr, "CO_EVENT_DOMAIN_0_CREATE_INFO_0_CAST changed the create info\n");
		return 1;
	}

	const co_instance_0_create_info_0 instance_create_info = {
		.structure_type = CO_STRUCTURE_TYPE_0_INSTANCE_0_CREATE_INFO_0,
		.api_version = CO_API_VERSION_0,
	};
	co_instance_0 instance = NULL;
	const co_result_0 result = co_instance_0_create_0(&instance_create_info, &instance);
	if (result != CO_RESULT_0_SUCCESS) {
		fprintf(stderr, "co_instance_0_create_0 failed: %lld\n", (long long)result);
		return 1;
	}
	co_instance_0_destroy(instance);
	return 0;
}
