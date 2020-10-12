#include <gtest/gtest.h>

#include "smgl/Singleton.hpp"

using namespace smgl;
using namespace smgl::policy;

TEST(Singleton, Defaults)
{
    using Singleton = detail::SingletonHolder<int>;
    {
        // Construct singleton in a scope
        EXPECT_EQ(Singleton::Instance(), 0);

        // Update its value
        Singleton::Instance() = 1;
    }

    // Check that the value holds after scope close
    EXPECT_EQ(Singleton::Instance(), 1);
}

TEST(Singleton, DynamicPhoenix)
{
    using Singleton =
        detail::SingletonHolder<int, CreateUsingNew, PhoenixLifetime>;
    {
        // Construct singleton in a scope
        EXPECT_EQ(Singleton::Instance(), 0);

        // Update its value
        Singleton::Instance() = 1;
    }

    // Check that the value holds after scope close
    EXPECT_EQ(Singleton::Instance(), 1);
}