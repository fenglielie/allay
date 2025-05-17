#ifndef MLOGFILEMANAGER_H_
#define MLOGFILEMANAGER_H_

#include "mlogtool.hpp"

#include <algorithm>
#include <map>
#include <memory>
#include <string>

// 最底层的文件层，一个文件名提供一个ofstream
// 必须被一个logger独占，通过map来管理，但是不负责文件打开关闭的细节
class MLogFileManager {
public:
    friend class MLogger;  // 所有的接口只可以被logger调用

    MLogFileManager &operator=(const MLogFileManager &) = delete;
    MLogFileManager(const MLogFileManager &) = delete;

    static void set_path_prefix(const std::string &path_prefix) {
        get_instance().m_path_prefix = path_prefix;
    }

    static std::string get_path_prefix() {
        return get_instance().m_path_prefix;
    }

private:
    static std::string to_low(const std::string &raw_file_name) {
        std::string file_name = raw_file_name;
        std::ranges::transform(file_name, file_name.begin(), ::tolower);
        return file_name;
    }

    // 如果file_name已经存在则不允许并返回nullptr
    // 否则通过智能指针返回一个新的ofstream
    // 注意内部map会把这个文件名改成小写来存储和查询，但是不影响真实文件名
    static std::shared_ptr<std::ofstream>
    get_unique_ofstream(const std::string &raw_file_name) {
        // 存储的map使用的是全小写
        std::string file_name = to_low(raw_file_name);

        // 如果已经存在，不会给新的用户
        if (get_instance().m_ofstream_map.find(file_name)
            != get_instance().m_ofstream_map.end())
            return nullptr;

        // 名称不合法也不分配
        if (file_name.empty() || !MLogTool::check_filename_valid(file_name))
            return nullptr;

        // 分配并存储新的日志文件流
        get_instance().m_ofstream_map[file_name] =
            std::make_shared<std::ofstream>();
        return get_instance().m_ofstream_map[file_name];
    }

    // 负责erase，但是不负责文件关闭
    static void erase_unique_ofstream(const std::string &raw_file_name) {
        // 存储的map使用的是全小写
        std::string file_name = to_low(raw_file_name);

        if (auto iter = get_instance().m_ofstream_map.find(file_name);
            iter != get_instance().m_ofstream_map.end())
            get_instance().m_ofstream_map.erase(iter);
    }

    static MLogFileManager &get_instance() {
        static MLogFileManager the_logfile_manager;
        return the_logfile_manager;
    }

    MLogFileManager() = default;
    ~MLogFileManager() = default;

    std::map<const std::string, std::shared_ptr<std::ofstream>> m_ofstream_map;
    std::string m_path_prefix;  // 路径前缀，注意路径全部采用/分隔符
};

#endif  // MLOGFILEMANAGER_H_
