add_executable(co_win32_tests)

set(co_win32_tests_sources
	"clock_0_test.cpp"
	"executor_priority_0_test.cpp"
	"platform_thread_pool_adapter_0_test.cpp"
	"test.cpp"
)

target_sources(co_win32_tests
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

target_include_directories(co_win32_tests
	PRIVATE
		"${PROJECT_SOURCE_DIR}/tests"
)

target_link_libraries(co_win32_tests
	PRIVATE
		${CO_CMAKE_PACKAGE_NAME}::co
		${CO_CMAKE_PACKAGE_NAME}::win32
		GTest::gtest_main
)

if(WIN32)
	add_custom_command(TARGET co_win32_tests POST_BUILD
		COMMAND "${CMAKE_COMMAND}" -E copy -t "$<TARGET_FILE_DIR:co_win32_tests>" "$<TARGET_RUNTIME_DLLS:co_win32_tests>"
		COMMAND_EXPAND_LISTS
	)
endif()

gtest_discover_tests(co_win32_tests)
