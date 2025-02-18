

include(CheckCXXSourceCompiles)

set(TEST_FORMAT_CODE
    "#include <format>\n"
    "int main() {\n"
    "    std::format(\"Hello, {}!\", \"world\");\n"
    "    return 0;\n"
    "}\n"
)

check_cxx_source_compiles("${TEST_FORMAT_CODE}" FORMAT_SUPPORTED)

if(FORMAT_SUPPORTED)
    add_executable(colorful_demo colorful_demo.cpp)
    target_link_libraries(colorful_demo PRIVATE colorful)

    add_test(NAME colorful_demo COMMAND colorful_demo)
else()
    message(STATUS "The compiler does not support C++20 <format>, skipping target 'colorful_demo'.")
endif()
