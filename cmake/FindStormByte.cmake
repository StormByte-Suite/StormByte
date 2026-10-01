# FindStormByte.cmake
# Finder for the StormByte core library and its optional modules.
# Linux only (Windows builds consume the in-tree target).
#
# Usage:
#   find_package(StormByte REQUIRED)
#   find_package(StormByte REQUIRED COMPONENTS Logger)
#   find_package(StormByte COMPONENTS Crypto)   # pulls Buffer, then Logger and System
#
# Provides:
#   - Imported targets: StormByte and StormByte::<Component>
#   - Variables: STORMBYTE_FOUND, STORMBYTE_INCLUDE_DIR, STORMBYTE_LIBRARY
#                StormByte_<Component>_FOUND, STORMBYTE_<Component>_LIBRARY
#
# A shared install defines STORMBYTE_SHARED on StormByte and
# STORMBYTE_<COMPONENT>_SHARED on each module target (INTERFACE).
# A static .a defines nothing; headers then take the empty-macro branch.

include(FindPackageHandleStandardArgs)
include(FeatureSummary)

# ----------------------------------------------------------------------
# User-editable section
# ----------------------------------------------------------------------

set(_STORMBYTE_AVAILABLE_COMPONENTS
	Buffer
	Config
	Crypto
	Database
	Logger
	Multimedia
	Network
	System
)

# Transitive dependencies (component → components it PUBLIC-links).
# Every component already links StormByte (the core). Config needs only that.
# Taken from each module's buildmaster_link.
set(_STORMBYTE_COMPONENT_DEPS_Buffer     "Logger" "System")
set(_STORMBYTE_COMPONENT_DEPS_Crypto     "Buffer")
set(_STORMBYTE_COMPONENT_DEPS_Database   "Logger")
set(_STORMBYTE_COMPONENT_DEPS_Multimedia "Buffer")
set(_STORMBYTE_COMPONENT_DEPS_Network    "Buffer")

# ----------------------------------------------------------------------
# Internal helpers
# ----------------------------------------------------------------------

function(_stormbyte_expand_dependencies components_var)
	set(_result ${${components_var}})
	set(_changed TRUE)

	while(_changed)
		set(_changed FALSE)
		foreach(_comp IN LISTS _result)
			if(DEFINED _STORMBYTE_COMPONENT_DEPS_${_comp})
				foreach(_dep IN LISTS _STORMBYTE_COMPONENT_DEPS_${_comp})
					if(NOT _dep IN_LIST _result)
						list(APPEND _result ${_dep})
						set(_changed TRUE)
					endif()
				endforeach()
			endif()
		endforeach()
	endwhile()

	list(REMOVE_DUPLICATES _result)
	set(${components_var} ${_result} PARENT_SCOPE)
endfunction()

# True when the on-disk artifact is a shared object (.so, .so.N, …).
function(_stormbyte_library_is_shared lib_path out_var)
	if(NOT lib_path)
		set(${out_var} FALSE PARENT_SCOPE)
		return()
	endif()
	get_filename_component(_name "${lib_path}" NAME)
	if(_name MATCHES "\\.so($|\\.)")
		set(${out_var} TRUE PARENT_SCOPE)
	else()
		set(${out_var} FALSE PARENT_SCOPE)
	endif()
endfunction()

# ----------------------------------------------------------------------
# Find core library + headers
# ----------------------------------------------------------------------

find_path(STORMBYTE_INCLUDE_DIR
	NAMES visibility.h
	PATH_SUFFIXES StormByte
	PATHS
		${CMAKE_PREFIX_PATH}
		/usr/include
		/usr/local/include
)

find_library(STORMBYTE_LIBRARY
	NAMES StormByte
	PATH_SUFFIXES lib lib64
	PATHS
		${CMAKE_PREFIX_PATH}
		/usr/lib /usr/lib64
		/usr/local/lib /usr/local/lib64
)

if(STORMBYTE_LIBRARY AND NOT STORMBYTE_INCLUDE_DIR)
	get_filename_component(_libdir "${STORMBYTE_LIBRARY}" DIRECTORY)
	foreach(_cand IN ITEMS
			"${_libdir}/../include"
			"${_libdir}/../../include"
			"${_libdir}/include")
		if(EXISTS "${_cand}/StormByte/visibility.h")
			set(STORMBYTE_INCLUDE_DIR "${_cand}")
			break()
		endif()
	endforeach()
endif()

# ----------------------------------------------------------------------
# Process requested components + transitive deps
# ----------------------------------------------------------------------

set(_requested_components)
foreach(_component IN LISTS StormByte_FIND_COMPONENTS)
	if(_component IN_LIST _STORMBYTE_AVAILABLE_COMPONENTS)
		list(APPEND _requested_components ${_component})
	else()
		set(StormByte_${_component}_FOUND FALSE)
	endif()
endforeach()
_stormbyte_expand_dependencies(_requested_components)

# ----------------------------------------------------------------------
# Create core target
# ----------------------------------------------------------------------

if(STORMBYTE_LIBRARY)
	if(NOT TARGET StormByte)
		add_library(StormByte UNKNOWN IMPORTED GLOBAL)
		set_target_properties(StormByte PROPERTIES
			IMPORTED_LOCATION "${STORMBYTE_LIBRARY}"
		)
		if(STORMBYTE_INCLUDE_DIR)
			set_target_properties(StormByte PROPERTIES
				INTERFACE_INCLUDE_DIRECTORIES "${STORMBYTE_INCLUDE_DIR}"
			)
		endif()
		_stormbyte_library_is_shared("${STORMBYTE_LIBRARY}" _sb_core_shared)
		if(_sb_core_shared)
			target_compile_definitions(StormByte INTERFACE STORMBYTE_SHARED)
		endif()
	endif()
endif()

# ----------------------------------------------------------------------
# Find and create component targets
# ----------------------------------------------------------------------

set(_found_components)
set(_missing_components)

foreach(component IN LISTS _requested_components)
	string(TOLOWER "${component}" _comp_lower)
	string(TOUPPER "${component}" _comp_upper)

	set(StormByte_${component}_FOUND FALSE)
	unset(STORMBYTE_${component}_LIBRARY CACHE)

	if(STORMBYTE_INCLUDE_DIR)
		set(_possible_headers
			"${STORMBYTE_INCLUDE_DIR}/StormByte/${_comp_lower}/visibility.h"
			"${STORMBYTE_INCLUDE_DIR}/${_comp_lower}/visibility.h"
			"${STORMBYTE_INCLUDE_DIR}/StormByte/${component}/visibility.h"
			"${STORMBYTE_INCLUDE_DIR}/${component}/visibility.h"
		)
		foreach(_hdr IN LISTS _possible_headers)
			if(EXISTS "${_hdr}")
				set(StormByte_${component}_FOUND TRUE)
				break()
			endif()
		endforeach()
	endif()

	find_library(STORMBYTE_${component}_LIBRARY
		NAMES
			StormByte-${component}
			StormByte_${component}
			stormbyte-${_comp_lower}
			stormbyte_${_comp_lower}
		PATH_SUFFIXES lib lib64
		PATHS
			${CMAKE_PREFIX_PATH}
			/usr/lib /usr/lib64
			/usr/local/lib /usr/local/lib64
	)

	if(STORMBYTE_${component}_LIBRARY)
		set(StormByte_${component}_FOUND TRUE)
	endif()

	if(StormByte_${component}_FOUND)
		if(NOT TARGET StormByte::${component})
			add_library(StormByte-${component} UNKNOWN IMPORTED GLOBAL)
			add_library(StormByte::${component} ALIAS StormByte-${component})

			if(STORMBYTE_${component}_LIBRARY)
				set_target_properties(StormByte-${component} PROPERTIES
					IMPORTED_LOCATION "${STORMBYTE_${component}_LIBRARY}"
				)
			endif()

			if(STORMBYTE_INCLUDE_DIR)
				set_target_properties(StormByte-${component} PROPERTIES
					INTERFACE_INCLUDE_DIRECTORIES "${STORMBYTE_INCLUDE_DIR}"
				)
			endif()

			_stormbyte_library_is_shared("${STORMBYTE_${component}_LIBRARY}" _sb_comp_shared)
			if(_sb_comp_shared)
				target_compile_definitions(StormByte-${component} INTERFACE
					STORMBYTE_${_comp_upper}_SHARED
				)
			endif()
		endif()

		list(APPEND _found_components ${component})
	else()
		list(APPEND _missing_components ${component})
	endif()

	mark_as_advanced(STORMBYTE_${component}_LIBRARY)
endforeach()

# ----------------------------------------------------------------------
# Wire transitive INTERFACE_LINK_LIBRARIES
# ----------------------------------------------------------------------

foreach(component IN LISTS _found_components)
	set(_link_libs StormByte)

	if(DEFINED _STORMBYTE_COMPONENT_DEPS_${component})
		foreach(dep IN LISTS _STORMBYTE_COMPONENT_DEPS_${component})
			if(TARGET StormByte::${dep})
				list(APPEND _link_libs StormByte::${dep})
			endif()
		endforeach()
	endif()

	set_target_properties(StormByte-${component} PROPERTIES
		INTERFACE_LINK_LIBRARIES "${_link_libs}"
	)
endforeach()

# ----------------------------------------------------------------------
# Final result handling
# ----------------------------------------------------------------------

set(STORMBYTE_FOUND TRUE)
if(NOT STORMBYTE_LIBRARY)
	set(STORMBYTE_FOUND FALSE)
endif()

find_package_handle_standard_args(StormByte
	REQUIRED_VARS STORMBYTE_LIBRARY STORMBYTE_INCLUDE_DIR
	HANDLE_COMPONENTS
)

mark_as_advanced(STORMBYTE_INCLUDE_DIR STORMBYTE_LIBRARY)
