if(WIN32)
	add_executable(co_example_async WIN32)

	target_sources(co_example_async
		PRIVATE
			"async.cpp"
	)

	target_compile_definitions(co_example_async
		PRIVATE
			"UNICODE"
			"_UNICODE"
	)

	target_link_libraries(co_example_async
		PRIVATE
			${CO_CMAKE_PACKAGE_NAME}::co
			${CO_CMAKE_PACKAGE_NAME}::win32
	)

	add_custom_command(TARGET co_example_async POST_BUILD
		COMMAND "${CMAKE_COMMAND}" -E copy -t "$<TARGET_FILE_DIR:co_example_async>" "$<TARGET_RUNTIME_DLLS:co_example_async>"
		COMMAND_EXPAND_LISTS
	)
endif()
