set(MCG_Headers
	"include/parsePly.h"
	"include/parser.h"
	"include/ppm.h"
)
set(MCG_Source
	"source/parsePly.cpp"
	"source/parser.cpp"
	"source/ppm.cpp"
)

add_library(metucenggraph
	${MCG_Headers}
	${MCG_Source}
)
target_include_directories(metucenggraph
	PUBLIC
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
	$<INSTALL_INTERFACE:include/MCG>
)

target_link_libraries(metucenggraph
	PRIVATE tinyxml2
)

install(TARGETS metucenggraph
	EXPORT metucenggraphTargets
	RUNTIME DESTINATION bin
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib/MCG
)

install(DIRECTORY include/
	DESTINATION include/MCG
)

install(EXPORT metucenggraphTargets
	FILE metucenggraphTargets.cmake
	NAMESPACE metucenggraph::
	DESTINATION lib/cmake/metucenggraph
)

include(CMakePackageConfigHelpers)
configure_package_config_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake/Config.cmake.in
	"${CMAKE_CURRENT_BINARY_DIR}/metucenggraphConfig.cmake"
	INSTALL_DESTINATION lib/cmake/metucenggraph
)
write_basic_package_version_file(
	"${CMAKE_CURRENT_BINARY_DIR}/metucenggraphConfigVersion.cmake"
	VERSION "${CMAKE_PROJECT_VERSION}"
	COMPATIBILITY AnyNewerVersion
)
install(FILES
	"${CMAKE_CURRENT_BINARY_DIR}/metucenggraphConfig.cmake"
	"${CMAKE_CURRENT_BINARY_DIR}/metucenggraphConfigVersion.cmake"
	DESTINATION lib/cmake/metucenggraph
)
