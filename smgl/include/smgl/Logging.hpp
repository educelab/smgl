#pragma once

#include <ostream>
#include <string>

/**
 * @file
 *
 * @brief Library logging utilities
 *
 * The public interface for controlling smgl library logging.
 */

namespace smgl
{

/** @brief Log levels */
enum LogLevel {
    All = 0,       ///< All messages
    Debug = 10,    ///< Debug messages and above
    Info = 20,     ///< Info messages and above
    Warning = 30,  ///< Warning messages and above
    Error = 40,    ///< Error messages and above
    None = 100     ///< No messages
};

/** @brief Set the library log level */
void SetLogLevel(LogLevel level);

/**
 * @brief Set the library log level from a string
 * @overload SetLogLevel(LogLevel)
 */
void SetLogLevel(const std::string& level);

/** @brief Get the library log level */
LogLevel GetLogLevel();

/** @brief Set the output stream */
void SetLogStream(std::ostream* os);

}  // namespace smgl