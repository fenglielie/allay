add_executable(data_record_io_demo data_record_io_demo.cpp)
target_link_libraries(data_record_io_demo PRIVATE data_record_io)
target_compile_definitions(data_record_io_demo PRIVATE "PREFIX=\"${CMAKE_CURRENT_SOURCE_DIR}\"")

add_test(NAME data_record_io_demo COMMAND data_record_io_demo)
