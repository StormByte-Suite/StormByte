cmake_minimum_required(VERSION 3.28)

set(WIN32 TRUE)
include("${STORMBYTE_CMAKE_DIR}/StormByteCommon.cmake")

if(DEFINED INVALID_LIBRARY)
	if(DEFINED INVALID_OVERRIDE)
		set(STORMBYTE_TEST_LIBRARY_IS_SHARED "${INVALID_OVERRIDE}")
		_stormbyte_library_is_shared("${INVALID_LIBRARY}" _shared STORMBYTE_TEST_LIBRARY_IS_SHARED)
	else()
		_stormbyte_library_is_shared("${INVALID_LIBRARY}" _shared)
	endif()
	return()
endif()

if(DEFINED SHARED_LIBRARY AND DEFINED STATIC_LIBRARY)
	_stormbyte_library_is_shared("${SHARED_LIBRARY}" _shared)
	if(NOT _shared)
		message(FATAL_ERROR "Shared import library was classified as static: ${SHARED_LIBRARY}")
	endif()
	_stormbyte_library_is_shared("${STATIC_LIBRARY}" _shared)
	if(_shared)
		message(FATAL_ERROR "Static library was classified as shared: ${STATIC_LIBRARY}")
	endif()
endif()

if(DEFINED MIXED_LIBRARY)
	_stormbyte_library_is_shared("${MIXED_LIBRARY}" _shared STORMBYTE_MIXED_LIBRARY_IS_SHARED)
	if(_shared)
		message(FATAL_ERROR "Static library with imported dependencies was classified as shared: ${MIXED_LIBRARY}")
	endif()
endif()

file(MAKE_DIRECTORY "${ARCHIVE_TEST_DIR}")
file(WRITE "${ARCHIVE_TEST_DIR}/short.lib" "not an archive")
string(REPEAT "x" 68 _malformed_header)
file(WRITE "${ARCHIVE_TEST_DIR}/malformed.lib" "${_malformed_header}")
file(WRITE "${ARCHIVE_TEST_DIR}/truncated.lib"
	"!<arch>\n/               0           0     0     0       5         `\na")
file(WRITE "${ARCHIVE_TEST_DIR}/bad-count.lib"
	"!<arch>\n/               0           0     0     0       4         `\nABCD")

set(_cases short malformed truncated bad-count)
if(STORMBYTE_COFF_FIXTURE_GENERATOR)
	foreach(_case missing-names unterminated)
		execute_process(
			COMMAND "${STORMBYTE_COFF_FIXTURE_GENERATOR}" "${ARCHIVE_TEST_DIR}/${_case}.lib" "--${_case}"
			RESULT_VARIABLE _result
			ERROR_VARIABLE _error
		)
		if(NOT _result STREQUAL "0")
			message(FATAL_ERROR "Could not create ${_case}.lib (${_result}):\n${_error}")
		endif()
		list(APPEND _cases "${_case}")
	endforeach()
endif()

foreach(_case IN LISTS _cases)
	if(_case STREQUAL "short")
		set(_expected "truncated archive header")
	elseif(_case STREQUAL "malformed")
		set(_expected "expected a COFF archive with a linker index")
	elseif(_case STREQUAL "bad-count")
		set(_expected "invalid linker index symbol count")
	elseif(_case STREQUAL "missing-names")
		set(_expected "missing linker index symbol names")
	elseif(_case STREQUAL "unterminated")
		set(_expected "unterminated linker index symbol names")
	else()
		set(_expected "truncated linker index")
	endif()
	execute_process(
		COMMAND "${CMAKE_COMMAND}"
			"-DSTORMBYTE_CMAKE_DIR=${STORMBYTE_CMAKE_DIR}"
			"-DINVALID_LIBRARY=${ARCHIVE_TEST_DIR}/${_case}.lib"
			-P "${CMAKE_CURRENT_LIST_FILE}"
		RESULT_VARIABLE _result
		OUTPUT_VARIABLE _output
		ERROR_VARIABLE _error
	)
	if(_result STREQUAL "0" OR NOT "${_output}${_error}" MATCHES "${_expected}")
		message(FATAL_ERROR "Incorrect diagnostic for ${_case}.lib (${_result}):\n${_output}\n${_error}")
	endif()
endforeach()

execute_process(
	COMMAND "${CMAKE_COMMAND}"
		"-DSTORMBYTE_CMAKE_DIR=${STORMBYTE_CMAKE_DIR}"
		"-DINVALID_LIBRARY=${ARCHIVE_TEST_DIR}/short.lib"
		-DINVALID_OVERRIDE=not-a-boolean
		-P "${CMAKE_CURRENT_LIST_FILE}"
	RESULT_VARIABLE _result
	OUTPUT_VARIABLE _output
	ERROR_VARIABLE _error
)
if(_result STREQUAL "0" OR NOT "${_output}${_error}" MATCHES "must be a boolean")
	message(FATAL_ERROR "Invalid linkage override was not rejected (${_result}):\n${_output}\n${_error}")
endif()
