#ifndef MLOGGER_H_
#define MLOGGER_H_

#include "mlogtool.hpp"

#include "mlogfilemanager.hpp"

#include <fstream>
#include <iostream>
#include <string>

class MLogger {
public:
    using Format = MLogTool::LogStartFormat;
    using Level = MLogTool::Level;
    using Out = MLogTool::OutType;
    using Color = MLogTool::ColorType;

    //----------------------------------------------------------------------------//
    // 直接对外暴露的接口

    // Part 1, 输出路径和文件流操作

    // 如果当前打开了日志文件则做一些收尾工作，包括close以及移除这个ofstream
    MLogger &link_cout() {
        return if_unlock().clean_file_and_ofstream(true).set_flags(Out::C);
    }

    // 丢弃所有的输出
    MLogger &link_none() {
        return if_unlock().clean_file_and_ofstream(true).set_flags(Out::N);
    }

    MLogger &link_file_app(const std::string &file_name) {
        return if_unlock().link_file_detail(file_name, std::ios_base::app);
    }

    MLogger &link_file_trunc(const std::string &file_name) {
        return if_unlock().link_file_detail(file_name, std::ios_base::trunc);
    }

    // 指定文件流为默认日志文件，这里只是省略了文件名和打开方式参数的填写
    MLogger &link_file_default() {
        std::string filename = MLogTool::date_string() + "." + m_name + ".log";
        return if_unlock().link_file_detail(filename, std::ios_base::app);
    }

    // Part 2. 输出开关选项修改

    // 锁定，不可以改变输出流状态
    MLogger &lock() {
        m_lock = true;
        return (*this);
    }

    // 解锁，可以改变输出流状态
    MLogger &unlock() {
        m_lock = false;
        return (*this);
    }

    // 在未锁定时，只对cout输出
    MLogger &enable_cout_only() { return if_unlock().set_flags(Out::C); }

    // 在未锁定时，只对文件流输出
    MLogger &enable_file_only() { return if_unlock().set_flags(Out::F); }

    // 在未锁定时，同时对cout和文件流输出
    MLogger &enable_file_and_cout() { return if_unlock().set_flags(Out::CF); }

    //----------------------------------------------------------------------------//
    // 对外输出
    // 以下模板和实例化方案参考c++ primer

    // 这个最通用模板负责所有无法处理的类型
    template <typename MessageType>
    MLogger &operator<<(const MessageType &msg) {
        if (!m_output_flag) { return *this; }  // 永久关闭的话直接返回

        if (m_use_cout_flag) { std::cout << msg; }  // 对接到cout
        if (m_use_file_flag && (m_logfile_ofstream != nullptr)) {
            (*m_logfile_ofstream) << msg;
        }  // 对接到文件流
        return *this;
    }

    // 对于一般的指针，打印它的内容
    template <typename MessageType>
    MLogger &operator<<(MessageType *msg_str) {
        if (!m_output_flag) { return *this; }  // 永久关闭的话直接返回

        if (m_use_cout_flag) { std::cout << (*msg_str); }  // 对接到cout
        if (m_use_file_flag && (m_logfile_ofstream != nullptr)) {
            (*m_logfile_ofstream) << (*msg_str);
        }  // 对接到文件流
        return *this;
    }

    // 对于string字符串，直接处理
    MLogger &operator<<(const std::string &msg_str) {
        if (!m_output_flag) { return *this; }  // 永久关闭的话直接返回

        if (m_use_cout_flag) { std::cout << msg_str; }  // 对接到cout
        if (m_use_file_flag && (m_logfile_ofstream != nullptr)) {
            (*m_logfile_ofstream) << msg_str;
        }  // 对接到文件流
        return *this;
    }

    // 对于char*风格的，转换成string处理
    MLogger &operator<<(char *msg_str_raw) {
        return operator<<(std::string(msg_str_raw));
    }

    // 对于char*风格的，转换成string处理
    MLogger &operator<<(const char *msg_str_raw) {
        return operator<<(std::string(msg_str_raw));
    }

    using CoutType = std::basic_ostream<char, std::char_traits<char>>;
    using StandardEndLineType = CoutType &(*)(CoutType &);

    // 为了支持std::endl的额外处理
    MLogger &operator<<(StandardEndLineType func) {
        if (!m_output_flag) { return *this; }  // 永久关闭的话直接返回

        if (m_use_cout_flag) func(std::cout);
        if (m_use_file_flag) { func(*m_logfile_ofstream); }

        return *this;
    }

    //----------------------------------------------------------------------------//

    MLogger &flush() {
        // 无论是否对接到cout
        std::cout.flush();
        // 无论flag是否对接到文件流，只要可以访问这个流
        if (m_logfile_ofstream != nullptr) { m_logfile_ofstream->flush(); }
        return *this;
    }

    //----------------------------------------------------------------------------//

    MLogger &set_format(Format log_format) {
        m_log_start_format = log_format;
        return (*this);
    }

    MLogger &log_red(const std::string &message) {
        return log_with_color(message, Color::RED);
    }

    MLogger &log_green(const std::string &message) {
        return log_with_color(message, Color::GREEN);
    }

    MLogger &log_blue(const std::string &message) {
        return log_with_color(message, Color::BLUE);
    }

    MLogger &log_yellow(const std::string &message) {
        return log_with_color(message, Color::YELLOW);
    }

    MLogger(const MLogger &) = delete;
    MLogger &operator=(const MLogger &) = delete;

    // 默认模式 外部创造出来的都是不具名的
    // 从外部进行创造有隐患，因为析构时可能文件句柄没有去除
    MLogger() = default;

    // 可能MLogFileCenter析构在它之前，所以析构不能负责文件关闭
    // 因为只允许mloggermanager进行构造，并且单例是局部静态的，这里也同样析构只能发生在main函数之后
    ~MLogger() { clean_file_and_ofstream(false); }

    //----------------------------------------------------------------------------//
private:
    friend class MLoggerManager;

    // 核心方法，不负责检查是否锁定，这是内层方法
    // 改变文件流为其它文件流，并负责打开文件
    // 失败则绑定nullptr
    // 析构时不会负责关闭，由MLogFileManager的单例负责逐个关闭
    MLogger &link_file_detail(const std::string &file_name,
                              const std::ios_base::openmode mode) {
        // 妥善收尾
        clean_file_and_ofstream(true);

        auto full_file_name = MLogFileManager::get_path_prefix() + file_name;

        // 尝试获取新文件流，不负责检查合法性和唯一性
        m_logfile_ofstream = MLogFileManager::get_unique_ofstream(file_name);

        // 没有获取到文件句柄，报错退出
        if (!m_logfile_ofstream) {
            set_flags(Out::C)
                .clean_file_and_ofstream(true)
                .notice_open_file_failed_and_exit(full_file_name);
        }

        // 获取了新的文件句柄，
        // 尝试打开文件
        m_logfile_ofstream->open(full_file_name, std::ios_base::out | mode); // NOLINT(hicpp-signed-bitwise)

        // 没有打开文件，报错退出
        if (m_logfile_ofstream->fail()) {
            set_flags(Out::C)
                .clean_file_and_ofstream(true)
                .notice_open_file_failed_and_exit(full_file_name);
        }

        m_file_name = file_name;  // 记录更新日志文件名
        // 成功打开新的日志文件，在写日志之前加入固定的前缀
        return set_flags(Out::F).notice_open_file();
    }

    MLogger &log_with_color(const std::string &message, Color color) {
        switch (color) {
        case Color::RED:
            return log_with_color_detail(message, MLogTool::ansi_color_red,
                                         MLogTool::ansi_color_end);
        case MLogTool::ColorType::GREEN:
            return log_with_color_detail(message, MLogTool::ansi_color_green,
                                         MLogTool::ansi_color_end);
        case MLogTool::ColorType::YELLOW:
            return log_with_color_detail(message, MLogTool::ansi_color_yellow,
                                         MLogTool::ansi_color_end);
        case MLogTool::ColorType::BLUE:
            return log_with_color_detail(message, MLogTool::ansi_color_blue,
                                         MLogTool::ansi_color_end);
        case MLogTool::ColorType::NONE:
        default: return (*this);
        }
    }

    // 颜色输出需要前缀和后缀，但向文件中输出会自动删除这些特殊字符
    MLogger &log_with_color_detail(const std::string &message,
                                   const char *color_prefix,
                                   const char *color_suffix) {
        if (!m_output_flag) return (*this);

        if (m_use_cout_flag) {
            std::cout << color_prefix << message << color_suffix;
        }

        if (m_use_file_flag && (m_logfile_ofstream != nullptr)) {
            (*m_logfile_ofstream) << message;
        }

        return (*this);
    }

    // 外部指定格式
    MLogger &log_start(Level level, Format log_start_format) {
        switch (log_start_format) {
        case Format::LEVEL_SIGNATURE_TIME:
            return (*this) << MLogTool::level_stamp(level) << m_signature
                           << MLogTool::time_stamp();
        case Format::LEVEL_SIGNATURE:
            return (*this) << MLogTool::level_stamp(level) << m_signature;
        case Format::LEVEL_TIME:
            return (*this) << MLogTool::level_stamp(level)
                           << MLogTool::time_stamp();
        case Format::LEVEL: return (*this) << MLogTool::level_stamp(level);
        case Format::NONE: return (*this);
        case Format::LEVEL_COLOR:
            switch (level) {
            case Level::debug:
                return log_with_color(MLogTool::level_stamp(level),
                                      Color::BLUE);
            case Level::info:
                return log_with_color(MLogTool::level_stamp(level),
                                      Color::GREEN);
            case Level::warn:
                return log_with_color(MLogTool::level_stamp(level),
                                      Color::YELLOW);
            case Level::error:
                return log_with_color(MLogTool::level_stamp(level), Color::RED);
            case Level::on:
            case Level::off:
            default: return (*this) << MLogTool::level_stamp(level);
            }
        default: return (*this);
        }
    }

    // 使用自带的格式
    MLogger &log_start(Level level) {
        return log_start(level, m_log_start_format);
    }

    // 建议只采用cout或者file单通道输出，并且通常情况下会自动进行切换不需要设置
    // 但是这里也可以设置两个通道都输出或者都关闭
    MLogger &set_flags(Out out_type) {
        switch (out_type) {
        case Out::N:
            m_use_cout_flag = false;
            m_use_file_flag = false;
            break;
        case Out::C:
            m_use_cout_flag = true;
            m_use_file_flag = false;
            break;
        case Out::F:
            m_use_cout_flag = false;
            m_use_file_flag = true;
            break;
        case Out::CF:
            m_use_cout_flag = true;
            m_use_file_flag = true;
            break;
        }

        return (*this);
    }

    // 初始时的注册
    MLogger &init_register(const std::string &name, bool output_flag,
                           Out out_type) {
        m_name = name;
        m_signature = "{" + m_name + "}";
        m_output_flag = output_flag;

        return set_flags(out_type);
    }

    MLogger &clean_file_and_ofstream(bool erase_flag) {
        if (m_logfile_ofstream) {
            // 关闭文件
            if (m_logfile_ofstream->is_open()) {
                // 正常状态就向这个文件流写入结束语
                set_flags(Out::F).notice_close_file().set_flags(Out::C);

                m_logfile_ofstream->flush();
                m_logfile_ofstream->close();
            }
            if (erase_flag) {
                MLogFileManager::erase_unique_ofstream(
                    m_file_name);  // 清理ofstream，在析构时不会调用，否则因为析构顺序可能有异常
            }
        }

        // 清理内部数据
        m_file_name = std::string();
        m_logfile_ofstream = nullptr;
        std::cout.flush();

        return (*this);
    }

    // 如果被锁定就报错退出，否则返回自身
    MLogger &if_unlock() {
        if (m_lock) notice_locked_and_exit();
        return (*this);
    }

    std::string m_name;           // 自己的名字
    std::string m_signature;      // 签名，比名字多了个{}
    bool m_output_flag{true};     // 是否直接关闭所有输出
    bool m_use_cout_flag{true};   // 是否使用cout
    bool m_use_file_flag{false};  // 是否使用文件流
    std::shared_ptr<std::ofstream>
        m_logfile_ofstream;  // 指针可以空但是引用必须初始化并且无法改变
    std::string
        m_file_name;  // 日志文件名是不含前缀的，并且需要通过文件名合法性检查
    bool m_lock{false};  // 加锁后只可以使用输出，不能用对外接口改变输出方式
    Format m_log_start_format{
        Format::LEVEL_SIGNATURE};  // 普通日志默认使用的开头格式

    //----------------------------------------------------------------------------//

    // 提示打开日志文件，日志等级off，日志戳为等级和签名和时间
    MLogger &notice_open_file() {
        return log_start(Level::off, Format::LEVEL_SIGNATURE_TIME)
               << " MLOG START\n";
    }

    // 提示关闭日志文件，日志等级off，日志戳为等级和签名和时间
    MLogger &notice_close_file() {
        return log_start(Level::off, Format::LEVEL_SIGNATURE_TIME)
               << " MLOG END\n";
    }

    // 通知无效名称，然后报错退出
    void notice_invalid_name_and_exit(const std::string &new_logger_name) {
        log_start(Level::on, Format::LEVEL_SIGNATURE_TIME)
            << " Invalid logger name \"" << new_logger_name
            << "\" can not be created.\n";

        MLogTool::raise_error();
    }

    // 通知被锁定无法修改输出流状态，然后报错退出
    void notice_locked_and_exit() {
        log_start(Level::on, Format::LEVEL_SIGNATURE_TIME)
            << " The logger named \"" << m_name
            << "\" has been locked, unable to change.\n";

        MLogTool::raise_error();
    }

    // 通知打开文件失败，然后报错退出
    void notice_open_file_failed_and_exit(const std::string &file_name) {
        log_start(Level::on, Format::LEVEL_SIGNATURE_TIME)
            << " Can not open file \"" << file_name << "\".\n";

        MLogTool::raise_error();
    }

    // 通知无效路径前缀，然后报错退出
    void notice_invalid_path_prefix_and_exit(const std::string &path_prefix) {
        log_start(Level::on, Format::LEVEL_SIGNATURE_TIME)
            << " Invalid directory \"" << path_prefix << "\".\n";

        MLogTool::raise_error();
    }
};

#endif  // MLOGGER_H_
