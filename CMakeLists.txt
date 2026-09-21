cmake_minimum_required(VERSION 4.2)

project(co
	VERSION 0.0.0.0
	LANGUAGES C CXX
)

#

include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

#

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Helper variables

# Version components that break compatibility when bumped, as checked by co_instance_0_create_0(): the epoch, then
# major, minor and patch up to and including the first nonzero one. 1.2.3.4 gives 1_2, 0.0.1.5 gives 0_0_1.
set(co_abi_components "${PROJECT_VERSION_MAJOR}")
foreach(component IN ITEMS "${PROJECT_VERSION_MINOR}" "${PROJECT_VERSION_PATCH}" "${PROJECT_VERSION_TWEAK}")
	list(APPEND co_abi_components "${component}")
	if(NOT component EQUAL 0)
		break()
	endif()
endforeach()
list(JOIN co_abi_components "_" CO_ABI_VERSION)
list(JOIN co_abi_components "." CO_SOVERSION)

set(CO_CMAKE_PACKAGE_NAME "co_${CO_ABI_VERSION}")
set(CO_INSTALL_INCLUDEDIR "${CMAKE_INSTALL_INCLUDEDIR}/co-${CO_ABI_VERSION}")
set(CO_INSTALL_CMAKEDIR "${CMAKE_INSTALL_LIBDIR}/cmake/${CO_CMAKE_PACKAGE_NAME}")

# Library kind of co and all its modules, one per build: CO_SHARED_LIBS overrides BUILD_SHARED_LIBS for co only, and
# without either co is shared. Structure types are identified by addresses, so a static co must be linked into a single
# binary together with all its modules.
if(DEFINED CO_SHARED_LIBS)
	set(co_shared_libs "${CO_SHARED_LIBS}")
elseif(DEFINED BUILD_SHARED_LIBS)
	set(co_shared_libs "${BUILD_SHARED_LIBS}")
else()
	set(co_shared_libs ON)
endif()

if(co_shared_libs)
	set(CO_LIBRARY_TYPE "SHARED")
	set(CO_LIBRARY_KIND "shared")
	set(CO_OUTPUT_NAME_SUFFIX "")
else()
	set(CO_LIBRARY_TYPE "STATIC")
	set(CO_LIBRARY_KIND "static")
	# Where static libraries are named like import libraries (co.lib with MSVC), static ones get a suffix so both kinds
	# can be installed into the same prefix.
	if("${CMAKE_STATIC_LIBRARY_PREFIX}${CMAKE_STATIC_LIBRARY_SUFFIX}" STREQUAL "${CMAKE_IMPORT_LIBRARY_PREFIX}${CMAKE_IMPORT_LIBRARY_SUFFIX}")
		set(CO_OUTPUT_NAME_SUFFIX "-static")
	else()
		set(CO_OUTPUT_NAME_SUFFIX "")
	endif()
endif()

#

add_library(co ${CO_LIBRARY_TYPE})
add_library(${CO_CMAKE_PACKAGE_NAME}::co ALIAS co)

#

set(co_sources
	"src/cancellation_0.cpp"
	"src/clock_0.cpp"
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
	"src/impl.cpp"
	"src/instance_0.cpp"
	"src/platform_thread_pool_0.cpp"
	"src/runtime_0.cpp"
	"src/service_0.cpp"
	"src/socket_0_service_0.cpp"
	"src/suspend_watcher_0.cpp"
	"src/suspend_watcher_0_service_0.cpp"
	"src/strand_0.cpp"
	"src/thread_count_bounds_0.cpp"
	"src/timer_0.cpp"
	"src/timer_0_service_0.cpp"
	"src/time_watcher_0.cpp"
	"src/time_watcher_0_service_0.cpp"
)

target_sources(co
	PRIVATE
		${co_sources}
)

set(co_headers
	"include/co.h"
	"include/co.hpp"
	"include/co/api.h"
	"include/co/clock_0.h"
	"include/co/clock_0.hpp"
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
	"include/co/suspend_watcher_0.h"
	"include/co/suspend_watcher_0.hpp"
	"include/co/suspend_watcher_0_service_0.h"
	"include/co/suspend_watcher_0_service_0.hpp"
	"include/co/strand_0.h"
	"include/co/strand_0.hpp"
	"include/co/task_0.hpp"
	"include/co/thread_count_bounds_0.h"
	"include/co/thread_count_bounds_0.hpp"
	"include/co/timer_0.h"
	"include/co/timer_0.hpp"
	"include/co/timer_0_service_0.h"
	"include/co/timer_0_service_0.hpp"
	"include/co/time_watcher_0.h"
	"include/co/time_watcher_0.hpp"
	"include/co/time_watcher_0_service_0.h"
	"include/co/time_watcher_0_service_0.hpp"
)

target_sources(co
	PUBLIC
		FILE_SET HEADERS
		BASE_DIRS
			"include"
			"${CMAKE_CURRENT_BINARY_DIR}/include"
		FILES
			"${co_headers}"
)

target_compile_features(co
	PUBLIC
		c_std_11
		cxx_std_20
)

if(CO_LIBRARY_TYPE STREQUAL "STATIC")
	target_compile_definitions(co
		PUBLIC
			"CO_STATIC"
	)
endif()

target_include_directories(co
	PRIVATE
		"include/co"
)

set_target_properties(co PROPERTIES
	DEFINE_SYMBOL CO_EXPORTS
	OUTPUT_NAME "co-${CO_ABI_VERSION}${CO_OUTPUT_NAME_SUFFIX}"
	VERSION "${PROJECT_VERSION}"
	SOVERSION "${CO_SOVERSION}"
)

# Modules

add_subdirectory("modules/linux")
add_subdirectory("modules/posix")
add_subdirectory("modules/win32")

# Tests

option(CO_BUILD_TESTS "Build the tests" ${PROJECT_IS_TOP_LEVEL})

if(CO_BUILD_TESTS)
	include(FetchContent)
	FetchContent_Declare(googletest
		URL "https://github.com/google/googletest/archive/refs/tags/v1.17.0.zip"
		FIND_PACKAGE_ARGS NAMES GTest
	)
	set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
	set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
	FetchContent_MakeAvailable(googletest)
	include(GoogleTest)

	enable_testing()
	add_subdirectory("tests")
	add_subdirectory("modules/linux/tests")
	add_subdirectory("modules/posix/tests")
	add_subdirectory("modules/win32/tests")
endif()

# Docs

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

# Examples

option(CO_BUILD_EXAMPLES "Build the examples" ${PROJECT_IS_TOP_LEVEL})

if(CO_BUILD_EXAMPLES)
	add_subdirectory("examples")
endif()

# Installation

install(TARGETS co
    EXPORT co-targets
    ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
    FILE_SET HEADERS DESTINATION "${CO_INSTALL_INCLUDEDIR}"
)

install(EXPORT co-targets
    FILE "${CO_CMAKE_PACKAGE_NAME}-${CO_LIBRARY_KIND}-targets.cmake"
    NAMESPACE "${CO_CMAKE_PACKAGE_NAME}::"
    DESTINATION "${CO_INSTALL_CMAKEDIR}"
)

configure_package_config_file("cmake/co-config.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/${CO_CMAKE_PACKAGE_NAME}-config.cmake"
    INSTALL_DESTINATION "${CO_INSTALL_CMAKEDIR}"
)

write_basic_package_version_file("${CMAKE_CURRENT_BINARY_DIR}/${CO_CMAKE_PACKAGE_NAME}-config-version.cmake"
    VERSION "${PROJECT_VERSION}"
    COMPATIBILITY AnyNewerVersion
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/${CO_CMAKE_PACKAGE_NAME}-config.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/${CO_CMAKE_PACKAGE_NAME}-config-version.cmake"
    DESTINATION "${CO_INSTALL_CMAKEDIR}"
)
