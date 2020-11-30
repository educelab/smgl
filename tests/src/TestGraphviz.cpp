#include <gtest/gtest.h>

#include "smgl/Graph.hpp"
#include "smgl/Graphviz.hpp"
#include "smgl/TestLib.hpp"
#include "smgl/filesystem.hpp"

using namespace smgl;
namespace fs = smgl::filesystem;

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

    WriteDotFile("BasicGraph.gv", graph);

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

    WriteDotFile("ComplexGraph.gv", graph);

    // Cleanup
    DeregisterNode<SourceNode>();
    DeregisterNode<SumOp>();
    DeregisterNode<SubOp>();
}