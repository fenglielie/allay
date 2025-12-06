cmake_minimum_required(VERSION 3.15 FATAL_ERROR)
project(allay VERSION 0.1)
include(cmake/zero.cmake)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

zero_setup()
zero_check()

zero_add_subdirs(src RECURSE)

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    include(CTest)
    if(BUILD_TESTING)
        include(CheckCXXSourceCompiles)

        check_cxx_source_compiles("#include <format>
int main() {
    std::format(\"Hello, {}!\", \"world\");
    return 0;
}\n"
            FORMAT_SUPPORTED)

        zero_add_subdirs(demo RECURSE)
    endif()
endif()

set(ALLAY_TARGETS
    mtest
    mlog
    data_handler
    ini_parser
    cmd_parser
    progress
    console
)

foreach(target ${ALLAY_TARGETS})
    add_library(allay::${target} ALIAS ${target})
endforeach()

install(TARGETS ${ALLAY_TARGETS} EXPORT allayTargets)
install(EXPORT allayTargets
    FILE allayConfig.cmake
    NAMESPACE allay::
    DESTINATION lib/cmake/allay)
install(DIRECTORY include/allay DESTINATION include)
