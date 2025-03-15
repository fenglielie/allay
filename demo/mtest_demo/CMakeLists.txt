add_executable(mtest_demo mtest_demo.cpp)
target_link_libraries(mtest_demo PRIVATE mtest)

add_test(NAME mtest_demo COMMAND mtest_demo)
