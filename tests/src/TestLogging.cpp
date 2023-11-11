#include <iomanip>
#include <sstream>

#include <gtest/gtest.h>

#include "smgl/Logging.hpp"
#include "smgl/LoggingPrivate.hpp"

TEST(Logging, None)
{
    // Set up logging to std::stringstream
    std::ostringstream logOut;
    smgl::SetLogStream(&logOut);

    // Nothing should print
    smgl::LogError("Error message");
    smgl::LogWarning("Warning message");
    smgl::LogInfo("Info message");
    smgl::LogDebug("Debug message");
    EXPECT_EQ(logOut.str(), "");
}

TEST(Logging, Error)
{
    // Set up logging to std::stringstream
    std::ostringstream logOut;
    smgl::SetLogStream(&logOut);

    // Log messages
    smgl::SetLogLevel(smgl::LogLevel::Error);
    smgl::LogError();
    smgl::LogWarning();
    smgl::LogInfo();
    smgl::LogDebug();

    EXPECT_EQ(logOut.str(), "[smgl] [error]\n");
}

TEST(Logging, Warning)
{
    // Set up logging to std::stringstream
    std::ostringstream logOut;
    smgl::SetLogStream(&logOut);

    // Log messages
    smgl::SetLogLevel(smgl::LogLevel::Warning);
    smgl::LogError();
    smgl::LogWarning();
    smgl::LogInfo();
    smgl::LogDebug();

    EXPECT_EQ(
        logOut.str(),
        "[smgl] [error]\n"
        "[smgl] [warning]\n");
}

TEST(Logging, Info)
{
    // Set up logging to std::stringstream
    std::ostringstream logOut;
    smgl::SetLogStream(&logOut);

    // Log messages
    smgl::SetLogLevel(smgl::LogLevel::Info);
    smgl::LogError();
    smgl::LogWarning();
    smgl::LogInfo();
    smgl::LogDebug();

    EXPECT_EQ(
        logOut.str(),
        "[smgl] [error]\n"
        "[smgl] [warning]\n"
        "[smgl] [info]\n");
}

TEST(Logging, Debug)
{
    // Set up logging to std::stringstream
    std::ostringstream logOut;
    smgl::SetLogStream(&logOut);

    // Log messages
    smgl::SetLogLevel(smgl::LogLevel::Debug);
    smgl::LogError();
    smgl::LogWarning();
    smgl::LogInfo();
    smgl::LogDebug();

    EXPECT_EQ(
        logOut.str(),
        "[smgl] [error]\n"
        "[smgl] [warning]\n"
        "[smgl] [info]\n"
        "[smgl] [debug]\n");
}

TEST(Logging, VariadicArgs)
{
    // Set up logging to std::stringstream
    std::ostringstream logOut;
    logOut << std::boolalpha;
    smgl::SetLogStream(&logOut);

    // Log messages
    smgl::SetLogLevel(smgl::LogLevel::Info);
    smgl::LogInfo(-3.1, -2.1F, -1, 0U, 1L, 2UL, 3LL, true, false);
    EXPECT_EQ(logOut.str(), "[smgl] [info] -3.1 -2.1 -1 0 1 2 3 true false\n");
}
