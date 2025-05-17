if(FORMAT_SUPPORTED)
    add_executable(mtimer_demo mtimer_demo.cpp)
    target_link_libraries(mtimer_demo PRIVATE mtimer)

    add_test(NAME mtimer_demo COMMAND mtimer_demo)
else()
    message(STATUS "The compiler does not support C++20 <format>, skipping target 'mtimer_demo'.")
endif()
