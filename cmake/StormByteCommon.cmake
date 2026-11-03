function(_stormbyte_library_is_shared library out)
	if(library MATCHES "\\.so($|\\.)|\\.dylib$")
		set(${out} TRUE PARENT_SCOPE)
	else()
		set(${out} FALSE PARENT_SCOPE)
	endif()
endfunction()

function(_stormbyte_load_component component)
	set(StormByte_${component}_FOUND FALSE)
	if(TARGET StormByte::${component})
		set(StormByte_${component}_FOUND TRUE PARENT_SCOPE)
		return()
	endif()
	if(component STREQUAL "Base" OR NOT TARGET StormByte)
		set(StormByte_${component}_FOUND FALSE PARENT_SCOPE)
		return()
	endif()
	if(component IN_LIST _STORMBYTE_COMPONENT_STACK)
		message(FATAL_ERROR "StormByte component dependency cycle: ${_STORMBYTE_COMPONENT_STACK};${component}")
	endif()
	list(APPEND _STORMBYTE_COMPONENT_STACK "${component}")

	set(_config "_config-NOTFOUND")
	find_file(_config
		NAMES "FindStormByte${component}.cmake"
		HINTS "${CMAKE_CURRENT_FUNCTION_LIST_DIR}" ${CMAKE_MODULE_PATH}
		PATHS ${CMAKE_PREFIX_PATH}
		PATH_SUFFIXES share/cmake/Modules
		NO_CACHE
	)
	if(_config)
		include("${_config}")
	endif()
	set(StormByte_${component}_FOUND "${StormByte_${component}_FOUND}" PARENT_SCOPE)
endfunction()

function(stormbyte_find_component component)
	cmake_parse_arguments(PARSE_ARGV 1 SB "" "" "DEPENDS")
	if(SB_UNPARSED_ARGUMENTS OR SB_KEYWORDS_MISSING_VALUES)
		message(FATAL_ERROR "Invalid stormbyte_find_component(${component}) arguments")
	endif()
	set(StormByte_${component}_FOUND FALSE PARENT_SCOPE)
	if(NOT TARGET StormByte)
		return()
	endif()

	set(_dependencies_found TRUE)
	set(_link_libraries StormByte)
	foreach(_dependency IN LISTS SB_DEPENDS)
		_stormbyte_load_component("${_dependency}")
		if(StormByte_${_dependency}_FOUND)
			list(APPEND _link_libraries StormByte::${_dependency})
		else()
			set(_dependencies_found FALSE)
		endif()
	endforeach()

	string(TOLOWER "${component}" _lower)
	string(TOUPPER "${component}" _upper)
	find_path(STORMBYTE_${component}_INCLUDE_DIR
		NAMES "StormByte/${_lower}/visibility.h" "StormByte/${component}/visibility.h"
		HINTS "${STORMBYTE_INCLUDE_DIR}"
		PATH_SUFFIXES include
	)
	find_library(STORMBYTE_${component}_LIBRARY
		NAMES "StormByte-${component}" "StormByte_${component}" "stormbyte-${_lower}" "stormbyte_${_lower}"
		PATH_SUFFIXES lib lib64
	)
	mark_as_advanced(STORMBYTE_${component}_INCLUDE_DIR STORMBYTE_${component}_LIBRARY)

	if(NOT _dependencies_found OR NOT STORMBYTE_${component}_INCLUDE_DIR OR NOT STORMBYTE_${component}_LIBRARY)
		return()
	endif()

	if(NOT TARGET StormByte-${component})
		add_library(StormByte-${component} UNKNOWN IMPORTED GLOBAL)
		set_target_properties(StormByte-${component} PROPERTIES
			IMPORTED_LOCATION "${STORMBYTE_${component}_LIBRARY}"
			INTERFACE_INCLUDE_DIRECTORIES "${STORMBYTE_${component}_INCLUDE_DIR}"
			INTERFACE_LINK_LIBRARIES "${_link_libraries}"
		)
		_stormbyte_library_is_shared("${STORMBYTE_${component}_LIBRARY}" _shared)
		if(_shared)
			target_compile_definitions(StormByte-${component} INTERFACE STORMBYTE_${_upper}_SHARED)
		endif()
	endif()
	if(NOT TARGET StormByte::${component})
		add_library(StormByte::${component} ALIAS StormByte-${component})
	endif()
	set(StormByte_${component}_FOUND TRUE PARENT_SCOPE)
endfunction()
