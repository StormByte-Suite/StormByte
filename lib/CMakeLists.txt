include(GNUInstallDirs)

option(BUILD_SHARED_LIBS "Build shared libraries" ON)

file(GLOB_RECURSE STORMBYTE_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_LIST_DIR}/*.cxx")

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
	CXX_VISIBILITY_PRESET hidden
	VISIBILITY_INLINES_HIDDEN ON
)

if(WIN32)
	target_link_libraries(StormByte PRIVATE Bcrypt.lib)
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
)
