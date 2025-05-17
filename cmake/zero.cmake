#================================
# zero.cmake
# fenglielie@qq.com 2025-05-10
#================================

# macros:
# - zero_setup()  (call after project)
# - zero_check()
# functions:
# - zero_add_subdirs(src): go to src/CMakeLists.txt and src/*/CMakeLists.txt
# - zero_add_subdirs(src RECURSE): go to src/CMakeLists.txt and src/*/*/CMakeLists.txt (recurse)
# - zero_get_files(tmp test): search source files in test/ => tmp
# - zero_get_files(tmp test RECURSE): search source files in test/ and test/*/ => tmp (recurse)
# - zero_check_target(targetname): display target properties

## marcos

macro(zero_setup)
    message(STATUS ">> Set up project: ${PROJECT_NAME} ${PROJECT_VERSION} (supported by zero.cmake)")

    if(PROJECT_BINARY_DIR STREQUAL PROJECT_SOURCE_DIR)
        message(FATAL_ERROR "The binary directory cannot be the same as source directory.")
    endif()

    if(NOT CMAKE_BUILD_TYPE)
        message(STATUS ">> Set CMAKE_BUILD_TYPE = Release (default)")
        set(CMAKE_BUILD_TYPE "Release" CACHE STRING "(set by zero.cmake)" FORCE)
    endif()

    # create compile_commands.json
    set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE BOOL "(set by zero.cmake)" FORCE)

    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin")
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/lib")
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY})
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY})

    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/lib")
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})

    # c/c++ compile flags
    if(("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU") OR ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang"))
        set(flags "-Wall -Wextra -Wfatal-errors -Wshadow -Wconversion -Wsign-conversion -Wuninitialized -pedantic -Wno-unused-parameter")
    elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
        set(flags "/W3 /WX /MP /utf-8 /permissive- /Zc:__cplusplus")
    else()
        set(flags "")
    endif()

    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${flags}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${flags}")
endmacro()

macro(zero_check)
    message(STATUS "------- <Check Information> -------")
    message(STATUS ">> system = ${CMAKE_SYSTEM_NAME}")
    message(STATUS ">> generator = ${CMAKE_GENERATOR}")
    message(STATUS ">> build_type = ${CMAKE_BUILD_TYPE}")
    message(STATUS ">> cxx_compiler_id = ${CMAKE_CXX_COMPILER_ID}(${CMAKE_CXX_COMPILER_VERSION})")
    message(STATUS ">> cxx_compiler = ${CMAKE_CXX_COMPILER}")
    message(STATUS ">> cxx_flags = " ${CMAKE_CXX_FLAGS})
    message(STATUS ">> cxx_flags_debug = " ${CMAKE_CXX_FLAGS_DEBUG})
    message(STATUS ">> cxx_flags_release = " ${CMAKE_CXX_FLAGS_RELEASE})
    message(STATUS ">> source_dir = ${PROJECT_SOURCE_DIR}")
    message(STATUS ">> binary_dir = ${CMAKE_BINARY_DIR}")
    message(STATUS ">> install_prefix = ${CMAKE_INSTALL_PREFIX}")
    message(STATUS "-----------------------------------")
endmacro()

## functions

function(zero_get_files rst _sources)
    set(options RECURSE)
    set(oneValueArgs "")
    set(multiValueArgs "")
    cmake_parse_arguments("ARG" "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    set(tmp_rst "")

    foreach(item IN LISTS _sources)
        if(IS_DIRECTORY ${item})
            if(ARG_RECURSE)
                file(GLOB_RECURSE itemSrcs CONFIGURE_DEPENDS
                    ${item}/*.c ${item}/*.C ${item}/*.cc ${item}/*.cpp ${item}/*.cxx
                    ${item}/*.h ${item}/*.hpp
                )
            else()
                file(GLOB itemSrcs CONFIGURE_DEPENDS
                    ${item}/*.c ${item}/*.C ${item}/*.cc ${item}/*.cpp ${item}/*.cxx
                    ${item}/*.h ${item}/*.hpp
                )
            endif()

            foreach(src IN LISTS itemSrcs)
                cmake_path(ABSOLUTE_PATH src NORMALIZE OUTPUT_VARIABLE src)
                list(APPEND tmp_rst ${src})
            endforeach()
        else()
            cmake_path(ABSOLUTE_PATH item NORMALIZE OUTPUT_VARIABLE item)
            list(APPEND tmp_rst ${item})
        endif()
    endforeach()

    set(${rst} ${tmp_rst} PARENT_SCOPE) # return rst
endfunction()

# go to all relative subdirs which contains CMakeLists.txt from CMAKE_CURRENT_SOURCE_DIR.
# may not ordered as you want.
function(zero_add_subdirs _path)
    set(options RECURSE)
    set(oneValueArgs "")
    set(multiValueArgs "")
    cmake_parse_arguments("ARG" "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    cmake_path(ABSOLUTE_PATH _path BASE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} NORMALIZE OUTPUT_VARIABLE _path)
    string(REGEX REPLACE "/+$" "" _path "${_path}") # a/b/ -> a/b

    if((NOT EXISTS "${_path}") OR (NOT IS_DIRECTORY "${_path}"))
        message(FATAL_ERROR "Invalid path: '${_path}' is not a valid directory.")
    endif()

    if(ARG_RECURSE)
        file(GLOB_RECURSE children LIST_DIRECTORIES ON CONFIGURE_DEPENDS ${_path}/*)
    else()
        file(GLOB children LIST_DIRECTORIES ON CONFIGURE_DEPENDS ${_path}/*)
    endif()

    # Add _path if it is not the current directory
    if(NOT ("${_path}" STREQUAL "${CMAKE_CURRENT_SOURCE_DIR}"))
        list(PREPEND children "${_path}")
    endif()

    set(dirs "")
    foreach(item IN LISTS children)
        if((IS_DIRECTORY ${item}) AND (EXISTS "${item}/CMakeLists.txt"))
            cmake_path(ABSOLUTE_PATH item NORMALIZE OUTPUT_VARIABLE item)
            list(APPEND dirs "${item}")
        endif()
    endforeach()

    foreach(dir IN LISTS dirs)
        if(ARG_RECURSE)
            message(STATUS ">> Add subdirectory (recurse): ${dir}")
        else()
            message(STATUS ">> Add subdirectory: ${dir}")
        endif()
        add_subdirectory(${dir})
    endforeach()
endfunction()

## target functions

function(zero_inside_list_print)
    set(options "")
    set(oneValueArgs TITLE PREFIX)
    set(multiValueArgs STRS)
    cmake_parse_arguments("ARG" "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    list(LENGTH ARG_STRS strsLength)
    if(NOT "${strsLength}")
        return()
    endif()

    if(NOT ("${ARG_TITLE}" STREQUAL ""))
        message(STATUS "${ARG_TITLE}")
    endif()

    foreach(str IN LISTS ARG_STRS)
        message(STATUS "${ARG_PREFIX}${str}")
    endforeach()
endfunction()

function(zero_inside_print_property _target _porperty)
    string(TOUPPER "${_porperty}" _porperty)
    string(TOLOWER "${_porperty}" _porperty_lower)

    get_target_property(tmp ${_target} ${_porperty})
    if(NOT (tmp STREQUAL "tmp-NOTFOUND"))
        zero_inside_list_print(STRS "${tmp}" TITLE "${_porperty_lower}:" PREFIX "  * ")
    endif()

    get_target_property(tmp ${_target} INTERFACE_${_porperty})
    if(NOT (tmp STREQUAL "tmp-NOTFOUND"))
        zero_inside_list_print(STRS "${tmp}" TITLE "${_porperty_lower}: (interface)" PREFIX "  + ")
    endif()
endfunction()

function(zero_check_target _target)
    if(NOT TARGET "${_target}")
        message(FATAL_ERROR "${_target} is not a target.")
    endif()

    get_target_property(target_type ${_target} TYPE)
    string(TOLOWER "${target_type}" target_type)
    get_target_property(target_source_dir ${_target} SOURCE_DIR)

    message(STATUS "---------- <Check Target> ----------")
    message(STATUS "name: ${_target}")
    message(STATUS "type: ${target_type}")
    message(STATUS "location: ${target_source_dir}")

    zero_inside_print_property("${_target}" SOURCES)
    zero_inside_print_property("${_target}" INCLUDE_DIRECTORIES)
    zero_inside_print_property("${_target}" LINK_DIRECTORIES)
    zero_inside_print_property("${_target}" LINK_LIBRARIES)
    zero_inside_print_property("${_target}" LINK_OPTIONS)
    zero_inside_print_property("${_target}" COMPILE_OPTIONS)
    zero_inside_print_property("${_target}" COMPILE_DEFINITIONS)
    message(STATUS "------------------------------------")
endfunction()
