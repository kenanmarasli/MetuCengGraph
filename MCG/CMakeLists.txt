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
