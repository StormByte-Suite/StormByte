option(ENABLE_TEST "Enable Unit Tests" OFF)
if(ENABLE_TEST)
	enable_testing()

	add_executable(Base64Tests base64_test.cxx)
	target_link_libraries(Base64Tests StormByte)
	add_test(NAME Base64Tests COMMAND Base64Tests)

	add_executable(ByteSizeTests byte_size_test.cxx)
	target_link_libraries(ByteSizeTests StormByte)
	add_test(NAME ByteSizeTests COMMAND ByteSizeTests)

	add_executable(ErrorTests error_test.cxx)
	target_link_libraries(ErrorTests StormByte)
	add_test(NAME ErrorTests COMMAND ErrorTests)

	add_executable(ExceptionTests exception_test.cxx)
	target_link_libraries(ExceptionTests StormByte)
	add_test(NAME ExceptionTests COMMAND ExceptionTests)

	add_executable(SerializableTests serializable_test.cxx)
	target_link_libraries(SerializableTests StormByte)
	add_test(NAME SerializableTests COMMAND SerializableTests)

	add_executable(SizeTests size_test.cxx)
	target_link_libraries(SizeTests StormByte)
	add_test(NAME SizeTests COMMAND SizeTests)

	add_executable(TelemetryTests telemetry_test.cxx)
	target_link_libraries(TelemetryTests StormByte)
	add_test(NAME TelemetryTests COMMAND TelemetryTests)

	add_executable(ThreadLockTests thread_lock_test.cxx)
	target_link_libraries(ThreadLockTests StormByte)
	add_test(NAME ThreadLockTests COMMAND ThreadLockTests)

	add_executable(UUIDTest uuid_test.cxx)
	target_link_libraries(UUIDTest StormByte)
	add_test(NAME UUIDTest COMMAND UUIDTest)

	add_subdirectory(safe)
	add_subdirectory(type_traits)

endif()
