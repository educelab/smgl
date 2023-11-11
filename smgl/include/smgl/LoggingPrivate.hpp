#include <iomanip>
#include <iostream>

#include "smgl/Logging.hpp"
#include "smgl/Singleton.hpp"
#include "smgl/Utilities.hpp"

namespace smgl
{

namespace detail
{

class LoggingConfig
{
private:
    LogLevel level_{LogLevel::None};
    std::ostream* out_;

public:
    LoggingConfig() : out_{&std::cerr} { *out_ << std::boolalpha; }

    [[nodiscard]] LogLevel level() const { return level_; }

    void level(LogLevel level) { level_ = level; }

    bool check(LogLevel msgLevel) { return msgLevel >= level_; }

    std::ostream& out() { return *out_; }

    void out(std::ostream* out) { out_ = out; }
};

using LogConf = SingletonHolder<LoggingConfig>;

template <typename T>
void LogStart(const T& arg)
{
    LogConf::Instance().out() << arg;
}

template <typename T>
void LogArg(const T& arg)
{
    LogConf::Instance().out() << ' ' << arg;
}

template <typename... Args>
void LogMessage(Args... args)
{
#if __cplusplus >= 201703L
    (detail::LogArg(args), ...);
#elif __cplusplus > 201103L
    detail::ExpandType{0, (detail::LogArg(std::forward<Args>(args)), 0)...};
#endif
    LogConf::Instance().out() << std::endl;
}
}  // namespace detail

template <typename... Args>
void LogError(Args... args)
{
    if (not detail::LogConf::Instance().check(LogLevel::Error)) {
        return;
    }

    detail::LogStart("[smgl] [error]");
    detail::LogMessage(std::forward<Args>(args)...);
}

template <typename... Args>
void LogWarning(Args... args)
{
    if (not detail::LogConf::Instance().check(LogLevel::Warning)) {
        return;
    }

    detail::LogStart("[smgl] [warning]");
    detail::LogMessage(std::forward<Args>(args)...);
}

template <typename... Args>
void LogInfo(Args... args)
{
    if (not detail::LogConf::Instance().check(LogLevel::Info)) {
        return;
    }

    detail::LogStart("[smgl] [info]");
    detail::LogMessage(std::forward<Args>(args)...);
}

template <typename... Args>
void LogDebug(Args... args)
{
    if (not detail::LogConf::Instance().check(LogLevel::Debug)) {
        return;
    }

    detail::LogStart("[smgl] [debug]");
    detail::LogMessage(std::forward<Args>(args)...);
}

}  // namespace smgl
