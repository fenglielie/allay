cmake_minimum_required(VERSION 3.15 FATAL_ERROR)
project(allay VERSION 0.1)
include(cmake/zero.cmake)

zero_init_quiet()
zero_info()

zero_add_subdirs_rec(src)

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

        zero_add_subdirs_rec(demo)
    endif()
endif()

set(ALLAY_TARGETS
    mtest
    safe_input
    msignal
    mlog
    data_handler
    var_type_dict
    simple_thread_pool
    mtracer
    mparser
    ini_parser
    pbar
    windows_console
    colorful
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

zero_check_update()
