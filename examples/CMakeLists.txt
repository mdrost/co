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
			co::co
	)

	add_custom_command(TARGET co_example_async POST_BUILD
		COMMAND "${CMAKE_COMMAND}" -E copy_if_different "$<TARGET_RUNTIME_DLLS:co_example_async>" "$<TARGET_FILE_DIR:co_example_async>"
		COMMAND_EXPAND_LISTS
	)
endif()
