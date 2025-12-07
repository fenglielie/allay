#pragma once

#include <algorithm>
#include <chrono>
#include <cmath>
#include <format>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

class ColorGradient {
public:
    struct RGB {
        int r;
        int g;
        int b;
    };

    ColorGradient() = default;

    explicit ColorGradient(std::vector<RGB> colors)
        : m_colors(std::move(colors)) {}

    explicit ColorGradient(RGB color) : ColorGradient(std::vector{color}) {}

    std::string prefix(double pct) const {
        pct = std::clamp(pct, 0.0, 1.0);

        if (m_colors.empty()) { return ""; }

        if (m_colors.size() == 1) { return rgb_to_ansi(m_colors[0]); }

        double seg = 1.0 / static_cast<double>(m_colors.size() - 1);
        size_t idx =
            std::min(static_cast<size_t>(pct / seg), m_colors.size() - 2);

        double local_t =
            static_cast<double>(pct - static_cast<double>(idx) * seg) / seg;

        RGB c1 = m_colors[idx];
        RGB c2 = m_colors[idx + 1];

        RGB c = interpolate(c1, c2, local_t);
        return rgb_to_ansi(c);
    }

    static std::string suffix() { return "\033[0m"; }

    std::string colorful(std::string msg, double pct) const {
        return prefix(pct) + msg + suffix();
    }

    // red -> yellow -> green
    static ColorGradient Heat() {
        return ColorGradient({{255, 0, 0},    // red
                              {255, 255, 0},  // yellow
                              {0, 255, 0}}    // green
        );
    }

    // red -> purple -> blue
    static ColorGradient Energy() {
        return ColorGradient({
            {255, 0, 0},    // red
            {180, 0, 180},  // purple
            {0, 128, 255}   // blue
        });
    }

    // deep blue -> cyan -> light green
    static ColorGradient Ocean() {
        return ColorGradient({
            {0, 32, 96},    // deep blue
            {0, 128, 192},  // cyan
            {0, 255, 128}   // light green
        });
    }

private:
    std::vector<RGB> m_colors;

    static RGB interpolate(const RGB &c1, const RGB &c2, double t) {
        return {
            static_cast<int>(c1.r + (c2.r - c1.r) * t),
            static_cast<int>(c1.g + (c2.g - c1.g) * t),
            static_cast<int>(c1.b + (c2.b - c1.b) * t),
        };
    }

    static std::string rgb_to_ansi(const RGB &c) {
        return "\033[38;2;" + std::to_string(c.r) + ";" + std::to_string(c.g)
               + ";" + std::to_string(c.b) + "m";
    }
};

struct ProgressStyle {
    size_t length = 20;
    std::string fill = "#";
    std::string empty = " ";
    std::string left = "|";
    std::string right = "|";
    std::vector<std::string> active;

    std::string prefix_desc;
    std::string suffix_desc;

    ColorGradient color_of_pct;
    ColorGradient color_of_bar;
    ColorGradient color_of_time;

    static ProgressStyle Classic() {
        return {.length = 20,
                .fill = "#",
                .empty = " ",
                .left = "|",
                .right = "|",
                .active = std::vector<std::string>{"|", "/", "-", "\\"},
                .prefix_desc = {},
                .suffix_desc = {},
                .color_of_pct = {},
                .color_of_bar = {},
                .color_of_time = {}};
    }

    static ProgressStyle Block() {
        return {.length = 20,
                .fill = "█",
                .empty = " ",
                .left = "▕",
                .right = "▏",
                .active = std::vector<std::string>{" ", "▏", "▎", "▍", "▌", "▋",
                                                   "▊", "▉"},
                .prefix_desc = {},
                .suffix_desc = {},
                .color_of_pct = {},
                .color_of_bar = {},
                .color_of_time = {}};
    }

    static ProgressStyle Braille() {
        return {.length = 20,
                .fill = "⣿",
                .empty = " ",
                .left = "|",
                .right = "|",
                .active = std::vector<std::string>{" ", "⡀", "⡄", "⡆", "⡇", "⡏",
                                                   "⡟", "⡿"},
                .prefix_desc = {},
                .suffix_desc = {},
                .color_of_pct = {},
                .color_of_bar = {},
                .color_of_time = {}};
    }
};

class DataUpdater {
public:
    DataUpdater()
        : m_start(std::chrono::steady_clock::now()), m_last(m_start) {}

    bool update(double pct) {
        if (pct < m_last_pct) return false;

        auto now = std::chrono::steady_clock::now();
        double dt = static_cast<double>(
                        std::chrono::duration_cast<std::chrono::milliseconds>(
                            now - m_last)
                            .count())
                    / 1000.0;

        double cur_rate = (pct - m_last_pct) / dt;

        bool bad = std::isnan(m_rate) || std::isinf(m_rate);
        if (!m_rate_ok || bad) {
            m_rate = cur_rate;
            m_rate_ok = true;
        }
        else { m_rate = alpha * cur_rate + (1 - alpha) * m_rate; }

        m_last = now;
        m_last_pct = pct;
        return true;
    }

    double cost() const {
        auto d = std::chrono::duration_cast<std::chrono::milliseconds>(
            m_last - m_start);
        return static_cast<double>(d.count()) / 1000.0;
    }

    double left() const {
        if (!m_rate_ok) return 0;
        return (1.0 - m_last_pct) / m_rate;
    }

    double last_pct() const { return m_last_pct; }

    double last_rate() const { return m_rate; }

private:
    static constexpr double alpha = 0.4;

    std::chrono::steady_clock::time_point m_start;
    std::chrono::steady_clock::time_point m_last;

    double m_last_pct = 0;
    double m_rate = 0;
    bool m_rate_ok = false;
};

class Progress {
public:
    Progress() : Progress(ProgressStyle::Classic()) {}

    explicit Progress(ProgressStyle st) : m_st(std::move(st)) {
        m_lbars.reserve(m_st.length + 1);
        m_lbars.emplace_back("");
        m_rbars.reserve(m_st.length + 1);
        m_rbars.emplace_back("");
        m_rbars.emplace_back("");

        std::string tmp1;
        std::string tmp2;
        for (size_t i = 1; i <= m_st.length; i++) {
            tmp1 += m_st.fill;
            tmp2 += m_st.empty;
            m_lbars.push_back(tmp1);
            m_rbars.push_back(tmp2);
        }
    }

    void update(double pct) {
        m_upt.update(pct);

        auto msg = showline(m_upt.last_pct(), m_upt.cost(), m_upt.left(),
                            m_upt.last_rate());

        std::cout << std::format("\r {}{}{}", m_st.prefix_desc, msg,
                                 m_st.suffix_desc)
                  << std::flush;
    }

    static void nextline() { std::cout << "\n"; }

private:
    DataUpdater m_upt;
    ProgressStyle m_st;
    std::vector<std::string> m_lbars;
    std::vector<std::string> m_rbars;

    std::string pct_bar(double pct) {
        size_t len = m_st.length;
        size_t n =
            std::clamp(static_cast<size_t>(pct * static_cast<double>(len)),
                       static_cast<size_t>(0), len);

        double block_pct = 1.0 / static_cast<double>(len);
        double rest_pct = pct - block_pct * static_cast<int>(n);
        std::string last_str;

        if (!m_st.active.empty() && n < len) {
            auto idx = static_cast<size_t>(
                static_cast<double>(m_st.active.size()) * rest_pct / block_pct);
            last_str = m_st.active[idx];
        }

        return m_st.left + m_lbars[n] + last_str + m_rbars[len - n]
               + m_st.right;
    }

    static std::string time_str(double dt) {
        if (dt < 3600) {
            return std::format("{:02}:{:02}", static_cast<int>(dt / 60),
                               static_cast<int>(dt) % 60);
        }
        if (dt < 86400) {
            return std::format("{:02}:{:02}:{:02}", static_cast<int>(dt / 3600),
                               (static_cast<int>(dt) % 3600) / 60,
                               static_cast<int>(dt) % 60);
        }
        return std::format(">{}h", static_cast<int>(dt / 3600));
    }

    std::string showline(double pct, double time_cost, double time_left,
                         double rate) {
        auto bar = pct_bar(pct);

        auto result = std::format(
            "{}{:6.2f}%{} {}{}{} {}[{}<{}]{}", m_st.color_of_pct.prefix(pct),
            pct * 100, ColorGradient::suffix(), m_st.color_of_bar.prefix(pct),
            bar, ColorGradient::suffix(), m_st.color_of_time.prefix(pct),
            time_str(time_cost), time_str(time_left), ColorGradient::suffix());

        return result;
    }
};
