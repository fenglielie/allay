#ifndef MLOGTOOL_H_
#define MLOGTOOL_H_

#include <chrono>
#include <iostream>
#include <mutex>  // IWYU pragma: keep
#include <regex>
#include <string>

class MLogTool {
public:
    constexpr static const char *ansi_color_red = "\x1b[91m";
    constexpr static const char *ansi_color_green = "\x1b[92m";
    constexpr static const char *ansi_color_yellow = "\x1b[93m";
    constexpr static const char *ansi_color_blue = "\x1b[94m";
    constexpr static const char *ansi_color_end = "\x1b[0m";

    // user: debug，info，warn，error
    enum class Level { on = 0, debug, info, warn, error, off };

    // supported format
    enum class LogStartFormat {
        LEVEL_SIGNATURE_TIME = 0,
        LEVEL_SIGNATURE,  // default
        LEVEL_TIME,
        LEVEL,
        LEVEL_COLOR,  // cout default
        NONE,
    };

    enum class OutType {
        N = 0,
        C,
        F,
        CF,
    };

    enum class ColorType {
        NONE = 0,
        RED,
        GREEN,
        BLUE,
        YELLOW,
    };

    class FirstN {
    private:
        const std::size_t m_first_n;
        std::size_t m_first_count_n{0};

    public:
        explicit FirstN(std::size_t first_n) : m_first_n(first_n) {}

        bool judge() {
            if (m_first_count_n < m_first_n) {
                ++m_first_count_n;
                return true;
            }
            return false;
        }

        std::size_t get_count() const { return m_first_count_n; }
    };

    class EveryN {
    private:
        const std::size_t m_every_n;
        std::size_t m_every_count_n{0};

    public:
        explicit EveryN(std::size_t every_n) : m_every_n(every_n) {}

        bool judge() { return ((m_every_count_n++) % m_every_n) == 0; }

        std::size_t get_count() const { return m_every_count_n; }
    };

    class MoreThanN {
    private:
        const std::size_t m_morethan_n;
        std::size_t m_morethan_count_n{0};

    public:
        explicit MoreThanN(std::size_t morethan_n) : m_morethan_n(morethan_n) {}

        bool judge() { return (m_morethan_count_n++ >= m_morethan_n); }

        std::size_t get_count() const { return m_morethan_count_n; }
    };

    static void set_level(MLogTool::Level level) {
#ifndef MLOG_USE_MACRO_LEVEL
        get_level_instance() = level;
#endif
    }

    static MLogTool::Level &get_level_instance() {
#ifndef MLOG_USE_MACRO_LEVEL
        static MLogTool::Level the_global_level{MLogTool::Level::on};
#else
        static MLogTool::Level the_global_level = MLOG_LEVEL;
#endif
        return the_global_level;
    }

    // 时间戳
    // 格式例如[2016-06-21 20:54:11:123]
    static std::string time_stamp() {
        auto now = std::chrono::system_clock::now();
        auto now_time_t = std::chrono::system_clock::to_time_t(now);
        auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                          now.time_since_epoch())
                      % 1000;

        char buffer[32]{};

        struct tm timeinfo {};

#if defined(_MSC_VER)
        localtime_s(&timeinfo, &now_time_t);
#elif defined(__unix__)
        localtime_r(&now_time_t, &timeinfo);
#else
        static std::mutex mtx;
        {
            std::lock_guard<std::mutex> lock(mtx);
            timeinfo = *localtime(&now_time_t);
        }
#endif

        std::strftime(static_cast<char *>(buffer), sizeof(buffer),
                      "[%Y-%m-%d %H:%M:%S", &timeinfo);

        std::snprintf(static_cast<char *>(buffer) + 20, 7, ".%03d]",
                      static_cast<int>(now_ms.count()));

        return std::string{static_cast<char *>(buffer)};
    }

    // 等级输出
    // 例如[DEBUG]
    static const std::string &level_stamp(Level level) {
        static const std::string debug_str{"[DEBUG]"};
        static const std::string info_str{"[INFO]"};
        static const std::string warn_str{"[WARN]"};
        static const std::string error_str{"[ERROR]"};
        static const std::string on_str{"[     ]"};
        static const std::string off_str{"[-----]"};

        switch (level) {
        case Level::debug: return debug_str;
        case Level::info: return info_str;
        case Level::warn: return warn_str;
        case Level::error: return error_str;
        case Level::off: return off_str;
        default: return on_str;
        }
    }

    // 时间字符串 可用于日志文件名
    // 例如01-25-21-33
    static std::string date_string() {
        auto now = std::chrono::system_clock::now();
        auto now_time_t = std::chrono::system_clock::to_time_t(now);

        char buffer[32]{};

        struct tm timeinfo {};

#if defined(_MSC_VER)
        localtime_s(&timeinfo, &now_time_t);
#elif defined(__unix__)
        localtime_r(&now_time_t, &timeinfo);
#else
        static std::mutex mtx;
        {
            std::lock_guard<std::mutex> lock(mtx);
            timeinfo = *localtime(&now_time_t);
        }
#endif

        std::strftime(static_cast<char *>(buffer), sizeof(buffer),
                      "%m-%d-%H-%M-%S", &timeinfo);

        return std::string{static_cast<char *>(buffer)};
    }

    // 判断文件名合法
    // Windows下文件名中不能包含\/:*?"<>|这些字符
    // 并且文件名长度这里不允许超过100
    static bool check_filename_valid(const std::string &file_name) {
        std::regex reg_express("[\\/:*?\"<>|]");
        return (!std::regex_search(file_name, reg_express)
                && (file_name.size() <= 100));
    }

    static void raise_error() {
        std::cerr << "MLog: The program can not perform as expected!";
        exit(1);
    }
};

#endif  // MLOGTOOL_H_
