#if defined(_MSC_VER)
#pragma warning(disable : 4996)

#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#endif

#ifndef MLOG_H_
#define MLOG_H_

#include "mlog_macro.hpp"  // IWYU pragma: keep

#include "mlogtool.hpp"

#include "mlogfilemanager.hpp"

#include "mlogger.hpp"

#include "mloggermanager.hpp"

/*
这里有好几个单例，注意static变量最后的析构
一个是单例的全局日志等级变量
一个是单例的MLogFileManager
一个是单例的MLoggerManager，所有的MLogger对象在它的map中存在
*/

// 提升常用的接口到MLog类
class MLog {
public:
    using Format = MLogTool::LogStartFormat;
    using Level = MLogTool::Level;

    MLog() = delete;
    MLog(const MLog &) = delete;
    MLog &operator=(const MLog &) = delete;
    ~MLog() = delete;

    static void set_level_debug() { MLogTool::set_level(Level::debug); }

    static void set_level_info() { MLogTool::set_level(Level::info); }

    static void set_level_warn() { MLogTool::set_level(Level::warn); }

    static void set_level_error() { MLogTool::set_level(Level::error); }

    static MLogger &create_logger(const std::string &logger_name) {
        return MLoggerManager::create_logger(logger_name);
    }

    static MLogger &get_logger(const std::string &logger_name) {
        return MLoggerManager::get_logger(logger_name);
    }

    static MLogger &get_logger_cout() {
        return MLoggerManager::get_logger_cout();
    }

    static void init(const std::string &path_prefix) {
        MLoggerManager::init(path_prefix);
    }

    static void init() { MLoggerManager::init(std::string{}); }

    static void show_detail() { MLoggerManager::show_detail(); }

    //----------------------------------------------------------------------------//

    static MLogger &out() { return MLoggerManager::get_logger_cout(); }

    static MLogger &debug() {
        return MLoggerManager::get_logger_when(Level::debug);
    }

    static MLogger &info() {
        return MLoggerManager::get_logger_when(Level::info);
    }

    static MLogger &warn() {
        return MLoggerManager::get_logger_when(Level::warn);
    }

    static MLogger &error() {
        return MLoggerManager::get_logger_when(Level::error);
    }

    static MLogger &out(const std::string &logger_name) {
        return MLoggerManager::get_logger(logger_name);
    }

    static MLogger &debug(const std::string &logger_name) {
        return MLoggerManager::get_logger_when(Level::debug, logger_name);
    }

    static MLogger &info(const std::string &logger_name) {
        return MLoggerManager::get_logger_when(Level::info, logger_name);
    }

    static MLogger &warn(const std::string &logger_name) {
        return MLoggerManager::get_logger_when(Level::warn, logger_name);
    }

    static MLogger &error(const std::string &logger_name) {
        return MLoggerManager::get_logger_when(Level::error, logger_name);
    }
};

// 加入一个别名，并且是小写的
using mlog = MLog;  // NOLINT(readability-identifier-naming)

#endif  // MLOG_H_
