option(ENABLE_TEST "Enable Unit Tests" OFF)
if(ENABLE_TEST)
	enable_testing()

	add_executable(CMakeDiscoveryArchiveFixtureGenerator cmake/windows_archive_fixture_generator.cxx)
	foreach(_linkage Shared Static)
		add_test(NAME CMakeDiscovery${_linkage}
			COMMAND "${CMAKE_COMMAND}"
				-S "${CMAKE_CURRENT_SOURCE_DIR}/cmake"
				-B "${CMAKE_CURRENT_BINARY_DIR}/cmake-${_linkage}"
				"-DSTORMBYTE_CMAKE_DIR=${CMAKE_SOURCE_DIR}/cmake"
				"-DSTORMBYTE_COFF_FIXTURE_GENERATOR=$<TARGET_FILE:CMakeDiscoveryArchiveFixtureGenerator>"
				"-DSTORMBYTE_TEST_STATIC=$<STREQUAL:${_linkage},Static>"
		)
		foreach(_windows_toolchain MSVC MinGW)
			add_test(NAME CMakeDiscoveryWindows${_windows_toolchain}${_linkage}
				COMMAND "${CMAKE_COMMAND}"
					-S "${CMAKE_CURRENT_SOURCE_DIR}/cmake"
					-B "${CMAKE_CURRENT_BINARY_DIR}/cmake-Windows${_windows_toolchain}${_linkage}"
					"-DSTORMBYTE_CMAKE_DIR=${CMAKE_SOURCE_DIR}/cmake"
					"-DSTORMBYTE_COFF_FIXTURE_GENERATOR=$<TARGET_FILE:CMakeDiscoveryArchiveFixtureGenerator>"
					"-DSTORMBYTE_TEST_STATIC=$<STREQUAL:${_linkage},Static>"
					"-DSTORMBYTE_TEST_MINGW=$<STREQUAL:${_windows_toolchain},MinGW>"
					-DCMAKE_SYSTEM_NAME=Windows
			)
		endforeach()
		add_test(NAME CMakeDiscoveryWindowsMSVC${_linkage}CoreOverride
			COMMAND "${CMAKE_COMMAND}"
				-S "${CMAKE_CURRENT_SOURCE_DIR}/cmake"
				-B "${CMAKE_CURRENT_BINARY_DIR}/cmake-WindowsMSVC${_linkage}CoreOverride"
				"-DSTORMBYTE_CMAKE_DIR=${CMAKE_SOURCE_DIR}/cmake"
				"-DSTORMBYTE_COFF_FIXTURE_GENERATOR=$<TARGET_FILE:CMakeDiscoveryArchiveFixtureGenerator>"
				"-DSTORMBYTE_TEST_STATIC=$<STREQUAL:${_linkage},Static>"
				"-DSTORMBYTE_TEST_CORE_OVERRIDE=$<STREQUAL:${_linkage},Static>"
				-DCMAKE_SYSTEM_NAME=Windows
		)
	endforeach()
	add_test(NAME CMakeDiscoveryWindowsInvalidArchives
		COMMAND "${CMAKE_COMMAND}"
			"-DSTORMBYTE_CMAKE_DIR=${CMAKE_SOURCE_DIR}/cmake"
			"-DSTORMBYTE_COFF_FIXTURE_GENERATOR=$<TARGET_FILE:CMakeDiscoveryArchiveFixtureGenerator>"
			"-DARCHIVE_TEST_DIR=${CMAKE_CURRENT_BINARY_DIR}/cmake-WindowsInvalidArchives"
			-P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/windows_archive_test.cmake"
	)
	if(WIN32)
		add_library(CMakeDiscoveryStaticFixture STATIC cmake/windows_static_fixture.cxx)
		add_test(NAME CMakeDiscoveryWindowsNativeArchives
			COMMAND "${CMAKE_COMMAND}"
				"-DSTORMBYTE_CMAKE_DIR=${CMAKE_SOURCE_DIR}/cmake"
				"-DSTORMBYTE_COFF_FIXTURE_GENERATOR=$<TARGET_FILE:CMakeDiscoveryArchiveFixtureGenerator>"
				"-DSHARED_LIBRARY=$<TARGET_LINKER_FILE:DLLBoundaryFixture>"
				"-DSTATIC_LIBRARY=$<TARGET_LINKER_FILE:CMakeDiscoveryStaticFixture>"
				"-DARCHIVE_TEST_DIR=${CMAKE_CURRENT_BINARY_DIR}/cmake-WindowsNativeArchives"
				-P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/windows_archive_test.cmake"
		)
	endif()

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

	add_executable(TelemetryAllocationFailureTests telemetry_allocation_failure_test.cxx)
	target_link_libraries(TelemetryAllocationFailureTests StormByte)
	add_test(NAME TelemetryAllocationFailureTests COMMAND TelemetryAllocationFailureTests)
	set_tests_properties(TelemetryAllocationFailureTests PROPERTIES SKIP_RETURN_CODE 77 TIMEOUT 20)

	add_executable(ThreadLockTests thread_lock_test.cxx)
	target_link_libraries(ThreadLockTests StormByte)
	add_test(NAME ThreadLockTests COMMAND ThreadLockTests)

	add_executable(UUIDTest uuid_test.cxx)
	target_link_libraries(UUIDTest StormByte)
	add_test(NAME UUIDTest COMMAND UUIDTest)

	add_subdirectory(safe)
	add_subdirectory(type_traits)

endif()
