if(FORMAT_SUPPORTED)
    add_executable(progress_demo progress_demo.cpp)
    target_link_libraries(progress_demo PRIVATE progress)

    add_test(NAME progress_demo COMMAND progress_demo)
else()
    message(STATUS "The compiler does not support C++20 <format>, skipping target 'progress_demo'.")
endif()
