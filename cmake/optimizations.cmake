# Optimizations
option(ENABLE_LTO "Link-time optimization. Not tied to the build type. Combines with ENABLE_PGO." ON)

if (CMAKE_BUILD_TYPE STREQUAL "Release" AND MSVC)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /arch:AVX2 /fp:fast /DNDEBUG")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /arch:AVX2 /fp:fast /DNDEBUG")
endif()

# Per-config variables win over the plain one, so set both.
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION "${ENABLE_LTO}")
foreach(_lto_cfg DEBUG RELEASE RELWITHDEBINFO MINSIZEREL)
	set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_${_lto_cfg} "${ENABLE_LTO}")
endforeach()
