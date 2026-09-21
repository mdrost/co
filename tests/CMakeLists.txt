add_executable(co_tests)

set(co_tests_sources
	"clock_0_test.cpp"
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
	"timer_0_test.cpp"
	"version_0_test.cpp"
	"test_executors.hpp"
	"test_guards.hpp"
)

target_sources(co_tests
	PRIVATE
		"${co_tests_sources}"
)

target_link_libraries(co_tests
	PRIVATE
		${CO_CMAKE_PACKAGE_NAME}::co
		${CO_CMAKE_PACKAGE_NAME}::win32
		GTest::gtest_main
)

if(WIN32)
	add_custom_command(TARGET co_tests POST_BUILD
		COMMAND "${CMAKE_COMMAND}" -E copy -t "$<TARGET_FILE_DIR:co_tests>" "$<TARGET_RUNTIME_DLLS:co_tests>"
		COMMAND_EXPAND_LISTS
	)
endif()

gtest_discover_tests(co_tests)

add_executable(co_c_api_test)

target_sources(co_c_api_test
	PRIVATE
		"c_api_test.c"
)

set_target_properties(co_c_api_test PROPERTIES
	C_STANDARD 11
	C_STANDARD_REQUIRED ON
	C_EXTENSIONS OFF
)

target_link_libraries(co_c_api_test
	PRIVATE
		${CO_CMAKE_PACKAGE_NAME}::co
		${CO_CMAKE_PACKAGE_NAME}::win32
)

if(WIN32)
	add_custom_command(TARGET co_c_api_test POST_BUILD
		COMMAND "${CMAKE_COMMAND}" -E copy -t "$<TARGET_FILE_DIR:co_c_api_test>" "$<TARGET_RUNTIME_DLLS:co_c_api_test>"
		COMMAND_EXPAND_LISTS
	)
endif()

add_test(NAME co_c_api_test COMMAND co_c_api_test)
