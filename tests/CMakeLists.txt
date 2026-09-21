include(FetchContent)

FetchContent_Declare(googletest
	URL "https://github.com/google/googletest/archive/refs/tags/v1.17.0.zip"
	FIND_PACKAGE_ARGS NAMES GTest
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest)

include(GoogleTest)

add_executable(co_tests)

set(co_tests_sources
	"awaitable_timer_0_test.cpp"
	"dynamic_thread_pool_0_test.cpp"
	"event_domain_group_0_test.cpp"
	"event_domain_0_test.cpp"
	"event_facility_0_test.cpp"
	"event_loop_0_test.cpp"
	"executor_0_test.cpp"
	"executor_priority_test.cpp"
	"instance_0_test.cpp"
	"platform_thread_pool_0_test.cpp"
	"runtime_0_test.cpp"
	"strand_0_test.cpp"
	"structure_0_test.cpp"
	"task_0_test.cpp"
	"thread_count_bounds_test.cpp"
	"timer_service_0_test.cpp"
	"version_0_test.cpp"
	"test_executors.hpp"
	"test_guards.hpp"
)

target_sources(co_tests
	PRIVATE
		"${co_tests_sources}"
)

set(co_win32_tests_sources
	"win32_test.cpp"
	"win32_executor_priority_test.cpp"
	"win32_platform_thread_pool_adapter_0_test.cpp"
)

target_sources(co_tests
	PRIVATE
		"${co_win32_tests_sources}"
)

if(NOT WIN32)
	set_source_files_properties("${co_win32_tests_sources}"
		PROPERTIES
			HEADER_FILE_ONLY ON
	)

	source_group("Source Files"
		FILES
			"${co_win32_tests_sources}"
	)
endif()

target_link_libraries(co_tests
	PRIVATE
		co::co
		GTest::gtest_main
)

if(WIN32)
	add_custom_command(TARGET co_tests POST_BUILD
		COMMAND "${CMAKE_COMMAND}" -E copy_if_different "$<TARGET_RUNTIME_DLLS:co_tests>" "$<TARGET_FILE_DIR:co_tests>"
		COMMAND_EXPAND_LISTS
	)
endif()

gtest_discover_tests(co_tests)
