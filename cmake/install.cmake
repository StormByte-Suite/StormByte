install(TARGETS StormByte
	ARCHIVE 		DESTINATION "${CMAKE_INSTALL_LIBDIR}"
	LIBRARY 		DESTINATION "${CMAKE_INSTALL_LIBDIR}"
	RUNTIME 		DESTINATION ${CMAKE_INSTALL_BINDIR}
)
install(DIRECTORY "${CMAKE_SOURCE_DIR}/lib/public/StormByte/"
	DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/StormByte"
	FILES_MATCHING
	PATTERN "*.h"
	PATTERN "*.hxx"
	PATTERN "*.txx"
)
configure_file(
	"${CMAKE_SOURCE_DIR}/cmake/StormByteCoreDependencies.cmake.in"
	"${CMAKE_BINARY_DIR}/StormByteCoreDependencies.cmake"
	@ONLY
)
install(FILES
	"${CMAKE_SOURCE_DIR}/cmake/FindStormByte.cmake"
	"${CMAKE_SOURCE_DIR}/cmake/StormByteCommon.cmake"
	"${CMAKE_BINARY_DIR}/StormByteCoreDependencies.cmake"
	DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/cmake/Modules"
)