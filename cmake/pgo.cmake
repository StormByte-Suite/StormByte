# Profile-guided optimization, as one include.
#
# include() this file once, after project() and before any targets or install().
# -DENABLE_PGO=ON changes the default build (the all target):
#   1. Compile with tests, instrumented.
#   2. Run ctest in <build>/test (override with PGO_TEST_DIR).
#   3. Recompile without tests, using that profile.
# cmake --install installs step 3, not the instrumented binaries.
#
# The training build forces the cache option named by PGO_TEST_OPTION
# (default ENABLE_TEST) ON. The optimized rebuild forces it OFF.
# Set PGO_FORWARD_CACHE before including this file to a list of extra cache
# variables the optimized rebuild must receive.
#
# Copy this file into another project and include it the same way.
# GCC, Clang, clang-cl and MSVC are supported. Release is the useful config.

option(ENABLE_PGO "Instrument, train with ctest, rebuild with the profile, install that build" OFF)

if(NOT ENABLE_PGO)
	return()
endif()

if(NOT PGO_TEST_OPTION)
	set(PGO_TEST_OPTION ENABLE_TEST)
endif()
if(NOT DEFINED PGO_FORWARD_CACHE)
	set(PGO_FORWARD_CACHE BUILD_SHARED_LIBS ENABLE_DOC)
endif()
if(NOT PGO_PROFILE_DIR)
	set(PGO_PROFILE_DIR "${CMAKE_BINARY_DIR}/pgo-data")
endif()
if(NOT PGO_TEST_DIR)
	set(PGO_TEST_DIR "${CMAKE_BINARY_DIR}/test")
endif()
file(MAKE_DIRECTORY "${PGO_PROFILE_DIR}")

if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
	set(_pgo_kind MSVC)
elseif(CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
	set(_pgo_kind CLANG_CL)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
	set(_pgo_kind CLANG)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	set(_pgo_kind GNU)
else()
	message(FATAL_ERROR "ENABLE_PGO does not support the ${CMAKE_CXX_COMPILER_ID} compiler")
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
	message(WARNING "ENABLE_PGO on a Debug build: the profile will not match an optimized binary")
endif()

# MSVC PGO is LTCG. CMake's own /LTCG would override /GENPROFILE, so own both flags.
if(_pgo_kind STREQUAL "MSVC")
	set(CMAKE_INTERPROCEDURAL_OPTIMIZATION OFF)
	foreach(_pgo_cfg RELEASE RELWITHDEBINFO MINSIZEREL DEBUG)
		set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_${_pgo_cfg} OFF)
	endforeach()
endif()

if(PGO_STAGE STREQUAL "use")
	set(${PGO_TEST_OPTION} OFF CACHE BOOL "Disabled in the PGO optimized rebuild" FORCE)
	if(_pgo_kind STREQUAL "GNU")
		add_compile_options(
			"SHELL:-fprofile-use=${PGO_PROFILE_DIR}"
			"SHELL:-fprofile-correction"
			"SHELL:-fprofile-prefix-map=${CMAKE_BINARY_DIR}=pgo"
			"SHELL:-Wno-error=missing-profile"
		)
		add_link_options("SHELL:-fprofile-use=${PGO_PROFILE_DIR}")
	elseif(_pgo_kind STREQUAL "CLANG")
		add_compile_options(
			"SHELL:-fprofile-use=${PGO_PROFILE_DIR}/default.profdata"
			"SHELL:-Wno-profile-instr-unprofiled"
			"SHELL:-Wno-profile-instr-out-of-date"
		)
		add_link_options("SHELL:-fprofile-use=${PGO_PROFILE_DIR}/default.profdata")
	elseif(_pgo_kind STREQUAL "CLANG_CL")
		add_compile_options(
			"SHELL:/clang:-fprofile-use=${PGO_PROFILE_DIR}/default.profdata"
			"SHELL:/clang:-Wno-profile-instr-unprofiled"
			"SHELL:/clang:-Wno-profile-instr-out-of-date"
		)
		add_link_options("SHELL:/clang:-fprofile-use=${PGO_PROFILE_DIR}/default.profdata")
	else()
		add_compile_options(/GL)
		add_link_options(/USEPROFILE)
	endif()
	message(STATUS "ENABLE_PGO: optimized rebuild, profile ${PGO_PROFILE_DIR}")
	return()
endif()

set(${PGO_TEST_OPTION} ON CACHE BOOL "Forced ON by the PGO training build" FORCE)
# Project install() rules would ship the instrumented binaries. The deferred
# hook turns this off and installs the optimized rebuild instead.
set(CMAKE_SKIP_INSTALL_RULES TRUE)

if(_pgo_kind STREQUAL "GNU")
	add_compile_options(
		"SHELL:-fprofile-generate=${PGO_PROFILE_DIR}"
		"SHELL:-fprofile-update=atomic"
		"SHELL:-fprofile-prefix-map=${CMAKE_BINARY_DIR}=pgo"
	)
	add_link_options("SHELL:-fprofile-generate=${PGO_PROFILE_DIR}")
elseif(_pgo_kind STREQUAL "CLANG")
	add_compile_options("SHELL:-fprofile-generate=${PGO_PROFILE_DIR}")
	add_link_options("SHELL:-fprofile-generate=${PGO_PROFILE_DIR}")
elseif(_pgo_kind STREQUAL "CLANG_CL")
	add_compile_options("SHELL:/clang:-fprofile-generate=${PGO_PROFILE_DIR}")
	add_link_options("SHELL:/clang:-fprofile-generate=${PGO_PROFILE_DIR}")
else()
	add_compile_options(/GL)
	add_link_options(/GENPROFILE)
endif()

message(STATUS "ENABLE_PGO: instrumented build, ctest in ${PGO_TEST_DIR}, then optimized rebuild")

function(_pgo_collect_targets dir out)
	set(found)
	get_property(targets DIRECTORY "${dir}" PROPERTY BUILDSYSTEM_TARGETS)
	foreach(target IN LISTS targets)
		get_property(excluded TARGET "${target}" PROPERTY EXCLUDE_FROM_ALL)
		if(NOT excluded)
			list(APPEND found "${target}")
		endif()
	endforeach()
	get_property(subs DIRECTORY "${dir}" PROPERTY SUBDIRECTORIES)
	foreach(sub IN LISTS subs)
		_pgo_collect_targets("${sub}" nested)
		list(APPEND found ${nested})
	endforeach()
	set(${out} "${found}" PARENT_SCOPE)
endfunction()

function(pgo_attach_training)
	set(CMAKE_SKIP_INSTALL_RULES FALSE)
	set(CMAKE_SKIP_INSTALL_RULES FALSE PARENT_SCOPE)
	_pgo_collect_targets("${CMAKE_SOURCE_DIR}" _pgo_deps)

	set(_pgo_profdata "")
	if(_pgo_kind STREQUAL "CLANG" OR _pgo_kind STREQUAL "CLANG_CL")
		get_filename_component(_pgo_bindir "${CMAKE_CXX_COMPILER}" DIRECTORY)
		find_program(PGO_LLVM_PROFDATA NAMES llvm-profdata llvm-profdata.exe HINTS "${_pgo_bindir}" REQUIRED)
		set(_pgo_profdata "${PGO_LLVM_PROFDATA}")
	endif()

	set(USE_DIR "${CMAKE_BINARY_DIR}/pgo-use")
	set(SOURCE_DIR "${CMAKE_SOURCE_DIR}")
	set(BUILD_DIR "${CMAKE_BINARY_DIR}")
	set(PROFILE_DIR "${PGO_PROFILE_DIR}")
	set(TEST_DIR "${PGO_TEST_DIR}")
	set(TEST_OPTION "${PGO_TEST_OPTION}")
	set(KIND "${_pgo_kind}")
	set(GENERATOR "${CMAKE_GENERATOR}")
	set(GEN_PLATFORM "${CMAKE_GENERATOR_PLATFORM}")
	set(GEN_TOOLSET "${CMAKE_GENERATOR_TOOLSET}")
	set(TOOLCHAIN "${CMAKE_TOOLCHAIN_FILE}")
	set(C_COMPILER "${CMAKE_C_COMPILER}")
	set(CXX_COMPILER "${CMAKE_CXX_COMPILER}")
	set(C_LAUNCHER "${CMAKE_C_COMPILER_LAUNCHER}")
	set(CXX_LAUNCHER "${CMAKE_CXX_COMPILER_LAUNCHER}")
	set(CTEST "${CMAKE_CTEST_COMMAND}")
	set(PROFDATA "${_pgo_profdata}")
	if(CMAKE_CONFIGURATION_TYPES)
		set(MULTI 1)
	else()
		set(MULTI 0)
	endif()
	set(BUILD_TYPE "${CMAKE_BUILD_TYPE}")
	set(FORWARD "")
	foreach(name IN LISTS PGO_FORWARD_CACHE)
		if(DEFINED ${name})
			string(APPEND FORWARD "list(APPEND _cfg \"-D${name}=${${name}}\")\n")
		endif()
	endforeach()

	string(CONFIGURE [=[
set(KIND "@KIND@")
if(NOT EXISTS "@TEST_DIR@/CTestTestfile.cmake")
	message(FATAL_ERROR "ENABLE_PGO: no tests in @TEST_DIR@. Set PGO_TEST_OPTION to the cache option that enables them.")
endif()
set(_ctest "@CTEST@" --output-on-failure)
if(@MULTI@ AND NOT CONFIG STREQUAL "")
	list(APPEND _ctest -C "${CONFIG}")
endif()
message(STATUS "ENABLE_PGO: ctest in @TEST_DIR@")
execute_process(COMMAND ${_ctest} WORKING_DIRECTORY "@TEST_DIR@" RESULT_VARIABLE _rc)
if(NOT _rc EQUAL 0)
	message(FATAL_ERROR "ENABLE_PGO: ctest failed (${_rc})")
endif()
if(KIND STREQUAL "CLANG" OR KIND STREQUAL "CLANG_CL")
	file(GLOB _raws "@PROFILE_DIR@/*.profraw")
	if(NOT _raws)
		message(FATAL_ERROR "ENABLE_PGO: ctest wrote no .profraw files in @PROFILE_DIR@")
	endif()
	execute_process(
		COMMAND "@PROFDATA@" merge -sparse "-output=@PROFILE_DIR@/default.profdata" ${_raws}
		RESULT_VARIABLE _rc
	)
	if(NOT _rc EQUAL 0)
		message(FATAL_ERROR "ENABLE_PGO: llvm-profdata merge failed (${_rc})")
	endif()
endif()
set(_cfg "@CMAKE_COMMAND@" -S "@SOURCE_DIR@" -B "@USE_DIR@" -G "@GENERATOR@")
if(NOT "@GEN_PLATFORM@" STREQUAL "")
	list(APPEND _cfg -A "@GEN_PLATFORM@")
endif()
if(NOT "@GEN_TOOLSET@" STREQUAL "")
	list(APPEND _cfg -T "@GEN_TOOLSET@")
endif()
if(NOT "@TOOLCHAIN@" STREQUAL "")
	list(APPEND _cfg "-DCMAKE_TOOLCHAIN_FILE=@TOOLCHAIN@")
endif()
if(NOT @MULTI@)
	list(APPEND _cfg "-DCMAKE_BUILD_TYPE=@BUILD_TYPE@")
endif()
list(APPEND _cfg
	"-DCMAKE_C_COMPILER=@C_COMPILER@"
	"-DCMAKE_CXX_COMPILER=@CXX_COMPILER@"
	"-DENABLE_PGO=ON"
	"-DPGO_STAGE=use"
	"-DPGO_PROFILE_DIR=@PROFILE_DIR@"
	"-DPGO_TEST_OPTION=@TEST_OPTION@"
	"-D@TEST_OPTION@=OFF"
)
if(NOT "@C_LAUNCHER@" STREQUAL "")
	list(APPEND _cfg "-DCMAKE_C_COMPILER_LAUNCHER=@C_LAUNCHER@")
endif()
if(NOT "@CXX_LAUNCHER@" STREQUAL "")
	list(APPEND _cfg "-DCMAKE_CXX_COMPILER_LAUNCHER=@CXX_LAUNCHER@")
endif()
@FORWARD@
message(STATUS "ENABLE_PGO: configuring optimized rebuild @USE_DIR@")
execute_process(COMMAND ${_cfg} RESULT_VARIABLE _rc)
if(NOT _rc EQUAL 0)
	message(FATAL_ERROR "ENABLE_PGO: optimized configure failed (${_rc})")
endif()
if(KIND STREQUAL "MSVC")
	file(GLOB_RECURSE _profs "@BUILD_DIR@/*.pgd" "@BUILD_DIR@/*.pgc")
	foreach(_src IN LISTS _profs)
		if(_src MATCHES "/pgo-use/")
			continue()
		endif()
		file(RELATIVE_PATH _rel "@BUILD_DIR@" "${_src}")
		get_filename_component(_dst_dir "@USE_DIR@/${_rel}" DIRECTORY)
		file(MAKE_DIRECTORY "${_dst_dir}")
		file(COPY "${_src}" DESTINATION "${_dst_dir}")
	endforeach()
endif()
set(_build "@CMAKE_COMMAND@" --build "@USE_DIR@")
if(@MULTI@ AND NOT CONFIG STREQUAL "")
	list(APPEND _build --config "${CONFIG}")
endif()
message(STATUS "ENABLE_PGO: building optimized rebuild")
execute_process(COMMAND ${_build} RESULT_VARIABLE _rc)
if(NOT _rc EQUAL 0)
	message(FATAL_ERROR "ENABLE_PGO: optimized build failed (${_rc})")
endif()
]=] _pgo_script @ONLY)
	file(WRITE "${CMAKE_BINARY_DIR}/pgo-run.cmake" "${_pgo_script}")

	string(CONFIGURE [=[
if(NOT EXISTS "@USE_DIR@/cmake_install.cmake")
	message(FATAL_ERROR "ENABLE_PGO: optimized build is missing. Build the default target before installing.")
endif()
set(_cmd "@CMAKE_COMMAND@" --install "@USE_DIR@" --prefix "${CMAKE_INSTALL_PREFIX}")
if(CMAKE_INSTALL_CONFIG_NAME)
	list(APPEND _cmd --config "${CMAKE_INSTALL_CONFIG_NAME}")
endif()
if(CMAKE_INSTALL_COMPONENT)
	list(APPEND _cmd --component "${CMAKE_INSTALL_COMPONENT}")
endif()
execute_process(COMMAND ${_cmd} RESULT_VARIABLE _rc)
if(NOT _rc EQUAL 0)
	message(FATAL_ERROR "ENABLE_PGO: install of the optimized build failed (${_rc})")
endif()
]=] _pgo_install @ONLY)
	file(WRITE "${CMAKE_BINARY_DIR}/pgo-install.cmake" "${_pgo_install}")
	install(SCRIPT "${CMAKE_BINARY_DIR}/pgo-install.cmake")

	if(CMAKE_CONFIGURATION_TYPES)
		add_custom_target(pgo ALL
			COMMAND ${CMAKE_COMMAND} -DCONFIG=$<CONFIG> -P "${CMAKE_BINARY_DIR}/pgo-run.cmake"
			DEPENDS ${_pgo_deps}
			COMMENT "ENABLE_PGO: ctest, then optimized rebuild"
			VERBATIM
		)
	else()
		add_custom_target(pgo ALL
			COMMAND ${CMAKE_COMMAND} -DCONFIG=${CMAKE_BUILD_TYPE} -P "${CMAKE_BINARY_DIR}/pgo-run.cmake"
			DEPENDS ${_pgo_deps}
			COMMENT "ENABLE_PGO: ctest, then optimized rebuild"
			VERBATIM
		)
	endif()
endfunction()

cmake_language(DEFER DIRECTORY "${CMAKE_SOURCE_DIR}" CALL pgo_attach_training)
