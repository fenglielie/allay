#pragma once

#include <algorithm>
#include <functional>
#include <iomanip>
#include <iostream>
#include <optional>
#include <regex>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace cmd_parser_detail {

// 选项支持的类型
template <typename T>
concept Supported =
    std::disjunction_v<std::is_same<T, std::string>, std::is_same<T, bool>,
                       std::is_same<T, char>, std::is_same<T, int>,
                       std::is_same<T, double>, std::is_same<T, size_t>>;

template <Supported T>
static constexpr const char *get_type_id() {
    if constexpr (std::is_same_v<T, std::string>) { return "string"; }
    else if constexpr (std::is_same_v<T, bool>) { return "bool"; }
    else if constexpr (std::is_same_v<T, char>) { return "char"; }
    else if constexpr (std::is_same_v<T, int>) { return "int"; }
    else if constexpr (std::is_same_v<T, double>) { return "double"; }
    else if constexpr (std::is_same_v<T, std::size_t>) { return "size_t"; }
    else { return "???"; }
}

template <Supported T>
static std::string get_default_value_str(const T &var) {
    std::ostringstream ss;
    ss << var;
    return ss.str();
}

template <Supported T>
static std::string get_usage_str(const std::string &name) {
    std::ostringstream ss;
    ss << name << "=" << get_type_id<T>();
    return ss.str();
}

// 转换值
template <typename T>
static std::optional<T> case_string_to(std::string value) {
    if constexpr (std::is_same_v<T, std::string>) { return value; }
    else if constexpr (std::is_same_v<T, bool>) {
        std::ranges::transform(value, value.begin(), ::tolower);
        return (value == "true" || value == "yes" || value == "ok"
                || value == "on" || value == "1");
    }
    else {
        T result;
        std::stringstream ss(value);

        // bad cast
        if (!(ss >> result && ss.eof())) { return std::nullopt; }

        return result;
    }
}

// 合法长名称
inline bool is_valid_full_name(const std::string &name) {
    static const std::regex full_name_pattern(R"(^--[A-Za-z0-9_]{1,10}$)");
    return std::regex_match(name, full_name_pattern);
}

// 合法短名称，允许空
inline bool is_valid_short_name(const std::string &name) {
    static const std::regex short_name_pattern(R"(^-[A-Za-z0-9]?$)");
    return name.empty() || std::regex_match(name, short_name_pattern);
}

// 但是长短名称不允许全空

}  // namespace cmd_parser_detail

class CmdParser {
public:
    class Item {
    public:
        Item(std::string first, std::string second)
            : full_name(std::move(first)), short_name(std::move(second)) {
            validate();
        }

        // NOLINTNEXTLINE(google-explicit-constructor,hicpp-explicit-conversions)
        Item(std::string name) : full_name(std::move(name)) { validate(); }

        // NOLINTNEXTLINE(google-explicit-constructor,hicpp-explicit-conversions)
        Item(const char *name) : full_name(name) { validate(); }

        std::string to_str() const {
            return short_name.empty() ? full_name
                                      : (short_name + ", " + full_name);
        }

        std::string full_name;
        std::string short_name;

        // 重载 operator==
        bool operator==(const Item &other) const noexcept {
            return full_name == other.full_name
                   && short_name == other.short_name;
        }

        // 自定义哈希函数
        struct Hash {
            std::size_t operator()(const Item &item) const noexcept {
                std::size_t h1 = std::hash<std::string>{}(item.full_name);
                std::size_t h2 = std::hash<std::string>{}(item.short_name);
                return h1 ^ (h2 << 1);
            }
        };

    private:
        void validate() {
            if (!short_name.empty()
                && !cmd_parser_detail::is_valid_full_name(full_name)) {
                // 如果存在第二个参数，并且第一个作为全称选项检查失败，可以尝试交换
                std::swap(full_name, short_name);
            }

            if (!cmd_parser_detail::is_valid_full_name(full_name)) {
                throw std::invalid_argument("Invalid full name: " + full_name);
            }
            if (!cmd_parser_detail::is_valid_short_name(short_name)) {
                throw std::invalid_argument("Invalid short name: "
                                            + short_name);
            }
        }
    };

    // 执行回调函数
    CmdParser &add_flag(const Item &flag, const std::string &desc,
                        std::function<void()> caller) {
        check_and_update_names(flag);

        m_flags.insert({flag, FlagInfo{.caller = caller,
                                       .desc = desc,
                                       .usage_str = flag.full_name}});
        return *this;
    }

    // 不指定回调函数，只有计数功能
    CmdParser &add_flag(const Item &flag, const std::string &desc) {
        return add_flag(flag, desc, []() {});
    }

    // 添加选项，提供变量默认值，提供参数检查函数
    template <cmd_parser_detail::Supported T>
    CmdParser &add_option(const Item &option, const std::string &desc,
                          bool required, T default_value,
                          std::function<bool(T)> checker) {
        check_and_update_names(option);

        auto checker_wrapper = [checker](const std::string &value_str) {
            auto parsed = cmd_parser_detail::case_string_to<T>(value_str);
            if (!parsed.has_value()) { return false; }  // 转换失败
            return checker(parsed.value());             // 检查
        };

        std::string type_id = cmd_parser_detail::get_type_id<T>();
        std::string usage_str =
            cmd_parser_detail::get_usage_str<T>(option.full_name);
        std::string default_value_str =
            cmd_parser_detail::get_default_value_str<T>(default_value);

        m_options.insert({option, OptionInfo{
                                      .checker = checker_wrapper,
                                      .desc = desc,
                                      .usage_str = usage_str,
                                      .type_id = type_id,
                                      .default_value_str = default_value_str,
                                      .required = required,
                                      .value_list = {default_value_str},
                                  }});

        return *this;
    }

    // 添加选项，提供参数检查函数
    template <cmd_parser_detail::Supported T>
    CmdParser &add_option(const Item &option, const std::string &desc,
                          bool required, std::function<bool(T)> checker) {
        check_and_update_names(option);

        auto checker_wrapper = [checker](const std::string &value_str) {
            auto parsed = cmd_parser_detail::case_string_to<T>(value_str);
            if (!parsed.has_value()) { return false; }  // 转换失败
            return checker(parsed.value());             // 检查
        };

        std::string usage_str =
            cmd_parser_detail::get_usage_str<T>(option.full_name);
        std::string type_id = cmd_parser_detail::get_type_id<T>();

        m_options.insert({option, OptionInfo{
                                      .checker = checker_wrapper,
                                      .desc = desc,
                                      .usage_str = usage_str,
                                      .type_id = type_id,
                                      .default_value_str = "",
                                      .required = required,
                                      .value_list = {},
                                  }});

        return *this;
    }

    // 添加选项，提供变量默认值，自动生成参数检查函数
    template <cmd_parser_detail::Supported T>
    CmdParser &add_option(const Item &option, const std::string &desc,
                          bool required, T default_value) {
        return add_option<T>(option, desc, required, default_value,
                             [](T) { return true; });
    }

    // 添加选项，自动生成参数检查函数
    template <cmd_parser_detail::Supported T>
    CmdParser &add_option(const Item &option, const std::string &desc,
                          bool required) {
        return add_option<T>(option, desc, required, [](T) { return true; });
    }

    // 获取标志的计数，如果名称不存在，返回空
    std::optional<size_t> get_count(const std::string &name) const {
        if (const auto *p_flag = get_flag_via_name(name)) {
            return m_flags.at(*p_flag).count;
        }

        return std::nullopt;
    }

    // 获取选项最后设置的值，如果选项名称不存在，或者类型不匹配，返回空
    template <cmd_parser_detail::Supported T>
    std::optional<T> get_option(const std::string &name) {
        if (const auto *p_option = get_option_via_name(name)) {  // 要求名称存在
            std::string cur_type_id = cmd_parser_detail::get_type_id<T>();
            if (m_options.at(*p_option).type_id
                == cur_type_id) {  // 要求类型一致
                auto &value_list = m_options.at(*p_option).value_list;

                // 异常情况下会为空
                if (value_list.empty()) { return std::nullopt; }

                // 获取最后一个值
                auto last_value_str = value_list.back();
                return cmd_parser_detail::case_string_to<T>(last_value_str);
            }
        }
        return std::nullopt;
    }

    // 获取选项所有设置的值，如果选项名称不存在，或者类型不匹配，返回空
    template <cmd_parser_detail::Supported T>
    std::optional<std::vector<T>> get_option_all(const std::string &name) {
        if (const auto *p_option = get_option_via_name(name)) {  // 要求名称存在
            std::string cur_type_id = cmd_parser_detail::get_type_id<T>();
            if (m_options.at(*p_option).type_id
                == cur_type_id) {  // 要求类型一致
                auto &value_list = m_options.at(*p_option).value_list;

                // 异常情况下会为空，此时返回空列表
                if (value_list.empty()) { return std::vector<T>{}; }

                // 获取所有的值
                std::vector<T> result;
                for (auto &value_str : value_list) {
                    auto parsed = cmd_parser_detail::case_string_to<T>(value_str);
                    if (!parsed.has_value()) { return std::nullopt; }
                    result.push_back(parsed.value());
                }

                return result;
            }
        }
        return std::nullopt;
    }

    bool parse(int argc, char *argv[]) {
        // 如果解析之前没有设置程序名，则使用argv[0]作为程序名
        if (m_program_name.empty()) { set_program_name(argv[0]); }

        // 如果解析之前没有设置--help或-h，自动添加一个默认的--help选项
        if (!(m_unique_names.contains("--help")
              || m_unique_names.contains("-h"))) {
            add_flag({"--help", "-h"}, "print help message", [this]() {
                print_usage();
                exit(0);
            });
        }

        std::vector<std::string> args = expand_args(argc, argv);
        return parse_detail(args);
    }

    void parse_check(int argc, char *argv[]) {
        // 如果解析失败，则打印帮助信息并退出程序
        if (!parse(argc, argv)) {
            print_usage();

            // 如果完全没有提供参数，返回0，否则返回1
            if (argc == 1) { exit(0); }
            exit(1);
        }
    }

    // 打印帮助信息
    void print_usage() const {
        std::cout << "usage: " << m_program_name << " ";
        for (const auto &[option, info] : m_options) {
            if (info.required) { std::cout << info.usage_str << " "; }
        }
        std::cout << "...\n";

        std::vector<std::pair<std::string, std::string>> flag_descs;
        std::vector<std::pair<std::string, std::string>> option_descs;
        std::vector<std::pair<std::string, std::string>> required_option_descs;

        size_t max_length = 0;

        // 构造格式化字符串并计算最大长度用于对齐
        for (const auto &[option, info] : m_options) {
            std::string full_desc;
            if (!info.desc.empty()) {  // 有描述信息
                // 如果必要，并且存在默认值
                if (!(info.required || info.default_value_str.empty())) {
                    full_desc = info.desc + " (" + info.type_id
                                + " [=" + info.default_value_str + "])";
                }
                else { full_desc = info.desc + " (" + info.type_id + ")"; }
            }
            else { full_desc = "(" + info.type_id + ")"; }

            max_length = std::max(max_length, option.to_str().size());

            if (info.required) {
                required_option_descs.emplace_back(option.to_str(), full_desc);
            }
            else { option_descs.emplace_back(option.to_str(), full_desc); }
        }
        for (const auto &[flag, info] : m_flags) {
            max_length = std::max(max_length, flag.to_str().size());

            flag_descs.emplace_back(flag.to_str(), info.desc);
        }

        max_length += 4;  // 空格对齐

        // 输出选项和标志
        if (!required_option_descs.empty()) {
            std::cout << " @required options:\n";
        }
        for (const auto &[use_str, desc_str] : required_option_descs) {
            std::cout << "   " << std::left
                      << std::setw(static_cast<int>(max_length)) << use_str
                      << desc_str << '\n';
        }

        if (!option_descs.empty()) { std::cout << " @options:\n"; }
        for (const auto &[use_str, desc_str] : option_descs) {
            std::cout << "   " << std::left
                      << std::setw(static_cast<int>(max_length)) << use_str
                      << desc_str << '\n';
        }

        if (!option_descs.empty()) { std::cout << " @flags:\n"; }
        for (const auto &[use_str, desc_str] : flag_descs) {
            std::cout << "   " << std::left
                      << std::setw(static_cast<int>(max_length)) << use_str
                      << desc_str << '\n';
        }
    }

    // 获取多余参数（保持原有顺序）
    const std::vector<std::string> &get_rest() const { return m_rest; }

    // 设置程序名称（如果没有设置，在parse阶段会自动尝试获取）
    CmdParser &set_program_name(const std::string &program_name) {
        m_program_name = program_name;
        return *this;
    }

    // 解析时使用严格模式
    CmdParser &enable_strict_mode() {
        m_strict_mode = true;
        return *this;
    }

    // 解析时关闭严格模式
    CmdParser &disable_strict_mode() {
        m_strict_mode = false;
        return *this;
    }

private:
    struct OptionInfo {
        const std::function<bool(const std::string &)> checker;

        const std::string desc;               // 描述信息（允许空）
        const std::string usage_str;          // 使用示例
        const std::string type_id;            // 类型信息
        const std::string default_value_str;  // 默认值
        const bool required{false};           // 是否的必要参数

        std::vector<std::string>
            value_list;  // 支持多次设置，每次的值都会被记录
    };

    struct FlagInfo {
        const std::function<void()> caller;

        const std::string desc;       // 描述信息（允许空）
        const std::string usage_str;  // 使用示例

        size_t count{0};  // 计数出现次数
    };

    std::unordered_map<Item, FlagInfo, Item::Hash> m_flags;
    std::unordered_map<Item, OptionInfo, Item::Hash> m_options;
    std::set<std::string> m_unique_names;
    std::vector<std::string> m_rest;
    std::string m_program_name;
    bool m_strict_mode{false};

    // 将参数替换为字符串，对于连续短选项尝试展开
    std::vector<std::string> expand_args(int argc, char *argv[]) {
        std::vector<std::string> result;
        for (int i = 1; i < argc; ++i) {  // 跳过第一个参数，即程序名
            std::string arg = argv[i];

            // 首先基于等号划分键值对
            std::string name;
            std::string value;
            if (arg.find('=') != std::string::npos) {
                size_t equal_pos = arg.find('=');
                name = arg.substr(0, equal_pos);
                value = arg.substr(equal_pos + 1);
            }
            else { name = arg; }

            // 尝试进行拆分

            // (1)
            // 如果名称部分是一个合法的option名称（全称或缩写均可），有没有等号不影响
            // 例如-a，--ab，值可能是下一项，不需要进行拆分
            // 或者-a=1，--ab=1
            if (get_option_via_name(name) != nullptr) {
                result.push_back(name);
                if (!value.empty()) { result.push_back(value); }

                continue;
            }

            // (2)
            // 如果名称部分是一个合法的flag名称（全称或缩写均可），并且不含有等号
            // 例如-a，--ab，直接放进去即可，其实不需要拆分
            if (get_flag_via_name(name) != nullptr && value.empty()) {
                result.push_back(name);
                continue;
            }

            // 下面的拆分仅在不含等号，非严格模式下进行
            if (!m_strict_mode && value.empty()) {
                // (s1)
                // 如果名称部分是若干个合法的flag缩写名称的合并，将其分别拆开
                // 例如-abc 拆为 -a -b -c
                if (name.size() > 2 && name[0] == '-' && name[1] != '-') {
                    std::vector<std::string> expand_short_names;
                    bool expand_ok = true;
                    for (size_t j = 1; j < name.size(); ++j) {
                        auto short_name = "-" + std::string(1, name[j]);
                        if (get_flag_via_name(short_name) != nullptr) {
                            expand_short_names.push_back(short_name);
                        }
                        else { expand_ok = false; }
                    }

                    // 如果可以进行拆分
                    if (expand_ok) {
                        result.insert(result.end(), expand_short_names.begin(),
                                      expand_short_names.end());
                        continue;
                    }
                }

                // (s2)
                // 如果名称部分在option中存在唯一的前缀匹配，按照前缀匹配重新划分
                // 例如-l2拆为-l2，--ab2 拆为 --ab 2
                if ((get_unique_option_prefix(name)) != nullptr) {
                    const auto *p_matching_name =
                        get_unique_option_prefix(name);
                    value = name.substr(p_matching_name->size());

                    result.push_back(*p_matching_name);
                    result.push_back(value);
                    continue;
                }
            }

            // 其他情况下保持原样
            result.push_back(arg);
        }

        return result;
    }

    // 解析参数
    bool parse_detail(std::vector<std::string> args) noexcept {
        try {
            size_t arg_size = args.size();
            for (size_t i = 0; i < arg_size; ++i) {
                std::string arg = args[i];

                // 解析option
                if (const auto *p_option = get_option_via_name(arg)) {
                    if (i + 1 >= arg_size)
                        throw std::runtime_error("Missing value for option: "
                                                 + arg);

                    std::string value = args[++i];  // 获取选项的值

                    // 调用选项的检查函数
                    if (m_options.at(*p_option).checker(value)) {
                        m_options.at(*p_option).value_list.push_back(value);
                    }
                    else {  // 转换或检查失败
                        std::string msg = "Failed to set option with value: "
                                          + p_option->to_str() + " = " + value;
                        throw std::runtime_error(msg);
                    }
                }
                // 解析flag
                else if (const auto *p_flag = get_flag_via_name(arg)) {
                    m_flags.at(*p_flag).count++;  // 计数器自增

                    // 调用标志的回调函数
                    m_flags.at(*p_flag).caller();
                }
                else {  // 其它的多余参数，保持顺序地收集
                    m_rest.push_back(arg);
                }
            }

            // 检查必需的选项是否提供
            for (const auto &[option, info] : m_options) {
                if (info.required && (info.value_list.empty())) {
                    throw std::runtime_error("Missing required option: "
                                             + option.to_str());
                }
            }

            return true;
        }
        catch (const std::exception &e) {
            std::cerr << "CmdParser error: " << e.what() << '\n';
            return false;
        }
    }

    const Item *get_flag_via_name(const std::string &name) const {
        if (cmd_parser_detail::is_valid_full_name(name)) {
            for (const auto &[flag, _] : m_flags) {
                if (flag.full_name == name) { return &flag; }
            }
        }
        else if (cmd_parser_detail::is_valid_short_name(name)) {
            for (const auto &[flag, _] : m_flags) {
                if (flag.short_name == name) { return &flag; }
            }
        }
        return nullptr;
    }

    const Item *get_option_via_name(const std::string &name) const {
        if (cmd_parser_detail::is_valid_full_name(name)) {
            for (const auto &[option, _] : m_options) {
                if (option.full_name == name) { return &option; }
            }
        }
        else if (cmd_parser_detail::is_valid_short_name(name)) {
            for (const auto &[option, _] : m_options) {
                if (option.short_name == name) { return &option; }
            }
        }
        return nullptr;
    }

    void check_and_update_names(const Item &item) {
        if (m_unique_names.find(item.full_name) != m_unique_names.end()) {
            std::cerr << "CmdParser error: " << item.full_name
                      << " already exists.";
            exit(1);
        }
        else { m_unique_names.insert(item.full_name); }

        if (!item.short_name.empty()) {
            if (m_unique_names.find(item.short_name) != m_unique_names.end()) {
                std::cerr << "CmdParser error: " << item.short_name
                          << " already exists.";
                exit(1);
            }
            else { m_unique_names.insert(item.short_name); }
        }
    }

    // 查找并返回option范围内唯一匹配的前缀指针
    const std::string *get_unique_option_prefix(const std::string &str) const {
        const std::string *match_prefix = nullptr;
        size_t cnt = 0;
        for (const auto &[option, _] : m_options) {
            // 检查当前前缀是否是option的full_name的前缀
            if (str.find(option.full_name) == 0) {
                match_prefix = &(option.full_name);
                cnt += 1;
            }

            if (option.short_name.empty()) continue;

            // 检查当前前缀是否是option的short_name的前缀
            if (str.find(option.short_name) == 0) {
                match_prefix = &(option.short_name);
                cnt += 1;
            }
        }

        if (cnt == 1) { return match_prefix; }

        return nullptr;
    }
};
