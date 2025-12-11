# Allay

一组纯头文件的现代 C++ 小组件，实现的都是非常基础的，其他语言直接自带的功能。

- **mtest**: 模仿 gtest 的 header-only 测试框架
- **mlog**:  简单的日志库 （TODO: 需要优化, 可以参考这些实现进行重写: spdlog, [superlxh02/FastLog](https://github.com/superlxh02/FastLog)）
- **cmd_parser**: 命令行参数解析
- **ini_parser**: INI 文件解析
- **progress**: 命令行进度条

---

| compiler | version | flags      |
| -------- | ------- | ---------- |
| clang    | 18.0.0  | -std=c++20 |
| gcc      | 13.0.0  | -std=c++20 |
| MSVC     | 2022    | /std:c++20 |

构建 -> 编译 -> 测试 -> 安装
```bash
# generate
cmake -S . -B build

# build
cmake --build ./build -j8

# test
cd ./build
ctest -j8

# install
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="~/.local/"
cmake --build build --target install
```

在其他 cmake 项目中使用
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
