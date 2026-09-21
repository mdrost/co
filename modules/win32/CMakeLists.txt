add_library(co_win32 ${CO_LIBRARY_TYPE})
add_library(${CO_CMAKE_PACKAGE_NAME}::win32 ALIAS co_win32)

#

set(co_win32_sources
	"src/win32.cpp"
	"src/win32/clock_0.cpp"
	"src/win32/iocp_adapter_0.cpp"
	"src/win32/iocp_0.cpp"
	"src/win32/message_pump_0.cpp"
	"src/win32/platform_thread_pool_adapter_0.cpp"
	"src/win32/platform_thread_pool_adapter_1.cpp"
	"src/win32/thread_pool_adapter_0.cpp"
	"src/win32/thread_pool_0.cpp"
	"src/win32/tp_executor_priority.hpp"
)

target_sources(co_win32
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
	"include/co/win32/api.h"
	"include/co/win32/clock_0.h"
	"include/co/win32/clock_0.hpp"
	"include/co/win32/executor_priority_0.h"
	"include/co/win32/executor_priority_0.hpp"
	"include/co/win32/iocp_adapter_0.h"
	"include/co/win32/iocp_adapter_0.hpp"
	"include/co/win32/iocp_0.h"
	"include/co/win32/iocp_0.hpp"
	"include/co/win32/message_pump_0.h"
	"include/co/win32/message_pump_0.hpp"
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
	target_sources(co_win32
		PUBLIC
			FILE_SET HEADERS
			BASE_DIRS
				"include"
				"${CMAKE_CURRENT_BINARY_DIR}/include"
			FILES
				"${co_win32_headers}"
	)
else()
	target_sources(co_win32
		PRIVATE
			"${co_win32_headers}"
	)
endif()

if(CO_LIBRARY_TYPE STREQUAL "STATIC")
	target_compile_definitions(co_win32
		PUBLIC
			"CO_WIN32_STATIC"
	)
endif()

if(WIN32)

	target_compile_definitions(co_win32
		PRIVATE
			"UNICODE"
			"_UNICODE"
	)
endif()

target_include_directories(co_win32
	PRIVATE
		"include/co"
		"include/co/win32"
)

target_link_libraries(co_win32
	PUBLIC
		${CO_CMAKE_PACKAGE_NAME}::co
)

set_target_properties(co_win32 PROPERTIES
	DEFINE_SYMBOL CO_WIN32_EXPORTS
	OUTPUT_NAME "co-${CO_ABI_VERSION}-win32${CO_OUTPUT_NAME_SUFFIX}"
	EXPORT_NAME "win32"
	VERSION "${PROJECT_VERSION}"
	SOVERSION "${CO_SOVERSION}"
)

if(WIN32)
	install(TARGETS co_win32
		EXPORT co-win32-targets
		ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
		LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
		RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
		FILE_SET HEADERS DESTINATION "${CO_INSTALL_INCLUDEDIR}"
	)

	install(EXPORT co-win32-targets
		FILE "${CO_CMAKE_PACKAGE_NAME}-win32-${CO_LIBRARY_KIND}-targets.cmake"
		NAMESPACE "${CO_CMAKE_PACKAGE_NAME}::"
		DESTINATION "${CO_INSTALL_CMAKEDIR}"
	)
endif()