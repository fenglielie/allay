add_executable(mlog_demo mlog_demo.cpp)
target_link_libraries(mlog_demo PRIVATE mlog)
target_compile_definitions(mlog_demo PRIVATE "PREFIX=\"${CMAKE_CURRENT_SOURCE_DIR}\"")

add_test(NAME mlog_demo COMMAND mlog_demo)
