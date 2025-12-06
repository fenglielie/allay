#pragma once

#include <algorithm>
#include <iostream>
#include <sstream>
#include <string>
#include <tuple>
#include <vector>

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

private:
    static void clean_line(std::string &line) {
        // 替换异常字符为空格
        std::ranges::replace(line, '\r', ' ');
        std::ranges::replace(line, '\t', ' ');

        // 删除行首行末的空格片段
        line.erase(0, line.find_first_not_of(' '));
        line.erase(line.find_last_not_of(' ') + 1);

        // 查找注释符#所在位置 并且删除后续的片段
        std::size_t n_comment_start = line.find_first_of('#');
        // 定位并删除行尾的注释
        if (n_comment_start != std::string::npos) {
            line.erase(n_comment_start);
        }
    }

    static LineType read_line(const std::string &str, char delimiter) {
        std::istringstream ss(str);
        LineType dataLine;
        read_tuple_from_stream(ss, dataLine, delimiter);
        return dataLine;
    }

    template <std::size_t I = 0, typename... Args>
    static void read_tuple_from_stream(std::istringstream &ss,
                                       std::tuple<Args...> &t, char delimiter) {
        if constexpr (I < sizeof...(Args)) {
            // 读取数据并检查错误
            if (!(ss >> std::get<I>(t))) {
                throw std::runtime_error("Error reading data");
            }

            // 检查分隔符
            if constexpr (I < sizeof...(Args) - 1) {
                if (delimiter != ' ') {
                    char sep = 0;
                    if (!(ss >> sep) || sep != delimiter) {
                        throw std::runtime_error("Delimiter not found");
                    }
                }
            }

            // 递归处理下一个元素
            read_tuple_from_stream<I + 1, Args...>(ss, t, delimiter);
        }
    }

    static void write_line(std::ostream &os, const LineType &dataLine,
                           char delimiter) {
        write_tuple_to_stream(os, dataLine, delimiter);
    }

    template <std::size_t I = 0, typename... Args>
    static void write_tuple_to_stream(std::ostream &os,
                                      const std::tuple<Args...> &t,
                                      char delimiter) {
        if constexpr (I < sizeof...(Args)) {
            // 写入数据
            os << std::get<I>(t);

            // 写入分隔符（如果是最后一个元素，则换成换行符）
            if constexpr (I < sizeof...(Args) - 1) { os << delimiter; }
            else { os << '\n'; }

            // 递归写入下一个元素
            write_tuple_to_stream<I + 1, Args...>(os, t, delimiter);
        }
    }
};
