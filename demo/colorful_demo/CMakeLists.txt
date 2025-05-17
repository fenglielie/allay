if(FORMAT_SUPPORTED)
    add_executable(colorful_demo colorful_demo.cpp)
    target_link_libraries(colorful_demo PRIVATE colorful)

    add_test(NAME colorful_demo COMMAND colorful_demo)
else()
    message(STATUS "The compiler does not support C++20 <format>, skipping target 'colorful_demo'.")
endif()
