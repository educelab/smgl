#include <iostream>

#include <gtest/gtest.h>

#include "smeagol/Ports.hpp"

using namespace smeagol;

struct Foo {
    void target(int i) { result = i; }
    int return_value_target(int i)
    {
        result = i;
        return result;
    }
    int source() const { return result; }
    int result{0};
};

int FreeFnSource() { return 1; }

TEST(InputPort, VariableTarget)
{
    int target = 0;
    InputPort<int> port(&target);
    port.notify(1);
    EXPECT_EQ(target, 1);
}

TEST(InputPort, MemberFnTarget)
{
    Foo f;
    InputPort<int> port([&f](int v) { f.target(v); });
    port.notify(1);
    EXPECT_EQ(f.result, 1);
}

TEST(OutputPort, VariableConstantSource)
{
    int source = 0;
    OutputPort<int> port(source);
    source = 1;
    EXPECT_EQ(port.val(), 0);
}

TEST(OutputPort, VariableReferenceSource)
{
    int source = 0;
    OutputPort<int> port(&source);
    source = 1;
    EXPECT_EQ(port.val(), 1);
}

TEST(OutputPort, FreeFnSource)
{
    OutputPort<int> port(&FreeFnSource);
    EXPECT_EQ(port.val(), 1);
}

TEST(OutputPort, MemberFnSource)
{
    Foo f;
    OutputPort<int> port([&f]() { return f.source(); });
    f.result = 1;
    EXPECT_EQ(port.val(), f.result);
}

TEST(Ports, IOConnectionBasic)
{
    int expected = FreeFnSource();
    OutputPort<int> source(&FreeFnSource);

    int result = 0;
    InputPort<int> target(&result);

    source.connect(target);
    EXPECT_NE(result, expected);
    target.update();
    EXPECT_EQ(result, expected);
}

TEST(Ports, IOConnectionTargetWithReturnValue)
{
    OutputPort<int> source(1);

    Foo f;
    InputPort<int> target([&f](int v) -> int { return f.return_value_target(v); });

    source.connect(target);
    EXPECT_NE(f.result, source.val());
    target.update();
    EXPECT_EQ(f.result, source.val());
}