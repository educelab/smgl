#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <memory>

#include "smgl/TestLib.hpp"

using namespace smgl;
using namespace smgl::detail;

TEST(Node, BinaryOp)
{
    test::AdditionNode<int> op;
    EXPECT_EQ(op.result(), 0);
    op.lhs(1);
    op.rhs(1);
    op.update();
    EXPECT_EQ(op.result(), 2);
}

TEST(Node, TrivialWrapperNode)
{
    test::ClassWrapperNode<int> node;
    EXPECT_EQ(node.get(), 0);

    node.set(1);
    node.update();
    EXPECT_EQ(node.get(), 1);
}

TEST(Node, MultiOpPipeline)
{
    // Setup nodes
    test::ClassWrapperNode<int> val1, val2, val4;
    test::AdditionNode<int> sumOp;
    test::SubtractionNode<int> subOp;

    // Make list of nodes
    std::vector<Node*> nodes{&val1, &val2, &val4, &sumOp, &subOp};

    // Init value nodes
    val1.set(1);
    val2.set(2);
    val4.set(4);

    // 1 + 2
    connect(val1.get, sumOp.lhs);
    connect(val2.get, sumOp.rhs);

    // 4 - (1 + 2)
    connect(val4.get, subOp.lhs);
    connect(sumOp.result, subOp.rhs);

    // Test init
    EXPECT_EQ(val1.get(), 0);
    EXPECT_EQ(val2.get(), 0);
    EXPECT_EQ(val4.get(), 0);
    EXPECT_EQ(sumOp.result(), 0);
    EXPECT_EQ(subOp.result(), 0);

    // Update all nodes.
    // Don't need to check state: Ordered in op order
    for (const auto& n : nodes) {
        n->update();
    }

    // Test results
    EXPECT_EQ(val1.get(), 1);
    EXPECT_EQ(val2.get(), 2);
    EXPECT_EQ(val4.get(), 4);
    EXPECT_EQ(sumOp.result(), 3);
    EXPECT_EQ(subOp.result(), 1);

    // Change source nodes
    val1.set(-1);
    val2.set(-2);
    val4.set(-4);

    // Update all nodes
    for (const auto& n : nodes) {
        n->update();
    }

    // Test results
    EXPECT_EQ(val1.get(), -1);
    EXPECT_EQ(val2.get(), -2);
    EXPECT_EQ(val4.get(), -4);
    EXPECT_EQ(sumOp.result(), -3);
    EXPECT_EQ(subOp.result(), -1);
}

TEST(Node, DestructNode)
{
    // Persistent nodes and connections
    test::PassThroughNode<int> start{1}, lhs{0};
    test::AdditionNode<int> end;
    connect(start.get, lhs.set);
    connect(lhs.get, end.lhs);

    // Check lhs branch
    start.update();
    lhs.update();
    end.update();
    EXPECT_EQ(end.result(), 1);

    // Scoped branch
    {
        test::PassThroughNode<int> rhs{0};
        connect(start.get, rhs.set);
        connect(rhs.get, end.rhs);

        rhs.set(1);
        rhs.update();
        end.update();
        EXPECT_EQ(end.result(), 2);
    }

    start.set(2);
    start.update();
    lhs.update();
    // This isn't much of a test since rhs isn't updated here.
    end.update();
    EXPECT_EQ(end.result(), 3);
}

TEST(Node, GetPortFunctions)
{
    using IntNode = test::PassThroughNode<int>;
    using SumOp = test::AdditionNode<int>;
    Node::Pointer lhs = std::make_shared<IntNode>(1);
    Node::Pointer rhs = std::make_shared<IntNode>(1);
    Node::Pointer op = std::make_shared<SumOp>();

    connect(lhs->getOutputPort("get"), op->getInputPort("lhs"));
    connect(rhs->getOutputPort("get"), op->getInputPort("rhs"));
    for (const auto& ptr : {lhs, rhs, op}) {
        ptr->update();
    }

    EXPECT_EQ(std::static_pointer_cast<SumOp>(op)->result(), 2);
}

TEST(Node, GetPortFunctionsOperatorConnect)
{
    using IntNode = test::PassThroughNode<int>;
    using SumOp = test::AdditionNode<int>;
    Node::Pointer lhs = std::make_shared<IntNode>(1);
    Node::Pointer rhs = std::make_shared<IntNode>(1);
    Node::Pointer op = std::make_shared<SumOp>();

    op->getInputPort("lhs") = lhs->getOutputPort("get");
    op->getInputPort("rhs") = rhs->getOutputPort("get");
    for (const auto& ptr : {lhs, rhs, op}) {
        ptr->update();
    }

    EXPECT_EQ(std::static_pointer_cast<SumOp>(op)->result(), 2);
}

TEST(Node, TestDefaultRegistration)
{
    using SourceNode = test::PassThroughNode<int>;

    // Test registration
    using ::testing::UnorderedElementsAreArray;
    EXPECT_TRUE(RegisterNode<SourceNode>());
    EXPECT_EQ(NodeFactoryType::Instance().GetRegisteredIdentifiers().size(), 1);
    EXPECT_THAT(
        NodeFactoryType::Instance().GetRegisteredIdentifiers(),
        UnorderedElementsAreArray({"smgl::test::PassThroughNode<int>"}));

    // Test unregistration
    EXPECT_TRUE(DeregisterNode<SourceNode>());
    EXPECT_EQ(NodeFactoryType::Instance().GetRegisteredIdentifiers().size(), 0);
}

TEST(Node, TestNamedRegistration)
{
    using SourceNode = test::PassThroughNode<int>;

    // Test registration
    using ::testing::UnorderedElementsAreArray;
    EXPECT_TRUE(RegisterNode<SourceNode>("SourceNode"));
    EXPECT_EQ(NodeFactoryType::Instance().GetRegisteredIdentifiers().size(), 1);
    EXPECT_THAT(
        NodeFactoryType::Instance().GetRegisteredIdentifiers(),
        UnorderedElementsAreArray({"SourceNode"}));

    // Test unregistration
    EXPECT_TRUE(DeregisterNode("SourceNode"));
    EXPECT_EQ(NodeFactoryType::Instance().GetRegisteredIdentifiers().size(), 0);
}

TEST(Node, TestCreateUnregistered)
{
    std::string id = "Null";
    EXPECT_THROW(CreateNode(id), smgl::unknown_identifier);
}

TEST(Node, TestUnregisteredTypeName)
{
    using SourceNode = test::PassThroughNode<int>;
    Node::Pointer src = std::make_shared<SourceNode>();
    EXPECT_THROW(NodeName(src), smgl::unknown_identifier);
}

TEST(Node, TestDefaultRegistrationMultiple)
{
    // Define nodes
    using SourceNode = test::PassThroughNode<int>;
    using SumOp = test::AdditionNode<int>;
    using SubOp = test::SubtractionNode<int>;
    using WrapperOp = test::ClassWrapperNode<int>;

    // Register nodes
    // Note: Tests in this file are all run in the same context, so these
    // registrations affect other tests and vice versa.
    auto res = RegisterNode<SourceNode, SumOp, SubOp, WrapperOp>();
    EXPECT_TRUE(res);

    // Check that the registrations are listed
    std::vector<std::string> keys{
        "smgl::test::PassThroughNode<int>", "smgl::test::AdditionNode<int>",
        "smgl::test::SubtractionNode<int>",
        "smgl::test::ClassWrapperNode<int>"};
    using ::testing::UnorderedElementsAreArray;
    EXPECT_EQ(NodeFactoryType::Instance().GetRegisteredIdentifiers().size(), 4);
    EXPECT_THAT(
        NodeFactoryType::Instance().GetRegisteredIdentifiers(),
        UnorderedElementsAreArray(keys));

    // Check construction
    for (const auto& k : keys) {
        std::shared_ptr<Node> ptr;
        EXPECT_NO_THROW(ptr = CreateNode(k));
        EXPECT_NE(ptr, nullptr);
    }

    // Deregister nodes
    res = DeregisterNode<SourceNode, SumOp, SubOp, WrapperOp>();
    EXPECT_EQ(NodeFactoryType::Instance().GetRegisteredIdentifiers().size(), 0);
}

TEST(Node, TestNamedRegistrationMultiple)
{
    // Define nodes
    using SourceNode = test::PassThroughNode<int>;
    using SumOp = test::AdditionNode<int>;
    using SubOp = test::SubtractionNode<int>;
    using WrapperOp = test::ClassWrapperNode<int>;

    // Register nodes
    // Note: Tests in this file are all run in the same context, so these
    // registrations affect other tests and vice versa.
    EXPECT_TRUE(RegisterNode<SourceNode>("SourceNode"));
    EXPECT_TRUE(RegisterNode<SumOp>("SumOp"));
    EXPECT_TRUE(RegisterNode<SubOp>("SubOp"));
    EXPECT_TRUE(RegisterNode<WrapperOp>("WrapperOp"));

    // Check that the registrations are listed
    std::vector<std::string> keys{"SourceNode", "SumOp", "SubOp", "WrapperOp"};
    using ::testing::UnorderedElementsAreArray;
    EXPECT_EQ(NodeFactoryType::Instance().GetRegisteredIdentifiers().size(), 4);
    EXPECT_THAT(
        NodeFactoryType::Instance().GetRegisteredIdentifiers(),
        UnorderedElementsAreArray(keys));

    // Check construction
    for (const auto& k : keys) {
        std::shared_ptr<Node> ptr;
        EXPECT_NO_THROW(ptr = CreateNode(k));
        EXPECT_NE(ptr, nullptr);
    }

    // Deregister nodes
    EXPECT_TRUE(DeregisterNode<SourceNode>());
    EXPECT_TRUE(DeregisterNode<SumOp>());
    EXPECT_TRUE(DeregisterNode<SubOp>());
    EXPECT_TRUE(DeregisterNode<WrapperOp>());
    EXPECT_EQ(NodeFactoryType::Instance().GetRegisteredIdentifiers().size(), 0);
}

TEST(Node, FactoryGetPortFunctions)
{
    // Register nodes
    using IntNode = test::PassThroughNode<int>;
    using SumOp = test::AdditionNode<int>;
    RegisterNode<IntNode>("IntNode");
    RegisterNode<SumOp>("SumOp");

    // Create nodes with the factory
    auto lhs = CreateNode("IntNode");
    auto rhs = CreateNode("IntNode");
    auto op = CreateNode("SumOp");

    // Initialize port values
    std::static_pointer_cast<IntNode>(lhs)->set(1);
    std::static_pointer_cast<IntNode>(rhs)->set(1);

    // Connect and update ports
    connect(lhs->getOutputPort("get"), op->getInputPort("lhs"));
    connect(rhs->getOutputPort("get"), op->getInputPort("rhs"));
    for (const auto& ptr : {lhs, rhs, op}) {
        ptr->update();
    }

    // Check that op worked
    EXPECT_EQ(std::static_pointer_cast<SumOp>(op)->result(), 2);

    // Cleanup
    DeregisterNode("IntNode");
    DeregisterNode("SumOp");
}

TEST(Node, SerializeNoCache)
{
    // Construct a test node
    using SumOp = test::AdditionNode<int>;
    RegisterNode<SumOp>();
    SumOp node;
    node.lhs(1);
    node.rhs(1);
    node.update();

    // Serialize
    auto meta = node.serialize(false, "");
    auto data = meta["data"];
    EXPECT_EQ(data["lhs"].get<int>(), 1);
    EXPECT_EQ(data["rhs"].get<int>(), 1);
    EXPECT_EQ(data["result"].get<int>(), 2);

    // Deserialize to new node
    SumOp nodeClone;
    nodeClone.deserialize(meta, "");
    EXPECT_EQ(nodeClone.result.val(), 2);
}