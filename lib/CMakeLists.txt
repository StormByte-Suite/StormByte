include(GNUInstallDirs)

option(BUILD_SHARED_LIBS "Build shared libraries" ON)

file(GLOB_RECURSE STORMBYTE_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_LIST_DIR}/*.cxx" "${CMAKE_CURRENT_LIST_DIR}/*.c" "${CMAKE_SOURCE_DIR}/thirdparty/sds/*.c")

if(BUILD_SHARED_LIBS)
	add_library(StormByte SHARED ${STORMBYTE_SOURCES})
	target_compile_definitions(StormByte PUBLIC STORMBYTE_SHARED)
else()
	add_library(StormByte STATIC ${STORMBYTE_SOURCES})
endif()

set_target_properties(StormByte PROPERTIES
	LINKER_LANGUAGE CXX
	SOVERSION ${CMAKE_PROJECT_VERSION}
	VERSION ${CMAKE_PROJECT_VERSION}
	POSITION_INDEPENDENT_CODE ON
	C_VISIBILITY_PRESET hidden
	CXX_VISIBILITY_PRESET hidden
	VISIBILITY_INLINES_HIDDEN ON
)

if(WIN32)
	target_link_libraries(StormByte PRIVATE Bcrypt.lib)
else()
	# Zeroing primitives the optimizer may not elide, used by Safe::Binary::Wipe.
	# Probed rather than keyed on libc macros: musl defines none.
	include(CheckCXXSymbolExists)
	check_cxx_symbol_exists(explicit_bzero "string.h;strings.h" STORMBYTE_HAVE_EXPLICIT_BZERO)
	check_cxx_symbol_exists(explicit_memset "string.h" STORMBYTE_HAVE_EXPLICIT_MEMSET)
	set(CMAKE_REQUIRED_DEFINITIONS -D__STDC_WANT_LIB_EXT1__=1)
	check_cxx_symbol_exists(memset_s "string.h" STORMBYTE_HAVE_MEMSET_S)
	unset(CMAKE_REQUIRED_DEFINITIONS)
	foreach(primitive EXPLICIT_BZERO EXPLICIT_MEMSET MEMSET_S)
		if(STORMBYTE_HAVE_${primitive})
			target_compile_definitions(StormByte PRIVATE STORMBYTE_HAVE_${primitive})
		endif()
	endforeach()
endif()

if(MSVC)
	target_compile_options(StormByte PRIVATE
		/EHsc
		$<$<CONFIG:Release>:/O2>
		$<$<CONFIG:Debug>:/Od>
	)
	target_compile_definitions(StormByte PUBLIC UNICODE NOMINMAX)
else()
	target_compile_options(StormByte PRIVATE
		$<$<CONFIG:Debug>:-pipe -g -ggdb -Wall -Wextra -Wnon-virtual-dtor -pedantic -pedantic-errors -O0>
	)
endif()

target_include_directories(StormByte
	SYSTEM BEFORE PUBLIC "${CMAKE_CURRENT_LIST_DIR}/public"
	PRIVATE "${CMAKE_CURRENT_LIST_DIR}/private"
	PRIVATE "${CMAKE_SOURCE_DIR}/thirdparty/sds"
)
