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
    add_executable(pbar_demo pbar_demo.cpp)
    target_link_libraries(pbar_demo PRIVATE pbar)

    add_test(NAME pbar_demo COMMAND pbar_demo)
else()
    message(STATUS "The compiler does not support C++20 <format>, skipping target 'pbar_demo'.")
endif()
