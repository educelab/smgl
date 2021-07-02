#include <gtest/gtest.h>

#include "smgl/Graph.hpp"
#include "smgl/Graphviz.hpp"
#include "smgl/TestLib.hpp"
#include "smgl/filesystem.hpp"

using namespace smgl;
namespace fs = smgl::filesystem;

static inline auto CreateDualChainGraph()
{
    using IntNode = test::ClassWrapperNode<int>;
    using SumNode = test::AdditionNode<int>;

    struct DualChainGraph {
        Graph graph;
        std::vector<std::shared_ptr<IntNode>> lhs;
        std::vector<std::shared_ptr<IntNode>> rhs;
        std::shared_ptr<SumNode> last;
        std::shared_ptr<IntNode> leaf;
    };

    // Setup graph
    DualChainGraph result;
    for (int i = 0; i < 3; i++) {
        auto n = result.graph.insertNode<IntNode>();
        if (not result.lhs.empty()) {
            n->set = result.lhs.back()->get;
        }
        result.lhs.push_back(n);
    }

    for (int i = 0; i < 2; i++) {
        auto n = result.graph.insertNode<IntNode>();
        if (not result.rhs.empty()) {
            n->set = result.rhs.back()->get;
        }
        result.rhs.push_back(n);
    }

    result.last = result.graph.insertNode<SumNode>();
    result.last->lhs = result.lhs.back()->get;
    result.last->rhs = result.rhs.back()->get;

    result.leaf = result.graph.insertNode<IntNode>();
    result.leaf->set = result.lhs.front()->get;

    return result;
}

TEST(Graphviz, BasicGraph)
{
    // Setup nodes
    using SourceNode = test::ClassWrapperNode<int>;
    using SumOpNode = test::AdditionNode<int>;
    using SubOpNode = test::SubtractionNode<int>;

    // Setup graph
    Graph graph;
    auto val1 = graph.insertNode<SourceNode>();
    auto val2 = graph.insertNode<SourceNode>();
    auto val4 = graph.insertNode<SourceNode>();
    auto sumOp = graph.insertNode<SumOpNode>();
    auto subOp = graph.insertNode<SubOpNode>();

    // Init value nodes
    val1->set(1);
    val2->set(2);
    val4->set(4);

    // 1 + 2
    val1->get >> sumOp->lhs;
    val2->get >> sumOp->rhs;

    // 4 - (1 + 2)
    val4->get >> subOp->lhs;
    sumOp->result >> subOp->rhs;

    // Update all nodes
    graph.update();

    // Register nodes
    RegisterNode<SourceNode>();
    RegisterNode<SumOpNode>();
    RegisterNode<SubOpNode>();

    WriteDotFile("TestGraphviz_BasicGraph.gv", graph);

    // Cleanup
    DeregisterNode<SourceNode>();
    DeregisterNode<SumOpNode>();
    DeregisterNode<SubOpNode>();
}

TEST(Graphviz, ComplexGraph)
{
    // Setup nodes + graph
    Graph graph;
    using SourceNode = test::PassThroughNode<int>;
    auto a = graph.insertNode<SourceNode>(3);
    auto b = graph.insertNode<SourceNode>(5);
    auto c = graph.insertNode<SourceNode>(6);
    auto d = graph.insertNode<SourceNode>(4);
    auto e = graph.insertNode<SourceNode>(6);
    auto final = graph.insertNode<SourceNode>(0);

    using SumOp = test::AdditionNode<int>;
    using SubOp = test::SubtractionNode<int>;
    auto a0 = graph.insertNode<SumOp>();
    auto a1 = graph.insertNode<SumOp>();
    auto a2 = graph.insertNode<SumOp>();
    auto a3 = graph.insertNode<SumOp>();
    auto s0 = graph.insertNode<SubOp>();
    auto s1 = graph.insertNode<SubOp>();

    // Connections
    // a0 srcs
    connect(b->get, a0->lhs);
    connect(c->get, a0->rhs);
    // a1 srcs
    connect(a0->result, a1->lhs);
    connect(d->get, a1->rhs);
    // a2 srcs
    connect(s0->result, a2->lhs);
    connect(s1->result, a2->rhs);
    // a3 srcs
    connect(a2->result, a3->lhs);
    connect(a1->result, a3->rhs);
    // s0 srcs
    connect(a->get, s0->lhs);
    connect(a0->result, s0->rhs);
    // s1 srcs
    connect(a1->result, s1->lhs);
    connect(e->get, s1->rhs);
    // final source
    connect(a3->result, final->set);

    graph.update();

    // Update graph and check final value
    RegisterNode<SourceNode>();
    RegisterNode<SumOp>();
    RegisterNode<SubOp>();

    WriteDotFile("TestGraphviz_ComplexGraph.gv", graph);

    // Cleanup
    DeregisterNode<SourceNode>();
    DeregisterNode<SumOp>();
    DeregisterNode<SubOp>();
}

TEST(Graphviz, GraphStyle)
{
    // Setup nodes
    using IntNode = test::ClassWrapperNode<int>;
    using FloatNode = test::ClassWrapperNode<float>;

    // Register nodes
    RegisterNode<IntNode>("IntNode");
    RegisterNode<FloatNode>("FloatNode");

    // Setup graph
    Graph graph;
    auto a = graph.insertNode<IntNode>();
    auto b = graph.insertNode<IntNode>();
    auto c = graph.insertNode<FloatNode>();

    // Global style
    GraphStyle style;
    style.defaultStyle().base.bgcolor = "red";

    // Class-specific style
    style.classStyle<IntNode>().base.bgcolor = "white";

    // Instance-specific style
    style.instanceStyle(a).base.bgcolor = "blue";
    style.instanceStyle(a).font.color = "white";

    // Write the graph
    WriteDotFile("TestGraphviz_GraphStyle.gv", graph, style);

    // Cleanup
    DeregisterNode<IntNode>();
}

TEST(Graphviz, NodeRankAuto)
{
    // Register nodes
    using IntNode = test::ClassWrapperNode<int>;
    using SumNode = test::AdditionNode<int>;
    RegisterNode<IntNode>("IntNode");
    RegisterNode<SumNode>("SumNode");

    // Create graph
    auto dcg = CreateDualChainGraph();

    // Write the DCG graph with auto ranking (for comparison)
    WriteDotFile("TestGraphviz_NodeRankAuto.gv", dcg.graph);

    // Cleanup
    DeregisterNode<IntNode>();
    DeregisterNode<SumNode>();
}

TEST(Graphviz, NodeRankMin)
{
    // Register nodes
    using IntNode = test::ClassWrapperNode<int>;
    using SumNode = test::AdditionNode<int>;
    RegisterNode<IntNode>("IntNode");
    RegisterNode<SumNode>("SumNode");

    // Create graph
    auto dcg = CreateDualChainGraph();

    // rhs root node rank=min
    GraphStyle style;
    style.setRankMin(dcg.rhs.front());

    // Write the graph
    WriteDotFile("TestGraphviz_NodeRankMin.gv", dcg.graph, style);

    // Cleanup
    DeregisterNode<IntNode>();
    DeregisterNode<SumNode>();
}

TEST(Graphviz, NodeRankSource)
{
    // Register nodes
    using IntNode = test::ClassWrapperNode<int>;
    using SumNode = test::AdditionNode<int>;
    RegisterNode<IntNode>("IntNode");
    RegisterNode<SumNode>("SumNode");

    // Create graph
    auto dcg = CreateDualChainGraph();

    // rhs root node rank=source (exclusive min)
    GraphStyle style;
    style.setRankSource(dcg.rhs.front());

    // Write the graph
    WriteDotFile("TestGraphviz_NodeRankSource.gv", dcg.graph, style);

    // Cleanup
    DeregisterNode<IntNode>();
    DeregisterNode<SumNode>();
}

TEST(Graphviz, NodeRankMax)
{
    // Register nodes
    using IntNode = test::ClassWrapperNode<int>;
    using SumNode = test::AdditionNode<int>;
    RegisterNode<IntNode>("IntNode");
    RegisterNode<SumNode>("SumNode");

    // Create graph
    auto dcg = CreateDualChainGraph();

    // leaf node rank=max
    GraphStyle style;
    style.setRankMax(dcg.leaf);

    // Write the graph
    WriteDotFile("TestGraphviz_NodeRankMax.gv", dcg.graph, style);

    // Cleanup
    DeregisterNode<IntNode>();
    DeregisterNode<SumNode>();
}

TEST(Graphviz, NodeRankSink)
{
    // Register nodes
    using IntNode = test::ClassWrapperNode<int>;
    using SumNode = test::AdditionNode<int>;
    RegisterNode<IntNode>("IntNode");
    RegisterNode<SumNode>("SumNode");

    // Create graph
    auto dcg = CreateDualChainGraph();

    // leaf node rank=sink (exclusive max)
    GraphStyle style;
    style.setRankSink(dcg.leaf);

    // Write the graph
    WriteDotFile("TestGraphviz_NodeRankSink.gv", dcg.graph, style);

    // Cleanup
    DeregisterNode<IntNode>();
    DeregisterNode<SumNode>();
}

TEST(Graphviz, NodeRankSame)
{
    // Register nodes
    using IntNode = test::ClassWrapperNode<int>;
    using SumNode = test::AdditionNode<int>;
    RegisterNode<IntNode>("IntNode");
    RegisterNode<SumNode>("SumNode");

    // Create graph
    auto dcg = CreateDualChainGraph();

    // lhs & rhs roots rank=same
    GraphStyle style;
    style.setRankSame(dcg.lhs.front(), dcg.rhs.front());

    // lhs & rhs root + 1 rank=same
    // Tests the append syntax
    auto rankIdx = style.setRankSame(dcg.lhs[1]);
    style.appendRankSame(rankIdx, dcg.rhs[1]);

    // Write the graph
    WriteDotFile("TestGraphviz_NodeRankSame.gv", dcg.graph, style);

    // Cleanup
    DeregisterNode<IntNode>();
    DeregisterNode<SumNode>();
}