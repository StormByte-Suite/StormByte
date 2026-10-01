option(ENABLE_TEST "Enable Unit Tests" OFF)
if(ENABLE_TEST)
	enable_testing()

	add_executable(Base64Tests base64_test.cxx)
	target_link_libraries(Base64Tests StormByte)
	add_test(NAME Base64Tests COMMAND Base64Tests)

	add_executable(BinaryDataTests binary_data_test.cxx)
	target_link_libraries(BinaryDataTests StormByte)
	add_test(NAME BinaryDataTests COMMAND BinaryDataTests)

	add_executable(BitmaskTests bitmask_test.cxx)
	target_link_libraries(BitmaskTests StormByte)
	add_test(NAME BitmaskTests COMMAND BitmaskTests)

	add_executable(ByteSizeTests byte_size_test.cxx)
	target_link_libraries(ByteSizeTests StormByte)
	add_test(NAME ByteSizeTests COMMAND ByteSizeTests)

	# Shared and hidden by default, like a consumer DLL deriving from Clonable.
	add_library(ClonablePlugin SHARED clonable_plugin.cxx)
	target_link_libraries(ClonablePlugin PUBLIC StormByte)
	set_target_properties(ClonablePlugin PROPERTIES
		CXX_VISIBILITY_PRESET hidden
		VISIBILITY_INLINES_HIDDEN ON
	)

	add_executable(ClonableTests clonable_test.cxx)
	target_link_libraries(ClonableTests ClonablePlugin StormByte)
	# Export the executable's vague-linkage typeinfo so it merges with the plugin's, as between two DLLs.
	set_target_properties(ClonableTests PROPERTIES ENABLE_EXPORTS ON)
	add_test(NAME ClonableTests COMMAND ClonableTests)

	add_executable(CStringTests cstring_test.cxx)
	target_link_libraries(CStringTests StormByte)
	add_test(NAME CStringTests COMMAND CStringTests)

	add_executable(ErrorTests error_test.cxx)
	target_link_libraries(ErrorTests StormByte)
	add_test(NAME ErrorTests COMMAND ErrorTests)

	add_executable(ExceptionTests exception_test.cxx)
	target_link_libraries(ExceptionTests StormByte)
	add_test(NAME ExceptionTests COMMAND ExceptionTests)

	add_executable(ExpectedTests expected_test.cxx)
	target_link_libraries(ExpectedTests StormByte)
	add_test(NAME ExpectedTests COMMAND ExpectedTests)

	add_executable(IterableTests iterable_test.cxx)
	target_link_libraries(IterableTests StormByte)
	add_test(NAME IterableTests COMMAND IterableTests)

	add_executable(SafePointersTests safe_pointers_test.cxx)
	target_link_libraries(SafePointersTests StormByte)
	add_test(NAME SafePointersTests COMMAND SafePointersTests)

	add_executable(SerializationTests serialization_test.cxx)
	target_link_libraries(SerializationTests StormByte)
	add_test(NAME SerializationTests COMMAND SerializationTests)

	add_executable(SizeTests size_test.cxx)
	target_link_libraries(SizeTests StormByte)
	add_test(NAME SizeTests COMMAND SizeTests)

	add_executable(StringTests string_test.cxx)
	target_link_libraries(StringTests StormByte)
	add_test(NAME StringTests COMMAND StringTests)

	add_executable(WStringTests wstring_test.cxx)
	target_link_libraries(WStringTests StormByte)
	add_test(NAME WStringTests COMMAND WStringTests)

	add_executable(TestHandlersTests test_handlers_test.cxx)
	target_link_libraries(TestHandlersTests StormByte)
	add_test(NAME TestHandlersTests COMMAND TestHandlersTests)

	add_executable(TelemetryTests telemetry_test.cxx)
	target_link_libraries(TelemetryTests StormByte)
	add_test(NAME TelemetryTests COMMAND TelemetryTests)

	add_executable(ThreadLockTests thread_lock_test.cxx)
	target_link_libraries(ThreadLockTests StormByte)
	add_test(NAME ThreadLockTests COMMAND ThreadLockTests)

	add_executable(TypeTraitsTests type_traits_test.cxx)
	target_link_libraries(TypeTraitsTests StormByte)
	add_test(NAME TypeTraitsTests COMMAND TypeTraitsTests)

	add_executable(UUIDTest uuid_test.cxx)
	target_link_libraries(UUIDTest StormByte)
	add_test(NAME UUIDTest COMMAND UUIDTest)

	add_executable(WCStringTests wcstring_test.cxx)
	target_link_libraries(WCStringTests StormByte)
	add_test(NAME WCStringTests COMMAND WCStringTests)
endif()
