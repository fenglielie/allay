add_executable(data_handler_demo data_handler_demo.cpp)
target_link_libraries(data_handler_demo PRIVATE data_handler)
target_compile_definitions(data_handler_demo PRIVATE "PREFIX=\"${CMAKE_CURRENT_SOURCE_DIR}\"")

add_test(NAME data_handler_demo COMMAND data_handler_demo)
