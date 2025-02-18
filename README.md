# Allay

<div style="text-align: center;">
  <img src="https://raw.githubusercontent.com/fenglielie/notes_image/main/img/allay_fly.gif" style="width: 50%" title="Allay"/>
</div>

Allay is a collection of small C++ tools for practice, provided as header-only libraries unless otherwise specified.

1. **mtest**: A header-only testing framework mimicking gtest.
2. **safe_input**: A component ensuring safe input handling.
3. **msignal**: An observer pattern implementation for signal transmission.
4. **mlog**:  A simple logging library.
5. **data_handler**: A component for reading and writing data files.
6. **var_type_dict**: A heterogeneous dictionary based on template metaprogramming (reference: 《C++模板元编程实战：一个深度学习框架的初步实现》).
7. **simple_thread_pool**: A simple C++ thread pool （[reference](https://www.limerence2017.com/2023/09/17/concpp07/)）.
8. **mtracer**: A simple function call stack tracing component (using C++20's `std::source_location` instead of traditional `__FILE__` and others).
9. **mparser**: A simple command-line parser.
10. **ini_parser**: A simple INI file parser.
11. **progressbar**: A simple command-line progress bar display.
12. **pbar**：c version of progressbar.
13. **gaussquad**: A Cpp implementation of [gaussquad](https://github.com/fenglielie/gaussquad) (MATLAB).
14. **windows_console**: A Windows-specific utility for handling console input/output with UTF-8 encoding and virtual terminal sequences. ([reference 1](https://chariri.moe/archives/408/windows-cin-read-utf8/), [reference 2](https://stackoverflow.com/questions/48176431/reading-utf-8-characters-from-console))
15. **colorful**: A C++ library for adding color to console output.

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

run xxx demo
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
target_link_libraries(demo PRIVATE allay::pbar)
```
