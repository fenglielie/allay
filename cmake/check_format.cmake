set(TEST_FORMAT_CODE
    "#include <format>\n"
    "int main() {\n"
    "    std::format(\"Hello, {}!\", \"world\");\n"
    "    return 0;\n"
    "}\n"
)

include(CheckCXXSourceCompiles)

check_cxx_source_compiles("${TEST_FORMAT_CODE}" FORMAT_SUPPORTED)
