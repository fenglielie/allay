#include "allay/data_handler/data_handler.hpp"

int main() {
    // 读取 Nodes.txt
    auto nodes = DataHandler<int, double, double>::read(
        FileRAII(PREFIX "/Nodes.csv", std::ios::in).get_stream(), ',');
    // 写入 tmp_Nodes.csv
    DataHandler<int, double, double>::write(
        FileRAII(PREFIX "/tmp_Nodes.csv", std::ios::out).get_stream(), nodes,
        ' ');

    // 读取 Triangles.txt（有格式错误）
    auto triangles = DataHandler<int, int, int, int, double>::read(
        FileRAII(PREFIX "/Triangles.txt", std::ios::in).get_stream(), ' ');
    // 写入 tmp_Triangles.txt（剔除了格式错误）
    DataHandler<int, int, int, int, double>::write(
        FileRAII(PREFIX "/tmp_Triangles.txt", std::ios::out).get_stream(),
        triangles, ',');

    return 0;
}
