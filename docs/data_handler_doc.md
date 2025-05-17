# 数据文件读写

数据文件读写组件`DataHandler`提供如下的静态方法接口：

1. `read` 逐行读取数据
2. `write` 逐行写入数据

两个接口的定义和主要实现如下
```cpp
template <typename... Ts>
class DataHandler {
public:
    using LineType = std::tuple<Ts...>;

    static std::vector<LineType> read(std::istream &input_stream,
                                      char delimiter) {
        std::vector<LineType> result;
        std::string line;
        while (std::getline(input_stream, line)) {
            clean_line(line);
            if (!line.empty()) { result.push_back(read_line(line, delimiter)); }
        }
        return result;
    }

    static void write(std::ostream &output_stream,
                      const std::vector<LineType> &data, char delimiter) {
        for (const auto &line : data) {
            write_line(output_stream, line, delimiter);
        }
    }
}
```

它们的处理逻辑核心是将每一行数据和一个`std::tuple`相对应，例如一行数据个数如下
```
1,10,2.0
```
可以对应为
```cpp
using LineType = std::tuple<int,int,double>;
```

- `read`方法需要提供一个输入流（通常是文件输入流），还需要提供行内的分隔符（例如`,`或` `），返回`std::vector<LineType>`数据。
- `write`方法需要提供一个输出流（通常是文件输出流），提供`std::vector<LineType>`数据，
也需要提供行内的分隔符（例如`,`或` `），无返回值。

上述接口都需要一个文件流，这里同时提供了针对文件流的RAII封装`FileRAII`，只需要提供文件名和打开方式，省略了打开和关闭文件的操作，在文件打开失败时会抛出异常。

一些细节的处理：

- `read`如果出现无法处理的格式错误（例如没有在数据之间读取到分隔符），会抛异常，不过在行内的最后一个元素之后，无论有没有分隔符，都不会影响`read`的读取；
- `read`支持`#`开头的注释，包括整行注释和行尾注释，同时`read`不支持`#`作为分隔符；
- `read`会自动跳过空行，自动将`\t`，`\r`替换为空格；
- `write`在写入时，不会在行内最后一个元素之后加上分隔符；
- `read`始终采用`>> item`从输入流中获取数据，`write`采用`<< item`向输出流传递数据。


使用示例：
读取一个以`,`作为分隔符的数据文件，再把数据重新以` `作为分隔符写入新文件中，只需要两条语句
```cpp
// 读取 Nodes.txt
auto nodes = DataHandler<int, double, double>::read(
    FileRAII(PREFIX "/Nodes.csv", std::ios::in).get_stream(), ',');
// 写入 tmp_Nodes.csv
DataHandler<int, double, double>::write(
    FileRAII(PREFIX "/tmp_Nodes.csv", std::ios::out).get_stream(), nodes,' ');
```
这里原本的Nodes.csv形如
```
35, 1.20086, 2.56496
36, 3.27009, 2.4683
37, 2.7188, 2.46564
```
重新写入的tmp_Nodes.csv形如
```
35 1.20086 2.56496
36 3.27009 2.4683
37 2.7188 2.46564
```
