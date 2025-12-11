cmake_minimum_required(VERSION 3.15 FATAL_ERROR)
project(allay VERSION 0.1)
include(cmake/zero.cmake)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

zero_setup()
zero_check()


set(ALLAY_TARGETS
    mtest
    mlog
    ini_parser
    cmd_parser
    progress
)

set(ALLAY_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

foreach(target ${ALLAY_TARGETS})
    add_library(${target} INTERFACE)
    target_include_directories(${target}
        INTERFACE
        $<BUILD_INTERFACE:${ALLAY_INCLUDE_DIR}>
        $<INSTALL_INTERFACE:include>
    )

    add_library(allay::${target} ALIAS ${target})
    zero_check_target(${target})
endforeach()


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


install(TARGETS ${ALLAY_TARGETS} EXPORT allayTargets)
install(EXPORT allayTargets
    FILE allayConfig.cmake
    NAMESPACE allay::
    DESTINATION lib/cmake/allay)
install(DIRECTORY include/allay DESTINATION include)
