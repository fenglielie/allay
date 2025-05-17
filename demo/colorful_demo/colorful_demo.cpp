#include "allay/colorful/colorful.hpp"
#include <iostream>

void print_colored(const std::string &msg, const ANSIColor &color) {
    std::cout << colored_str(msg, color) << '\n';
}

int main() {
    // 1. 使用 ANSI 16 色模式
    ANSI16Color red(ANSI16Color::ColorName::Red);
    ANSI16Color green(ANSI16Color::ColorName::Green);
    print_colored("ANSI 16 Red", red.to_ansi());
    print_colored("ANSI 16 Green", green.to_ansi());

    // 2. 使用 ANSI 256 色模式
    ANSI256Color color256(200);
    print_colored("ANSI 256 Color 200", color256.to_ansi());

    // 3. 使用 RGB 真彩色模式
    RGBColor rgb1(255, 0, 0);      // 红色
    RGBColor rgb2(0.0, 1.0, 0.0);  // 绿色（浮动）
    RGBColor rgb3("blue");         // 蓝色（名称）
    RGBColor rgb4("#FF5733");      // 十六进制色彩
    print_colored("True Color Red", rgb1.to_ansi());
    print_colored("True Color Green", rgb2.to_ansi());
    print_colored("True Color Blue", rgb3.to_ansi());
    print_colored("True Color #FF5733", rgb4.to_ansi());

    return 0;
}
