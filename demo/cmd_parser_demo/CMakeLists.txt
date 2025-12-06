add_executable(cmd_parser_demo cmd_parser_demo.cpp)
target_link_libraries(cmd_parser_demo PRIVATE cmd_parser)

add_test(NAME cmd_parser_demo_f COMMAND cmd_parser_demo -g)
set_property(TEST cmd_parser_demo_f PROPERTY WILL_FAIL TRUE)

add_test(NAME cmd_parser_demo_1 COMMAND cmd_parser_demo --scale 1.0)
add_test(NAME cmd_parser_demo_2 COMMAND cmd_parser_demo --scale 2.0 --len=10 --gzip)
add_test(NAME cmd_parser_demo_3 COMMAND cmd_parser_demo --scale2 --weight=10 -w3)
