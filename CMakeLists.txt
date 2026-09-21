cmake_minimum_required(VERSION 4.2)

project(co
	VERSION 0.0
	LANGUAGES C CXX
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(co SHARED)
add_library(co::co ALIAS co)

set(co_sources
	"src/dynamic_thread_pool_0.cpp"
	"src/event_domain_group_factory_0.cpp"
	"src/event_domain_group_0.cpp"
	"src/event_domain_0.cpp"
	"src/event_facility_0.cpp"
	"src/event_loop_0.cpp"
	"src/executor_0.cpp"
	"src/executor_priority_0.cpp"
	"src/file_0_service_0.cpp"
	"src/fixed_thread_pool_0.cpp"
	"src/instance_0.cpp"
	"src/platform_thread_pool_0.cpp"
	"src/runtime_0.cpp"
	"src/service_0.cpp"
	"src/socket_0_service_0.cpp"
	"src/strand_0.cpp"
	"src/thread_count_bounds_0.cpp"
	"src/timer_0.cpp"
	"src/timer_0_service_0.cpp"
)

target_sources(co
	PRIVATE
		${co_sources}
)

include(GenerateExportHeader)

set(co_export_header "${CMAKE_CURRENT_BINARY_DIR}/include/co/export.h")

generate_export_header(co
	BASE_NAME "CO"
	EXPORT_MACRO_NAME "CO_API"
	EXPORT_FILE_NAME "${co_export_header}"
)

set(co_headers
	"include/co.h"
	"include/co.hpp"
	"include/co/awaitable_timer_0.hpp"
	"include/co/dynamic_thread_pool_0.h"
	"include/co/dynamic_thread_pool_0.hpp"
	"include/co/executor_priority_0.h"
	"include/co/executor_priority_0.hpp"
	"include/co/file_0_service_0.h"
	"include/co/file_0_service_0.hpp"
	"include/co/fixed_thread_pool_0.h"
	"include/co/fixed_thread_pool_0.hpp"
	"include/co/impl.hpp"
	"include/co/platform_thread_pool_0.h"
	"include/co/platform_thread_pool_0.hpp"
	"include/co/socket_0_service_0.h"
	"include/co/socket_0_service_0.hpp"
	"include/co/strand_0.h"
	"include/co/strand_0.hpp"
	"include/co/task_0.hpp"
	"include/co/thread_count_bounds_0.h"
	"include/co/thread_count_bounds_0.hpp"
	"include/co/timer_0.h"
	"include/co/timer_0.hpp"
	"include/co/timer_0_service_0.h"
	"include/co/timer_0_service_0.hpp"
)

target_sources(co
	PUBLIC
		FILE_SET HEADERS
		BASE_DIRS
			"include"
			"${CMAKE_CURRENT_BINARY_DIR}/include"
		FILES
			"${co_export_header}"
			"${co_headers}"
)

set(co_win32_sources
	"src/win32.cpp"
	"src/win32/iocp_adapter_0.cpp"
	"src/win32/iocp_0.cpp"
	"src/win32/platform_thread_pool_adapter_0.cpp"
	"src/win32/platform_thread_pool_adapter_1.cpp"
	"src/win32/thread_pool_adapter_0.cpp"
	"src/win32/thread_pool_0.cpp"
	"src/win32/tp_executor_priority.hpp"
)

target_sources(co
	PRIVATE
		"${co_win32_sources}"
)

if(NOT WIN32)
	set_source_files_properties("${co_win32_sources}"
		PROPERTIES
			HEADER_FILE_ONLY ON
	)

	source_group("Source Files"
		FILES
			"${co_win32_sources}"
	)
endif()

set(co_win32_headers
	"include/co/win32.h"
	"include/co/win32.hpp"
	"include/co/win32/executor_priority_0.h"
	"include/co/win32/executor_priority_0.hpp"
	"include/co/win32/iocp_adapter_0.h"
	"include/co/win32/iocp_adapter_0.hpp"
	"include/co/win32/iocp_0.h"
	"include/co/win32/iocp_0.hpp"
	"include/co/win32/platform_thread_pool_adapter_0.h"
	"include/co/win32/platform_thread_pool_adapter_0.hpp"
	"include/co/win32/platform_thread_pool_adapter_1.h"
	"include/co/win32/platform_thread_pool_adapter_1.hpp"
	"include/co/win32/thread_pool_adapter_0.h"
	"include/co/win32/thread_pool_adapter_0.hpp"
	"include/co/win32/thread_pool_0.h"
	"include/co/win32/thread_pool_0.hpp"
)

if(WIN32)
	target_sources(co
		PUBLIC
			FILE_SET HEADERS
			FILES
				"${co_win32_headers}"
	)
else()
	target_sources(co
		PRIVATE
			"${co_win32_headers}"
	)
endif()

if(WIN32)
	target_compile_definitions(co
		PRIVATE
			"UNICODE"
			"_UNICODE"
	)
endif()

target_include_directories(co
	PRIVATE
		"include/co"
)

option(CO_BUILD_TESTS "Build the tests" ${PROJECT_IS_TOP_LEVEL})

if(CO_BUILD_TESTS)
	enable_testing()
	add_subdirectory("tests")
endif()

option(CO_BUILD_DOCS "Add a 'docs' target that builds the Sphinx documentation" OFF)

if(CO_BUILD_DOCS)
	find_package(Doxygen REQUIRED)
	find_program(SPHINX_BUILD_EXECUTABLE
		NAMES "sphinx-build"
		HINTS "${CMAKE_CURRENT_SOURCE_DIR}/.venv/Scripts" "${CMAKE_CURRENT_SOURCE_DIR}/.venv/bin"
		REQUIRED
	)

	add_custom_target(docs
		COMMAND "${CMAKE_COMMAND}" -E env "DOXYGEN_EXECUTABLE=${DOXYGEN_EXECUTABLE}"
			"${SPHINX_BUILD_EXECUTABLE}" -b html
				"${CMAKE_CURRENT_SOURCE_DIR}/docs"
				"${CMAKE_CURRENT_BINARY_DIR}/docs/html"
		WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/docs"
		COMMENT "Building documentation"
		VERBATIM
	)
endif()

option(CO_BUILD_EXAMPLES "Build the examples" ${PROJECT_IS_TOP_LEVEL})

if(CO_BUILD_EXAMPLES)
	add_subdirectory("examples")
endif()
