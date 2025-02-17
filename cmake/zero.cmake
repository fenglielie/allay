# zero.cmake
# fenglielie@qq.com
# 2025-02-17
# version 1.2

set(ZERO_CMAKE_VERSION "1.2" CACHE STRING "Version of zero.cmake. (set by zero.cmake)" FORCE)
set(ZERO_CMAKE_FILE "${CMAKE_CURRENT_LIST_FILE}" CACHE PATH "Path to zero.cmake. (set by zero.cmake)" FORCE)

## marcos

macro(zero_usage)
    message(STATUS "\n"
        "    ###################################\n"
        "    #                                 #\n"
        "    #          CMakeZero ${ZERO_CMAKE_VERSION}          #\n"
        "    #                                 #\n"
        "    ###################################\n")

    message(STATUS "macro usage:\n"
        "   - zero_usage(): print usage\n"
        "   - zero_init(): print usage, then init the project (call after project)\n"
        "   - zero_init_quiet(): init the project (call after project)\n"
        "   - zero_info(): show infomation\n"
        "   - zero_check_update(): check for updates to zero.cmake\n")

    message(STATUS "function usage:\n"
        "   - zero_add_subdirs(src): go to src/CMakeLists.txt and src/*/CMakeLists.txt\n"
        "   - zero_add_subdirs_rec(src): go to src/CMakeLists.txt and src/*/*/CMakeLists.txt (recurse)\n"
        "   - zero_get_files(tmp test): search source files in test/ => tmp\n"
        "   - zero_get_files_rec(tmp test): search source files in test/ and test/*/ => tmp (recurse)\n")

    message(STATUS "target function usage:\n"
        "   - zero_target_preset_definitions(targetname): add some compile definitions\n"
        "     * ZERO_TARGET_NAME=targetname\n"
        "     * ZERO_PROJECT_SOURCE_DIR=PROJECT_SOURCE_DIR\n"
        "     * ZERO_CURRENT_SOURCE_DIR=CMAKE_CURRENT_SOURCE_DIR\n"
        "   - zero_target_info(targetname): show target properties\n")

endmacro()

macro(zero_init)
    zero_usage()
    zero_init_quiet()
endmacro()

macro(zero_init_quiet)
    message(STATUS ">> Init Project: ${PROJECT_NAME} ${PROJECT_VERSION} (supported by zero.cmake)")

    if(PROJECT_BINARY_DIR STREQUAL PROJECT_SOURCE_DIR)
        message(FATAL_ERROR "The binary directory cannot be the same as source directory")
    endif()

    if(NOT CMAKE_BUILD_TYPE)
        message(STATUS ">> Set CMAKE_BUILD_TYPE = Release (default)")
        set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel. (set by zero.cmake)" FORCE)
    endif()

    # keep use folders in build/
    set_property(GLOBAL PROPERTY USE_FOLDERS ON)

    # create compile_commands.json in build/
    set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE BOOL "Export compile commands to compile_commands.json. (set by zero.cmake)" FORCE)

    # c++ standard = C++20 (required)
    set(CMAKE_CXX_STANDARD 20)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_CXX_EXTENSIONS OFF)

    # libfunc_d (debug) / libfunc (release)
    set(CMAKE_DEBUG_POSTFIX "_d")

    # ./bin
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin")
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

    # ./lib
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/lib")
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY})
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY})

    # ./lib
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/lib")
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})

    # c/c++ compile flags
    if (("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU") OR ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang"))
        set(common_flags "-Wall -Wextra -Wfatal-errors -Wshadow -Wconversion -Wsign-conversion -Wuninitialized -pedantic -Wno-unused-parameter")
    elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
        set(common_flags "/W3 /WX /MP /utf-8 /permissive- /Zc:__cplusplus")
    else()
        set(common_flags "")
    endif()

    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${common_flags}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${common_flags}")

endmacro()

macro(zero_info)
    message(STATUS "---------- <Check Information Begin> ----------")
    message(STATUS ">> system = ${CMAKE_SYSTEM_NAME}")
    message(STATUS ">> generator = ${CMAKE_GENERATOR}")
    message(STATUS ">> build_type = ${CMAKE_BUILD_TYPE}")
    message(STATUS ">> cxx_compiler_id = ${CMAKE_CXX_COMPILER_ID}(${CMAKE_CXX_COMPILER_VERSION})")
    message(STATUS ">> cxx_compiler = ${CMAKE_CXX_COMPILER}")
    message(STATUS ">> cxx_flags = " ${CMAKE_CXX_FLAGS})
    message(STATUS ">> cxx_flags_debug = " ${CMAKE_CXX_FLAGS_DEBUG})
    message(STATUS ">> cxx_flags_release = " ${CMAKE_CXX_FLAGS_RELEASE})
    message(STATUS ">> linker = ${CMAKE_LINKER}")
    message(STATUS ">> exe_linker_flags = ${CMAKE_EXE_LINKER_FLAGS}")
    message(STATUS ">> source_dir = ${PROJECT_SOURCE_DIR}")
    message(STATUS ">> binary_dir = ${CMAKE_BINARY_DIR}")
    message(STATUS ">> install_prefix = ${CMAKE_INSTALL_PREFIX}")
    message(STATUS "-----------------------------------------------")
endmacro()

macro(zero_check_update)
    set(TEMP_FILE "${CMAKE_BINARY_DIR}/zero.cmake")
    set(REMOTE_URL "https://raw.githubusercontent.com/fenglielie/cmakezero/main/cmake/zero.cmake")
    set(LOCAL_FILE "${ZERO_CMAKE_FILE}")

    set(ZERO_CMAKE_REMOTE_HASH "" CACHE STRING "Cached hash of the remote zero.cmake. (set by zero.cmake)")

    # [FAIL] Return if local file doesn't exist
    if(NOT (EXISTS "${LOCAL_FILE}"))
        message(WARNING "Local file ${LOCAL_FILE} does not exist.")
        return()
    endif()

    # [FAIL] Skip download if download failed previously
    if("${ZERO_CMAKE_REMOTE_HASH}" STREQUAL "DOWNLOAD_FAILED")
        message(STATUS ">> Skipping update check due to a previous download failure.")
        return()
    endif()

    set(NEED_DOWNLOAD TRUE)

    # Skip download if the remote file matches the cached local hash
    if(EXISTS "${TEMP_FILE}")
        file(SHA256 "${TEMP_FILE}" REMOTE_HASH_OLD)
        if("${REMOTE_HASH_OLD}" STREQUAL "${ZERO_CMAKE_REMOTE_HASH}")
            set(NEED_DOWNLOAD FALSE)
        endif()
    endif()

    if(NEED_DOWNLOAD)
        # Download the remote file
        message(STATUS ">> Downloading ${REMOTE_URL}")
        file(DOWNLOAD "${REMOTE_URL}" "${TEMP_FILE}" STATUS DOWNLOAD_STATUS SHOW_PROGRESS TIMEOUT 20)

        # Check if the download was successful
        list(GET DOWNLOAD_STATUS 0 DOWNLOAD_RESULT)

        # [FAIL] Download failed
        if(NOT DOWNLOAD_RESULT EQUAL 0)
            message(STATUS ">> Download failed. Status: ${DOWNLOAD_STATUS}")
            message(STATUS ">> Don't worry. This does not affect the usage.")

            set(ZERO_CMAKE_REMOTE_HASH "DOWNLOAD_FAILED" CACHE STRING "Cached hash of the remote zero.cmake. (set by zero.cmake)")
            return()
        endif()

        # update the hash
        file(SHA256 "${TEMP_FILE}" REMOTE_HASH)
        set(ZERO_CMAKE_REMOTE_HASH "${REMOTE_HASH}" CACHE STRING "Cached hash of the remote zero.cmake. (set by zero.cmake)" FORCE)
    endif()

    # Compare local and remote hashes
    file(SHA256 "${LOCAL_FILE}" LOCAL_HASH)
    if("${LOCAL_HASH}" STREQUAL "${ZERO_CMAKE_REMOTE_HASH}")
        # [OK] the local file matches the cached remote hash
        message(STATUS ">> ${LOCAL_FILE} is up to date.")
    else()
        # [FAIL] need update
        message(WARNING "A newer version is available: ${TEMP_FILE}")
    endif()
endmacro()

## functions

function(zero_get_files rst _sources)
    set(tmp_rst "")

    foreach(item ${_sources})
        if(IS_DIRECTORY ${item}) # item is dir
            file(GLOB itemSrcs CONFIGURE_DEPENDS
                ${item}/*.c ${item}/*.C ${item}/*.cc ${item}/*.cpp ${item}/*.cxx
                ${item}/*.h ${item}/*.hpp
            )
            foreach(src ${itemSrcs})
                # Convert to absolute and normalize
                cmake_path(ABSOLUTE_PATH src NORMALIZE OUTPUT_VARIABLE src)
                list(APPEND tmp_rst ${src})
            endforeach()
        else() # item is file
            # Convert to absolute and normalize
            cmake_path(ABSOLUTE_PATH item NORMALIZE OUTPUT_VARIABLE item)
            list(APPEND tmp_rst ${item})
        endif()
    endforeach()

    set(${rst} ${tmp_rst} PARENT_SCOPE) # return
endfunction()

function(zero_get_files_rec rst _sources)
    set(tmp_rst "")

    foreach(item ${_sources})
        if(IS_DIRECTORY ${item}) # item is dir
            file(GLOB_RECURSE itemSrcs CONFIGURE_DEPENDS
                ${item}/*.c ${item}/*.C ${item}/*.cc ${item}/*.cpp ${item}/*.cxx
                ${item}/*.h ${item}/*.hpp
            )
            foreach(src ${itemSrcs})
                # Convert to absolute and normalize
                cmake_path(ABSOLUTE_PATH src NORMALIZE OUTPUT_VARIABLE src)
                list(APPEND tmp_rst ${src})
            endforeach()
        else() # item is file
            # Convert to absolute and normalize
            cmake_path(ABSOLUTE_PATH item NORMALIZE OUTPUT_VARIABLE item)
            list(APPEND tmp_rst ${item})
        endif()
    endforeach()

    set(${rst} ${tmp_rst} PARENT_SCOPE) # return
endfunction()

# go to all relative subdirs which contains CMakeLists.txt from CMAKE_CURRENT_SOURCE_DIR.(not recurse)
# may not ordered as you want.
function(zero_add_subdirs _path)
    # Get absolute path of the target directory and normalize it
    cmake_path(ABSOLUTE_PATH _path BASE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} NORMALIZE OUTPUT_VARIABLE _path)

    # Search all subdirectories
    file(GLOB children LIST_DIRECTORIES ON CONFIGURE_DEPENDS ${_path}/*)
    set(dirs "")
    list(PREPEND children "${_path}") # Add root directory first

    # Append to dirs if it contains CMakeLists.txt
    foreach(item ${children})
        if((IS_DIRECTORY ${item}) AND (EXISTS "${item}/CMakeLists.txt"))
            cmake_path(ABSOLUTE_PATH item NORMALIZE OUTPUT_VARIABLE item)
            list(APPEND dirs ${item})
        endif()
    endforeach()

    foreach(dir ${dirs})
        message(STATUS ">> Go to ${dir}")
        add_subdirectory(${dir})
    endforeach()
endfunction()

# go to all relative subdirs which contains CMakeLists.txt from CMAKE_CURRENT_SOURCE_DIR.(recurse)
# may not ordered as you want.
function(zero_add_subdirs_rec _path)
    # Get absolute path of the target directory and normalize it
    cmake_path(ABSOLUTE_PATH _path BASE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} NORMALIZE OUTPUT_VARIABLE _path)

    # Search all subdirectories
    file(GLOB_RECURSE children LIST_DIRECTORIES ON CONFIGURE_DEPENDS ${_path}/*)
    set(dirs "")
    list(PREPEND children "${_path}") # Add root directory first

    # Append to dirs if it contains CMakeLists.txt
    foreach(item ${children})
        if((IS_DIRECTORY ${item}) AND (EXISTS "${item}/CMakeLists.txt"))
            cmake_path(ABSOLUTE_PATH item NORMALIZE OUTPUT_VARIABLE item)
            list(APPEND dirs ${item})
        endif()
    endforeach()

    foreach(dir ${dirs})
        message(STATUS ">> Go to ${dir}")
        add_subdirectory(${dir})
    endforeach()
endfunction()

## target functions

function(zero_inside_check_target _target)
    if(NOT TARGET "${_target}")
        message(FATAL_ERROR "${_target} is not a target")
    endif()
endfunction()

function(zero_target_preset_definitions _target)
    zero_inside_check_target(${_target})

    target_compile_definitions(${_target} PRIVATE
        "ZERO_TARGET_NAME=\"${_target}\""
        "ZERO_PROJECT_SOURCE_DIR=\"${PROJECT_SOURCE_DIR}\""
        "ZERO_CURRENT_SOURCE_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}\""
    )
endfunction()

function(zero_inside_list_print)
    # TITLE
    # PREFIX a
    # STRS a;b;c

    set(options "")
    set(oneValueArgs TITLE PREFIX)
    set(multiValueArgs STRS)
    cmake_parse_arguments("ARG" "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    # return if STRS is empty
    list(LENGTH ARG_STRS strsLength)
    if(NOT strsLength)
        return()
    endif()

    # print title
    if(NOT (${ARG_TITLE} STREQUAL ""))
        message(STATUS ${ARG_TITLE})
    endif()

    # print prefix+item in STRS
    foreach(str ${ARG_STRS})
        message(STATUS "${ARG_PREFIX}${str}")
    endforeach()

endfunction()

function(zero_inside_print_property _target _porperty)
    string(TOUPPER "${_porperty}" _porperty)

    get_target_property(tmp ${_target} ${_porperty})
    if(NOT (tmp STREQUAL "tmp-NOTFOUND"))
        string(TOLOWER "${_porperty}" porperty_lower)
        zero_inside_list_print(STRS "${tmp}" TITLE  "${porperty_lower}:" PREFIX "  * ")
    endif()

    get_target_property(tmp ${_target} INTERFACE_${_porperty})
    if(NOT (tmp STREQUAL "tmp-NOTFOUND"))
        string(TOLOWER "${_porperty}" porperty_lower)
        zero_inside_list_print(STRS "${tmp}" TITLE  "interface_${porperty_lower}:" PREFIX "  * ")
    endif()

endfunction()

function(zero_target_info _target)
    zero_inside_check_target(${_target})

    message(STATUS "---------- <Check Target Begin> ----------")
    message(STATUS "name: ${_target}")

    get_target_property(tmp ${_target} TYPE)
    string(TOLOWER "${tmp}" tmp)
    message(STATUS "type: ${tmp}")

    get_target_property(tmp ${_target} SOURCE_DIR)
    message(STATUS "location: ${tmp}")

    zero_inside_print_property(${_target} SOURCES)
    zero_inside_print_property(${_target} INCLUDE_DIRECTORIES)
    zero_inside_print_property(${_target} LINK_DIRECTORIES)
    zero_inside_print_property(${_target} LINK_LIBRARIES)
    zero_inside_print_property(${_target} LINK_OPTIONS)
    zero_inside_print_property(${_target} COMPILE_OPTIONS)
    zero_inside_print_property(${_target} COMPILE_DEFINITIONS)

    message(STATUS "------------------------------------------")
endfunction()
