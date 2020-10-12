#include <gtest/gtest.h>

#include "smgl/Ports.hpp"
#include "smgl/TestLib.hpp"

using namespace smgl;

TEST(InputPort, VariableTarget)
{
    int target = 0;
    InputPort<int> port(&target);
    port.post(1, true);
    EXPECT_EQ(target, 1);
}

TEST(InputPort, MemberFnTarget)
{
    using IntClass = test::TrivialClass<int>;
    IntClass f;
    InputPort<int> port(&f, &IntClass::target);
    port.post(1, true);
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
    OutputPort<int> port(&test::FreeFnSource);
    EXPECT_EQ(port.val(), 1);
}

TEST(OutputPort, MemberFnSource)
{
    using IntClass = test::TrivialClass<int>;
    IntClass f;
    OutputPort<int> port(&f, &IntClass::source);
    f.result = 1;
    EXPECT_EQ(port.val(), f.result);
}

TEST(OutputPort, WithArg)
{
    // Default argument
    auto echo = [](int a) { return a; };
    OutputPort<int, int> port(echo, 1);
    EXPECT_EQ(port.val(), 1);

    // Change argument
    port.setArgs(2);
    EXPECT_EQ(port.val(), 2);
}

TEST(OutputPort, WithMultipleArgs)
{
    // Default arguments
    auto add = [](int a, int b) { return a + b; };
    OutputPort<int, int, int> port(add, 0, 1);
    EXPECT_EQ(port.val(), 1);

    // Change arguments
    port.setArgs(1, 1);
    EXPECT_EQ(port.val(), 2);
}

TEST(OutputPort, MemberFnWithArg)
{
    // Setup object and function reference
    using IntClass = test::TrivialClass<int>;
    IntClass f;

    // Default argument
    OutputPort<int, int> port(&f, &IntClass::return_value_target, 1);
    EXPECT_EQ(port.val(), 1);

    // Change argument
    port.setArgs(2);
    EXPECT_EQ(port.val(), 2);
}

TEST(Ports, IOConnectionBasic)
{
    int expected = test::FreeFnSource();
    OutputPort<int> source(&test::FreeFnSource);

    int result = 0;
    InputPort<int> target(&result);

    connect(source, target);
    source.update();
    EXPECT_NE(result, expected);
    target.update();
    EXPECT_EQ(result, expected);
}

TEST(Ports, IOConnectionOperators)
{
    int expected = test::FreeFnSource();
    OutputPort<int> source(&test::FreeFnSource);

    int result = 0;
    InputPort<int> target(&result);

    source >> target;
    source.update();
    EXPECT_NE(result, expected);
    target.update();
    EXPECT_EQ(result, expected);
}

TEST(Ports, IOConnectionTargetWithReturnValue)
{
    OutputPort<int> source(1);

    test::TrivialClass<int> f;
    InputPort<int> target([&f](int v) -> int { return f.return_value_target(v); });

    connect(source, target);
    source.update();
    EXPECT_NE(f.result, source.val());
    target.update();
    EXPECT_EQ(f.result, source.val());
}

TEST(Ports, IODisconnectBasic)
{
    // Setup basic connection
    int input{2};
    int result{0};
    OutputPort<int> source(&input);
    InputPort<int> target(&result);
    connect(source, target);

    // Source should update target
    source.update();
    target.update();
    EXPECT_EQ(result, input);

    // Disconnect ports
    disconnect(source, target);

    // Source should no longer update target
    input = 0;
    source.update();
    target.update();
    EXPECT_NE(result, input);
}

TEST(Ports, BadConnection)
{
    float result{0.0};
    OutputPort<int> source(1);
    InputPort<float> target(&result);
    EXPECT_THROW(connect(source, target), smgl::bad_connection);

    // Source should not update target
    source.update();
    target.update();
    EXPECT_NE(result, 1);
}

TEST(Ports, AutoDisconnect)
{
    OutputPort<int> outGood{1};
    EXPECT_EQ(outGood.numConnections(), 0);

    int valid{0};
    InputPort<int> inGood{&valid};
    EXPECT_EQ(inGood.numConnections(), 0);

    {
        // Connect outer scope output to inner scope input
        int invalid{0};
        InputPort<int> inBad(&invalid);
        connect(outGood, inBad);
        EXPECT_EQ(outGood.numConnections(), 1);
        EXPECT_EQ(inBad.numConnections(), 1);

        // Connect outer scope input to inner scope output
        OutputPort<int> outBad(-1);
        connect(outBad, inGood);
        EXPECT_EQ(outBad.numConnections(), 1);
        EXPECT_EQ(inGood.numConnections(), 1);
    }
    // ~inBad(), ~outBad()

    // Connections should have been severed
    EXPECT_EQ(outGood.numConnections(), 0);
    EXPECT_EQ(inGood.numConnections(), 0);
}