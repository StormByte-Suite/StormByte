include(GNUInstallDirs)

option(BUILD_SHARED_LIBS "Build shared libraries" ON)
option(STORMBYTE_USE_SYSTEM_SDS "Use the system-installed SDS library" OFF)

file(GLOB_RECURSE STORMBYTE_SOURCES CONFIGURE_DEPENDS
	"${CMAKE_CURRENT_LIST_DIR}/*.cxx"
	"${CMAKE_CURRENT_LIST_DIR}/*.c"
)

if(STORMBYTE_USE_SYSTEM_SDS)
	find_package(PkgConfig REQUIRED)
	pkg_check_modules(SDS REQUIRED IMPORTED_TARGET sds>=2.0.0)
else()
	list(APPEND STORMBYTE_SOURCES "${CMAKE_SOURCE_DIR}/thirdparty/sds/sds.c")
endif()

if(BUILD_SHARED_LIBS)
	add_library(StormByte SHARED ${STORMBYTE_SOURCES})
	target_compile_definitions(StormByte PUBLIC STORMBYTE_SHARED)
else()
	add_library(StormByte STATIC ${STORMBYTE_SOURCES})
endif()

if(STORMBYTE_USE_SYSTEM_SDS)
	target_link_libraries(StormByte PRIVATE PkgConfig::SDS)
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
	target_compile_options(StormByte PRIVATE /EHsc)
	target_compile_definitions(StormByte PUBLIC UNICODE NOMINMAX)
else()
	target_compile_options(StormByte PRIVATE
		$<$<CONFIG:Debug>:-pipe -g -ggdb -Wall -Wextra -Wnon-virtual-dtor -pedantic -pedantic-errors>
	)
endif()

target_include_directories(StormByte
	SYSTEM BEFORE PUBLIC "${CMAKE_CURRENT_LIST_DIR}/public"
	PRIVATE "${CMAKE_CURRENT_LIST_DIR}/private"
)

if(NOT STORMBYTE_USE_SYSTEM_SDS)
	target_include_directories(StormByte PRIVATE "${CMAKE_SOURCE_DIR}/thirdparty/sds")
endif()
