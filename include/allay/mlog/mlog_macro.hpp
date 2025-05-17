#ifndef MLOG_MACRO_H_
#define MLOG_MACRO_H_

// use marco to set log level
// #define MLOG_LEVEL MLOG_LEVEL_DEBUG

#define MLOG_LEVEL_ON (MLogTool::Level::on)
#define MLOG_LEVEL_DEBUG (MLogTool::Level::debug)
#define MLOG_LEVEL_INFO (MLogTool::Level::info)
#define MLOG_LEVEL_WARN (MLogTool::Level::warn)
#define MLOG_LEVEL_ERROR (MLogTool::Level::error)
#define MLOG_LEVEL_OFF (MLogTool::Level::off)

#ifdef MLOG_LEVEL
// use macro to set level
#define MLOG_USE_MACRO_LEVEL
#endif

//----------------------------------------------------------------------------//

#define MLOG_STAMP                                                             \
    "[" + std::string(static_cast<const char *>(__FILE__)) + " "               \
        + std::string(static_cast<const char *>(__FUNCTION__)) + " "           \
        + std::to_string(__LINE__) + "]\n"

#define MLOG_DEBUG(...) (MLog::debug(__VA_ARGS__) << MLOG_STAMP)
#define MLOG_INFO(...) (MLog::info(__VA_ARGS__) << MLOG_STAMP)
#define MLOG_WARN(...) (MLog::warn(__VA_ARGS__) << MLOG_STAMP)
#define MLOG_ERROR(...) (MLog::error(__VA_ARGS__) << MLOG_STAMP)

#define MLOG_IF_FIRST_N(x)                                                     \
    static auto mlog_tmp_first_##x = MLogTool::FirstN((x));                    \
    if (mlog_tmp_first_##x.judge())

#define MLOG_IF_EVERY_N(x)                                                     \
    static auto mlog_tmp_every_##x = MLogTool::EveryN((x));                    \
    if (mlog_tmp_every_##x.judge())

#define MLOG_IF_MORETHAN_N(x)                                                  \
    static auto mlog_tmp_morethan_##x = MLogTool::MoreThanN((x));              \
    if (mlog_tmp_morethan_##x.judge())

// 加一个临时性跳过某个函数的功能，但是会在控制台发出提示
#define MLOG_SKIP(...)                                                         \
    do {                                                                       \
        MLog::error() << MLOG_STAMP << "Skip the function!" << std::endl;      \
        return __VA_ARGS__;                                                    \
    } while (false);

#endif  // MLOG_MACRO_H_
