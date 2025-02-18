#include <format>
#include <regex>
#include <unordered_map>

struct ANSIColor {
    std::string prefix;
    std::string suffix;
};

// ANSI 16 色模式的枚举
class ANSI16Color {
public:
    enum class ColorName {
        Black,
        Red,
        Green,
        Yellow,
        Blue,
        Magenta,
        Cyan,
        White,
        BrightBlack,
        BrightRed,
        BrightGreen,
        BrightYellow,
        BrightBlue,
        BrightMagenta,
        BrightCyan,
        BrightWhite
    };

    ANSIColor to_ansi() const {
        return {.prefix = get_ansi_color_code(color_name), .suffix = "\033[0m"};
    }
    enum ColorName color_name;

private:
    // 获取 ANSI 16 色模式的转义代码
    static constexpr const char *get_ansi_color_code(ColorName color) {
        switch (color) {
        case ColorName::Black: return "\033[30m";
        case ColorName::Red: return "\033[31m";
        case ColorName::Green: return "\033[32m";
        case ColorName::Yellow: return "\033[33m";
        case ColorName::Blue: return "\033[34m";
        case ColorName::Magenta: return "\033[35m";
        case ColorName::Cyan: return "\033[36m";
        case ColorName::White: return "\033[37m";
        case ColorName::BrightBlack: return "\033[90m";
        case ColorName::BrightRed: return "\033[91m";
        case ColorName::BrightGreen: return "\033[92m";
        case ColorName::BrightYellow: return "\033[93m";
        case ColorName::BrightBlue: return "\033[94m";
        case ColorName::BrightMagenta: return "\033[95m";
        case ColorName::BrightCyan: return "\033[96m";
        case ColorName::BrightWhite: return "\033[97m";
        default: return "\033[0m";  // 重置
        }
    }
};

// 256 色模式的封装类
class ANSI256Color {
public:
    explicit ANSI256Color(int color_code) : m_code(color_code) {
        if (color_code < 0 || color_code > 255) {
            throw std::out_of_range(
                "ANSI 256 color code must be in range 0-255.");
        }
    }

    // 获取 ANSI 颜色代码
    ANSIColor to_ansi() const {
        return {.prefix = std::format("\033[38;5;{}m", m_code),
                .suffix = "\033[0m"};
    }

private:
    int m_code;
};

// 真彩色（24-bit 颜色）结构体
struct RGBColor {
    int r = 0;
    int g = 0;
    int b = 0;

    // 直接使用整数构造
    RGBColor(int red, int green, int blue) : r(red), g(green), b(blue) {
        validate();
    }

    // 使用归一化的浮点数构造 (0.0 ~ 1.0)
    RGBColor(double red, double green, double blue)
        : r(static_cast<int>(red * 255)), g(static_cast<int>(green * 255)),
          b(static_cast<int>(blue * 255)) {
        validate();
    }

    // 从颜色名称（字符串）构造
    explicit RGBColor(std::string color_str) {
        static const std::unordered_map<std::string, RGBColor> color_map = {
            {"black", {0, 0, 0}},           {"white", {255, 255, 255}},
            {"red", {255, 0, 0}},           {"green", {0, 255, 0}},
            {"blue", {0, 0, 255}},          {"yellow", {255, 255, 0}},
            {"cyan", {0, 255, 255}},        {"magenta", {255, 0, 255}},
            {"gray", {128, 128, 128}},      {"darkgray", {64, 64, 64}},
            {"lightgray", {192, 192, 192}}, {"orange", {255, 165, 0}},
            {"purple", {128, 0, 128}},      {"pink", {255, 192, 203}},
            {"brown", {165, 42, 42}},       {"lime", {50, 205, 50}},
            {"gold", {255, 215, 0}},        {"navy", {0, 0, 128}}};

        std::string tmp = color_str;
        std::transform(tmp.begin(), tmp.end(), color_str.begin(),
                       [](unsigned char c) { return std::tolower(c); });

        // 1. 先尝试匹配标准颜色名称
        auto it = color_map.find(color_str);
        if (it != color_map.end()) {
            *this = it->second;
            return;
        }

        // 2. 处理 #RRGGBB 形式
        std::regex hex_pattern("^#([A-Fa-f0-9]{6})$");
        std::smatch match;
        if (std::regex_match(color_str, match, hex_pattern)) {
            std::string hex_value = match[1].str();
            r = std::stoi(hex_value.substr(0, 2), nullptr, 16);
            g = std::stoi(hex_value.substr(2, 2), nullptr, 16);
            b = std::stoi(hex_value.substr(4, 2), nullptr, 16);
            validate();
            return;
        }

        throw std::invalid_argument("Unknown color str: " + color_str);
    }

    // 获取 ANSI 颜色代码
    ANSIColor to_ansi() const {
        return {.prefix = std::format("\033[38;2;{};{};{}m", r, g, b),
                .suffix = "\033[0m"};
    }

private:
    void validate() const {
        if (r < 0 || r > 255 || g < 0 || g > 255 || b < 0 || b > 255) {
            throw std::out_of_range("RGB values must be in range 0-255.");
        }
    }
};

// 生成彩色文本字符串
inline std::string colored_str(const std::string &msg, const ANSIColor &color) {
    return color.prefix + msg + color.suffix;
}
