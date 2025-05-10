add_executable(ini_parser_demo ini_parser_demo.cpp)
target_link_libraries(ini_parser_demo PRIVATE ini_parser)
target_compile_definitions(ini_parser_demo PRIVATE "PREFIX=\"${CMAKE_CURRENT_SOURCE_DIR}\"")

add_test(NAME ini_parser_demo COMMAND ini_parser_demo)
