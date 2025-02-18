#pragma once

#include <algorithm>
#include <chrono>
#include <cmath>
#include <format>
#include <iostream>
#include <memory>
#include <string>
#include <utility>

// 输出基类
class Formatter {
public:
    struct Args {
        double pct;
        double time_cost;
        double time_left;
        double rate;
        bool color_enabled;
    };

    virtual ~Formatter() = default;

    virtual std::string showline(Args args) {
        if (args.color_enabled) {
            return color_str(std::format("{:6.2f}%", args.pct * 100), args.pct,
                             GradientType::Energy);
        }

        return std::format("{:6.2f}%", args.pct * 100);
    }

    static std::string time_str(double dt) {
        if (dt < 3600) {
            // 小于 1 小时，显示 mm:ss
            int minutes = static_cast<int>(dt) / 60;
            int seconds = static_cast<int>(dt) % 60;
            return std::format("{:02}:{:02}", minutes, seconds);
        }
        if (dt < 86400) {
            // 小于 24 小时，显示 hh:mm:ss
            int hours = static_cast<int>(dt) / 3600;
            int minutes = (static_cast<int>(dt) % 3600) / 60;
            int seconds = static_cast<int>(dt) % 60;
            return std::format("{:02}:{:02}:{:02}", hours, minutes, seconds);
        }
        // 超过 24 小时，直接显示 >xxh
        return std::format(">{}h", static_cast<int>(dt / 3600));
    }

    // 颜色渐变类别
    enum class GradientType {
        Heatmap,    // 红 -> 黄 -> 绿
        Cool,       // 蓝 -> 紫 -> 粉
        Grayscale,  // 深灰 -> 亮灰 -> 白
        Energy      // 红 -> 紫 -> 蓝
    };

    // 生成彩色字符串
    static std::string color_str(const std::string &msg, double pct,
                                 GradientType type) {
        int r = 0;
        int g = 0;
        int b = 0;

        switch (type) {
        case GradientType::Heatmap: {  // 红 -> 黄 -> 绿
            if (pct < 0.5) {
                r = 255;
                g = static_cast<int>(255 * (pct / 0.5));
                b = 0;
            }
            else {
                r = static_cast<int>(255 * (1 - (pct - 0.5) / 0.5));
                g = 255;
                b = 0;
            }
            break;
        }
        case GradientType::Cool: {  // 蓝 -> 紫 -> 粉
            if (pct < 0.5) {
                r = static_cast<int>(128 * (pct / 0.5));
                g = 0;
                b = static_cast<int>(255 - (127 * (pct / 0.5)));
            }
            else {
                r = static_cast<int>(128 + 127 * ((pct - 0.5) / 0.5));
                g = static_cast<int>(105 * ((pct - 0.5) / 0.5));
                b = static_cast<int>(180 * ((pct - 0.5) / 0.5));
            }
            break;
        }
        case GradientType::Grayscale: {  // 深灰 -> 亮灰 -> 白
            int value = static_cast<int>(50 + 205 * pct);
            r = g = b = value;
            break;
        }
        case GradientType::Energy: {  // 红 -> 紫 -> 蓝
            if (pct < 0.5) {
                r = static_cast<int>(255 * (1 - (pct / 0.5)));
                g = 0;
                b = static_cast<int>(255 * (pct / 0.5));
            }
            else {
                r = static_cast<int>(128 * (1 - (pct - 0.5) / 0.5));
                g = 0;
                b = 255;
            }
            break;
        }
        default: throw std::invalid_argument("Invalid gradient type");
        }

        return std::format("\033[38;2;{};{};{}m{}\033[0m", r, g, b, msg);
    }
};

class LongFormatter : public Formatter {
public:
    LongFormatter(size_t len, std::vector<std::string> status_strs,
                  std::vector<std::string> active_strs)
        : m_len(len), m_active_strs(std::move(active_strs)) {
        status_prepare(status_strs);
        bar_prepare();
    }

    LongFormatter()
        : LongFormatter(20, {"#", " ", "|"}, {"|", "/", "-", "\\"}) {};

    ~LongFormatter() override = default;

    std::string showline(Args args) override {
        if (args.color_enabled) {
            return std::format(
                "\033[91m{:6.2f}%\033[0m {} \033[93m[{}<{}]\033[0m",
                args.pct * 100,
                color_str(pct_bar(args.pct), args.pct, GradientType::Heatmap),
                time_str(args.time_cost), time_str(args.time_left));
        }
        return std::format("{:6.2f}% {} [{}<{}]", args.pct * 100,
                           pct_bar(args.pct), time_str(args.time_cost),
                           time_str(args.time_left));
    }

    static std::shared_ptr<LongFormatter> create(size_t len, int style) {
        switch (style) {
        case 1:
            return std::shared_ptr<LongFormatter>(
                new LongFormatter(len, {"⣿", " ", "|", "|"},
                                  {" ", "⡀", "⡄", "⡆", "⡇", "⡏", "⡟", "⡿"}));

        case 2:
            return std::shared_ptr<LongFormatter>(
                new LongFormatter(len, {"█", " ", "▏", "▕"},
                                  {" ", "▏", "▎", "▍", "▌", "▋", "▊", "▉"}));
        case 3:
            return std::shared_ptr<LongFormatter>(
                new LongFormatter(len, {"█", " ", "▏", "▕"},
                                  {"▁", "▂", "▃", "▄", "▅", "▆", "▇", "█"}));
        case 0:
        default:
            return std::shared_ptr<LongFormatter>(
                new LongFormatter(len, {"#", " ", "|"}, {"|", "/", "-", "\\"}));
        }
    }

private:
    const size_t m_len = 20;
    std::string m_fill_str;
    std::string m_empty_str;
    std::string m_start_str;
    std::string m_end_str;
    const std::vector<std::string> m_active_strs;

    std::vector<std::string> m_lbars;
    std::vector<std::string> m_rbars;

    void status_prepare(std::vector<std::string> status_strs) {
        size_t len = status_strs.size();
        switch (len) {
        case 0:
            m_fill_str = "#";
            m_empty_str = " ";
            m_end_str = "|";
            m_start_str = "";
            break;
        case 1:
            m_fill_str = status_strs[0];
            m_empty_str = " ";
            m_end_str = "|";
            m_start_str = "";
            break;
        case 2:
            m_fill_str = status_strs[0];
            m_empty_str = status_strs[1];
            m_end_str = "|";
            m_start_str = "";
            break;
        case 3:
            m_fill_str = status_strs[0];
            m_empty_str = status_strs[1];
            m_end_str = status_strs[2];
            m_start_str = status_strs[2];
            break;
        default:
            m_fill_str = status_strs[0];
            m_empty_str = status_strs[1];
            m_end_str = status_strs[2];
            m_start_str = status_strs[3];
            break;
        }
    }

    void bar_prepare() {
        m_lbars.reserve(m_len + 1);
        m_lbars.emplace_back("");

        m_rbars.reserve(m_len + 1);
        m_rbars.emplace_back("");
        m_rbars.emplace_back("");

        std::string tmp1;
        std::string tmp2;
        for (size_t i = 1; i <= m_len; ++i) {
            tmp1 += m_fill_str;
            tmp2 += m_empty_str;

            m_lbars.push_back(tmp1);
            m_rbars.push_back(tmp2);
        }
    }

    std::string pct_bar(double pct) {
        size_t n =
            std::clamp(static_cast<size_t>(pct * static_cast<double>(m_len)),
                       static_cast<size_t>(0), m_len);

        double block_pct = 1.0 / static_cast<double>(m_len);
        double rest_pct = pct - block_pct * static_cast<int>(n);
        std::string last_str;

        if (!m_active_strs.empty() && n < m_len) {
            auto idx =
                static_cast<size_t>(static_cast<double>(m_active_strs.size())
                                    * rest_pct / block_pct);
            last_str = m_active_strs[idx];
        }

        return m_start_str + m_lbars[n] + last_str + m_rbars[m_len - n]
               + m_end_str;
    }
};

class Pbar {
public:
    // 数据记录和更新类
    class DataUpdater {
    public:
        DataUpdater()
            : m_start_time(std::chrono::steady_clock::now()),
              m_last_time(m_start_time) {}

        bool update(double pct) {
            if (pct < m_last_pct) { return false; }

            auto time_now = std::chrono::steady_clock::now();

            // Time prediction
            auto duration =
                std::chrono::duration_cast<std::chrono::milliseconds>(
                    time_now - m_last_time);
            double elapsed_time =
                static_cast<double>(duration.count()) / 1000.0;

            // Progress rate
            double cur_rate = (pct - m_last_pct) / elapsed_time;

            // If m_last_rate is illegal, use cur_rate only.
            bool illegal_flag =
                std::isinf(m_last_rate) || std::isnan(m_last_rate);

            if (!m_rate_setted || illegal_flag) {
                m_last_rate = cur_rate;
                m_rate_setted = true;
            }
            else {  // Apply exponential smoothing
                m_last_rate = alpha * cur_rate + (1 - alpha) * m_last_rate;
            }

            // Update
            m_last_time = time_now;
            m_last_pct = pct;

            return true;
        }

        double get_time_cost() const {
            auto cost_duration =
                std::chrono::duration_cast<std::chrono::milliseconds>(
                    m_last_time - m_start_time);
            double cost_time =
                static_cast<double>(cost_duration.count()) / 1000.0;

            return cost_time;  // seconds
        }

        double get_time_left() const {
            if (m_rate_setted) { return (1.0 - m_last_pct) / m_last_rate; }

            return 0;
        }

    private:
        static constexpr double alpha = 0.4;  // Smoothing factor

        const std::chrono::steady_clock::time_point m_start_time;

        std::chrono::steady_clock::time_point m_last_time;
        double m_last_pct = 0;
        double m_last_rate = 0.0;

        bool m_rate_setted = false;

        friend class Pbar;
    };

    Pbar()
        : Pbar(std::make_shared<Formatter>(), std::make_shared<DataUpdater>()) {
    }

    explicit Pbar(std::shared_ptr<Formatter> ptr_formatter)
        : Pbar(ptr_formatter, std::make_shared<DataUpdater>()) {}

    Pbar(std::shared_ptr<Formatter> ptr_formatter,
         std::shared_ptr<DataUpdater> ptr_updater)
        : m_ptr_formatter(std::move(ptr_formatter)),
          m_ptr_updater(std::move(ptr_updater)) {}

    void update(double arg) {
        if (m_ptr_formatter == nullptr) { return; }

        m_ptr_updater->update(arg);

        if (m_ptr_formatter != nullptr) {
            auto msg = m_ptr_formatter->showline(
                {.pct = m_ptr_updater->m_last_pct,
                 .time_cost = m_ptr_updater->get_time_cost(),
                 .time_left = m_ptr_updater->get_time_left(),
                 .rate = m_ptr_updater->m_last_rate,
                 .color_enabled = m_color_enabled});

            std::cout << std::format("\r {}{}", m_desc, msg) << std::flush;
        }
    }

    static void nextline() { std::cout << std::endl; }

    Pbar &enable_color() {
        m_color_enabled = true;
        return *this;
    }

    Pbar &disable_color() {
        m_color_enabled = false;
        return *this;
    }

    Pbar &set_desc(const std::string &desc) {
        m_desc = desc;
        return *this;
    }

    double get_time_cost() const {
        if (m_ptr_formatter == nullptr) { return 0; }

        return m_ptr_updater->get_time_cost();
    }

private:
    std::shared_ptr<Formatter> m_ptr_formatter;
    std::shared_ptr<DataUpdater> m_ptr_updater;

    std::string m_desc;
    bool m_color_enabled = false;
};
