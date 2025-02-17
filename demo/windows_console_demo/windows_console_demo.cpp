#include "allay/windows_console/windows_console.hpp"

#include <iostream>
#include <string>

int main() {
    std::cout << "test utf8 output:\n";
    std::string str1 = "テスト Россия";
    std::cout << "测试 ｱｲｳｴｵ 😅🤣" << " | " << str1 << '\n';

    std::cout << "test utf8 input:\n";
    auto input = WindowsConsole::utf8_input();
    if (input.has_value()) {
        std::cout << "input = \"" << input->c_str() << "\"\n";
        std::cout << "len = " << input->size() << '\n';
    }
    else { std::cout << "fail\n"; }

    return 0;
}
