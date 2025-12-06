# Allay

Allay is a collection of small C++ tools for practice, provided as header-only libraries unless otherwise specified.

- **mtest**: A header-only testing framework in the style of gtest.
- **mlog**:  A simple logging library. (TODO: refactor)
- **data_handler**: A component for reading and writing data files.
- **cmd_parser**: A simple cmd parser.
- **ini_parser**: A simple INI file parser.
- **progress**: A simple command-line progress bar display. (TODO: refactor)
- **console**: A Windows-specific utility for handling console input/output with UTF-8 encoding and virtual terminal sequences. ([reference 1](https://chariri.moe/archives/408/windows-cin-read-utf8/), [reference 2](https://stackoverflow.com/questions/48176431/reading-utf-8-characters-from-console))

---

| compiler | version | flags      |
| -------- | ------- | ---------- |
| clang    | 18.0.0  | -std=c++20 |
| gcc      | 13.0.0  | -std=c++20 |
| MSVC     | 2022    | /std:c++20 |


generate and build (cmake version >= 3.15)
```bash
cmake -S . -B build
cmake --build ./build -j8
```

test
```bash
cd ./build
ctest -j8
```

run
```bash
./bin/xxx_demo
```

install
```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="~/.local/"
cmake --build build --target install
```

usage
```cmake
find_package(allay QUIET)
if(NOT allay_FOUND)
    include(FetchContent)
    FetchContent_Declare(
        allay
        GIT_REPOSITORY https://github.com/fenglielie/allay.git
        # GIT_REPOSITORY git@github.com:fenglielie/allay.git
        GIT_TAG main
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    )
    FetchContent_MakeAvailable(allay)
endif()

add_executable(demo demo.cpp)
target_link_libraries(demo PRIVATE allay::mlog)
```
