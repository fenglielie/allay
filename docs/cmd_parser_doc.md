# 命令行参数解析组件

`CmdParser` 是一个模仿 cmdline 的轻量级的 C++ 命令行参数解析组件，支持解析标志（flags）和选项（options），并能够处理回调函数、类型转换以及剩余参数。


## 特点：

1. **标志解析**
   - 支持定义简单的布尔标志，例如 `--help` 或 `--verbose`
   - 标志是可选的，在解析过程中会对标记出现的次数计数
   - 可以通过回调函数来定义标志存在时的触发事件

2. **选项解析**
   - 支持带值的选项，例如 `--scale=2.0` 或 `--scale 2.0`
   - 选项可以设置为可选或必选，在解析过程中会记录每次出现的选项的值
   - 支持的数据类型包括：`int`、`double`、`bool`、`string`、`char`、`size_t`
   - 支持四种添加方式
     1. 提供默认值，提供回调函数检查
     2. 提供回调函数检查
     3. 提供默认值
     4. 都不提供

3. **补充**
   - 标志和选项的名称要求：
     - 必须提供一个`--`开头的，加上若干字母、数字、下划线组成的完整名称，区分大小写，例如`--len`;
     - 可以额外提供一个`-`开头的，加上单个字母或数字组成的缩写名称，例如`-v`;
     - 所有名称互异。
   - 标志和选项都需要提供说明字符串，可以使用空字符串

## 使用方法

### 创建 `CmdParser` 实例
```cpp
auto parser = CmdParser{};
```


### 添加flag

例如
```cpp
parser.add_flag({"--gzip", "-g"}, "use gzip");

parser.add_flag({"-v", "--verbose"}, "");

parser.add_flag({"--help","-h"}, "show help message", [&parser]() {
    parser.print_usage();
    exit(0);
});
```

在解析开始之前，如果没有设置名称为`--help`或`-h`的flag或option，会自动按照上述形式进行添加。

### 添加option

下面是四种方式的示例
```cpp

parser.add_option<int>({"-l", "--len"},
                        "option with default value and checker", false, 10,
                        [](int arg) { return arg >= 0; });

parser.add_option({"-n", "--num"}, "option with default value", false, 0);

parser.add_option<double>("--scale", "option with checker", true,
                            [](double arg) { return arg >= 0; });

parser.add_option<int>({"-w", "--weight"},
                        "option without default value and checker", false);
```


### 解析命令行参数

提供`parse`方法进行解析，返回布尔值
```
bool ok = parser.parse(argc, argv);
```

可以包装一下，解析错误时直接打印帮助信息并退出
```cpp
if (!parser.parse(argc, argv)) {
    parser.print_usage();
    return 1;
}
```

默认提供的`parse_check`方法实现了类似的效果，可以直接使用
```cpp
void parse_check(int argc, char *argv[]) {
    // 如果解析失败，则打印帮助信息并退出程序
    if (!parse(argc, argv)) {
        print_usage();

        // 如果完全没有提供参数，返回0，否则返回1
        if (argc == 1) { exit(0); }
        exit(1);
    }
}
```


### 获取参数

对于flag，可以获取解析过程中出现的次数（通过完整名称或缩写均可）
```cpp
if (auto n = parser.get_count("--gzip")) {
    std::cout << "gzip count: " << n.value() << '\n';
}
```

对于option，可以获取对应的值，但是需要提供类型信息，并且这个类型需要和解析之前提供的一致
```cpp
std::cout << "Scale: " << parser.get_option<double>("--scale").value()
            << '\n';
std::cout << "Len: " << parser.get_option<int>("--len").value() << '\n';
```

如果option被多次设置，那么`get_option`只会获取最后一次的值，可以可以通过`get_option_all`获取设置的所有值，返回对应的`vector`。

```cpp
if (auto weights = parser.get_option_all<int>("-w")) {
    std::cout << "Weight: ";
    for (const auto &w : *weights) { std::cout << w << ' '; }
    std::cout << '\n';
}
```

某些情况下可能出现option没有值，此时`get_option`会返回一个`std::nullopt`，但是`get_option_all`会返回一个空的`vector`。

### 收集多余参数

`get_rest`方法会返回未被解析的参数列表，保持原有的顺序。
```cpp
auto rest = parser.get_rest();
for (const auto &s : rest) {
    std::cout << s << '\n';
}
```

### 辅助方法

设置程序名称（否则在解析时自动获取`argv[0]`），用于在帮助信息中提供使用示例
```cpp
parser.set_program_name("demo");
```

打印帮助信息
```cpp
parser.print_usage();
```

帮助信息例如
```
usage: demo --scale=double ...
 @required options:
   --scale          option with checker (double)
 @options:
   -l, --len        option with default value and checker (int [=10])
   -n, --num        option with default value (int [=0])
   -q, --quiet      option without default value and checker (int)
 @flags:
   -g, --gzip       use gzip
   -v, --verbose
   -h, --help       print help message
```

将参数解析规则设置为严格或宽松，默认情况下使用宽松模式
```cpp
parser.enable_strict_mode();
parser.disable_strict_mode();
```

严格和宽松模式的区别如下：

- 在严格模式下：
  - flag只有一种方式设置：`--verbose`，`-v`
  - option的键值对只有两种方式设置：
    - 分离：`--len 1`，`-l 1`
    - 使用等号：`--len=1`，`-l=1`
- 在宽松模式下，额外支持如下写法：
  - flag的缩写形式支持合并，例如`-abc`会被拆分为`-a -b -c`，如果这些都是合法的flag缩写名称；
  - option支持键值合并写法，例如`-l2`会被拆分为`-l 2`，`--len3`会被拆分为`--len 3`，如果拆分前只能在option范围内匹配到唯一的全称或缩写名称作为前缀，否则因为存在歧义，不会进行拆分。
