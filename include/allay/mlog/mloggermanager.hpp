#ifndef MLOGGERMANAGER_H_
#define MLOGGERMANAGER_H_

#include "mlogger.hpp"

#include <filesystem>

// 负责日志等级判定
class MLoggerManager {
public:
    using Format = MLogTool::LogStartFormat;
    using Level = MLogTool::Level;
    using OutType = MLogTool::OutType;

    MLoggerManager(const MLoggerManager &) = delete;
    MLoggerManager &operator=(const MLoggerManager &) = delete;

    // 创建MLogger对象,默认不向任何位置输出
    // 名称具有唯一性，因此如果重名返回原来的对象
    // 通过注册map保存
    // 对已经存在的直接报错
    static MLogger &create_logger(const std::string &logger_name) {
        // 如果logger已经存在则报错结束
        if (logger_map().find(logger_name) != logger_map().end()) {
            logger_map()["cout"].log_start(Level::on,
                                           Format::LEVEL_SIGNATURE_TIME)
                << " The logger named \"" << logger_name
                << "\" already exists. Can not create it again.)\n";

            MLogTool::raise_error();
        }

        // 检查名称的合法性
        // 如果空字符串或者名称不合法，报错
        if (logger_name.empty()
            || !MLogTool::check_filename_valid(logger_name)) {
            logger_map()["cout"].notice_invalid_name_and_exit(logger_name);
        }

        return logger_map()[logger_name].init_register(logger_name, true,
                                                       OutType::C);
    }

    // 如果没有在map找到，不会自动新建
    static MLogger &get_logger(const std::string &logger_name) {
        // 如果没有在map找到，不会自动新建，报错结束
        if (logger_map().find(logger_name) == logger_map().end()) {
            logger_map()["cout"].log_start(Level::on,
                                           Format::LEVEL_SIGNATURE_TIME)
                << " Can not find a logger named \"" << logger_name
                << "\". Please create it. (Maybe you forgot to use "
                   "MLog::init().)\n";

            MLogTool::raise_error();
        }

        return logger_map().find(logger_name)->second;
    }

    //----------------------------------------------------------------------------//

    // 标准MLogger对象cout，向cout输出
    static MLogger &get_logger_cout() { return logger_map()["cout"]; }

    // 标准MLogger对象__none__，关闭所有输出
    static MLogger &get_logger_none() { return logger_map()["__none__"]; }

    //----------------------------------------------------------------------------//

    // 如果满足条件返回MLogger单例cout，自动加标签
    // 否则返回MLogger单例__none__
    static MLogger &get_logger_when(Level level) {
        return (MLogTool::get_level_instance() <= level)
                   ? get_logger_cout().log_start(level)
                   : get_logger_none();
    }

    // 如果满足条件返回指定名称的logger对象，自动加标签
    // 否则返回MLogger单例__none__
    static MLogger &get_logger_when(Level level,
                                    const std::string &logger_name) {
        return (MLogTool::get_level_instance() <= level)
                   ? get_logger(logger_name).log_start(level)
                   : get_logger_none();
    }

    //----------------------------------------------------------------------------//

    // 初始化
    // 创建cout和__none__两个默认logger对象，并完成相关设置然后锁定
    // 接收并记录一下日志文件的路径前缀
    static void init(const std::string &path_prefix) {
        logger_map()["cout"]
            .init_register("cout", true, OutType::C)
            .set_format(Format::LEVEL_COLOR)
            .lock();

        logger_map()["__none__"]
            .init_register("__none__", false, OutType::C)
            .lock();

        // 设置日志根目录
        if (!path_prefix.empty()) {
            std::filesystem::create_directories(path_prefix);  // 先尝试创建路径
            if (std::filesystem::exists(path_prefix)) {  // 创建成功则没有问题
                MLogFileManager::set_path_prefix(path_prefix);
            }
            else {
                get_logger_cout().notice_invalid_path_prefix_and_exit(
                    path_prefix);
            }
        }
    }

    // 在控制台展示当前的日志根目录，当前打开的所有日志文件
    static void show_detail() {
        get_logger_cout().log_start(Level::off) << " MLOG DETAIL\n";
        // 日志根目录
        get_logger_cout().log_start(Level::info)
            << " log file dir: " << MLogFileManager::get_path_prefix() << '\n';

        // 遍历所有拥有文件流的logger
        auto iter = logger_map().begin();
        while (iter != logger_map().end()) {
            if (iter->second.m_logfile_ofstream) {
                get_logger_cout().log_start(Level::info)
                    << " log file: " << iter->second.m_file_name
                    << ", owner: " << iter->second.m_name << '\n';
            }
            ++iter;
        }

        get_logger_cout().log_start(Level::off) << '\n';
    }

    //----------------------------------------------------------------------------//

private:
    // 禁止从外部尝试构造，并且只允许static方法访问实例
    MLoggerManager() = default;
    ~MLoggerManager() = default;

    // 获取唯一实例的map
    static std::map<const std::string, MLogger> &logger_map() {
        static MLoggerManager the_logger_manager;
        return the_logger_manager.m_logger_map;
    }

    // 基于map存储logger，必须具名
    std::map<const std::string, MLogger> m_logger_map;
};

#endif  // MLOGGERMANAGER_H_
