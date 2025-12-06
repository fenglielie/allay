#include "allay/data_handler/data_handler.hpp"

#include <fstream>

int main() {
    {
        // 读取 Nodes.txt
        auto fin = std::fstream(PREFIX "/Nodes.csv", std::ios::in);
        auto nodes = DataHandler<int, double, double>::read(fin, ',');

        // 写入 tmp_Nodes.csv
        auto fout = std::fstream(PREFIX "/tmp_Nodes.csv", std::ios::out);
        DataHandler<int, double, double>::write(fout, nodes, ' ');
    }

    {
        // 读取 Triangles.txt（有格式错误）
        auto fin = std::fstream(PREFIX "/Triangles.txt", std::ios::in);
        auto triangles =
            DataHandler<int, int, int, int, double>::read(fin, ' ');

        // 写入 tmp_Triangles.txt（剔除了格式错误）
        auto fout = std::fstream(PREFIX "/tmp_Triangles.txt", std::ios::out);
        DataHandler<int, int, int, int, double>::write(fout, triangles, ',');
    }
    return 0;
}
