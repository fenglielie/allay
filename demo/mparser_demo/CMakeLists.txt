add_executable(mparser_demo mparser_demo.cpp)
target_link_libraries(mparser_demo PRIVATE mparser)

add_test(NAME mparser_demo_f COMMAND mparser_demo -g)
set_property(TEST mparser_demo_f PROPERTY WILL_FAIL TRUE)

add_test(NAME mparser_demo_1 COMMAND mparser_demo --scale 1.0)
add_test(NAME mparser_demo_2 COMMAND mparser_demo --scale 2.0 --len=10 --gzip)
add_test(NAME mparser_demo_3 COMMAND mparser_demo --scale2 --weight=10 -w3)
