#include "smgl/Logging.hpp"

#include <algorithm>

#include "smgl/LoggingPrivate.hpp"

using namespace smgl;

namespace
{
LogLevel level_from_str(std::string s)
{
    // convert to lower case
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) {
        return std::tolower(c);
    });

    // check the enum
    if (s == "all" or s == "a")
        return LogLevel::All;
    else if (s == "debug" or s == "d")
        return LogLevel::Debug;
    else if (s == "info" or s == "i")
        return LogLevel::Info;
    else if (s == "warning" or s == "w")
        return LogLevel::Warning;
    else if (s == "error" or s == "e")
        return LogLevel::Error;
    else
        return LogLevel::None;
}
}  // namespace

void smgl::SetLogLevel(LogLevel level)
{
    detail::LogConf::Instance().level(level);
}

void smgl::SetLogLevel(const std::string& level)
{
    SetLogLevel(::level_from_str(level));
}

LogLevel smgl::GetLogLevel() { return detail::LogConf::Instance().level(); }

void smgl::SetLogStream(std::ostream* os)
{
    detail::LogConf::Instance().out(os);
}