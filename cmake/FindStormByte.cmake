# Find the core and delegate optional components to their installed finders.
# Usage: find_package(StormByte REQUIRED COMPONENTS Base Network)
include("${CMAKE_CURRENT_LIST_DIR}/StormByteCommon.cmake")
include(FindPackageHandleStandardArgs)

find_path(STORMBYTE_INCLUDE_DIR
	NAMES StormByte/visibility.h
	PATH_SUFFIXES include
)
find_library(STORMBYTE_LIBRARY
	NAMES StormByte
	PATH_SUFFIXES lib lib64
)

if(STORMBYTE_LIBRARY AND STORMBYTE_INCLUDE_DIR AND NOT TARGET StormByte)
	include("${CMAKE_CURRENT_LIST_DIR}/StormByteCoreDependencies.cmake" OPTIONAL)
	add_library(StormByte UNKNOWN IMPORTED GLOBAL)
	set_target_properties(StormByte PROPERTIES
		IMPORTED_LOCATION "${STORMBYTE_LIBRARY}"
		INTERFACE_INCLUDE_DIRECTORIES "${STORMBYTE_INCLUDE_DIR}"
	)
	_stormbyte_library_is_shared("${STORMBYTE_LIBRARY}" _sb_core_shared)
	if(_sb_core_shared)
		target_compile_definitions(StormByte INTERFACE STORMBYTE_SHARED)
	endif()
	if(STORMBYTE_CORE_EXTERNAL_LIBRARIES)
		target_link_libraries(StormByte INTERFACE ${STORMBYTE_CORE_EXTERNAL_LIBRARIES})
	endif()
endif()

if(TARGET StormByte)
	if(NOT TARGET StormByte-Base)
		add_library(StormByte-Base ALIAS StormByte)
	endif()
	if(NOT TARGET StormByte::Base)
		add_library(StormByte::Base ALIAS StormByte)
	endif()
endif()

foreach(_sb_component IN LISTS StormByte_FIND_COMPONENTS)
	_stormbyte_load_component("${_sb_component}")
endforeach()

find_package_handle_standard_args(StormByte
	REQUIRED_VARS STORMBYTE_LIBRARY STORMBYTE_INCLUDE_DIR
	HANDLE_COMPONENTS
)
set(STORMBYTE_FOUND "${StormByte_FOUND}")
mark_as_advanced(STORMBYTE_INCLUDE_DIR STORMBYTE_LIBRARY)
