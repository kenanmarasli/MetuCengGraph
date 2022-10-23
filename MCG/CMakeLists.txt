cmake_minimum_required (VERSION 3.16)

add_library ( metucenggraph
	"source/parser.cpp"
	"source/ppm.cpp"
)
target_include_directories ( metucenggraph
	PUBLIC "include"
)

target_link_libraries ( metucenggraph
	PRIVATE tinyxml2
)
