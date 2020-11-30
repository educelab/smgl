#include <gtest/gtest.h>

#include <vector>

#include "smgl/Graph.hpp"
#include "smgl/Metadata.hpp"
#include "smgl/TestLib.hpp"
#include "smgl/filesystem.hpp"

using namespace smgl;
namespace fs = smgl::filesystem;

inline std::string LoadCachedValue(const fs::path& p, const std::string& uuid)
{
    auto m = LoadMetadata(p);
    fs::path cacheDir = m["cacheDir"].get<std::string>();
    if (cacheDir.is_relative()) {
        cacheDir = p.parent_path() / cacheDir;
    }
    auto valuePath =
        m["nodes"][uuid]["data"]["cacheFile"].get<std::string>();
    std::ifstream f((cacheDir / uuid / valuePath).string());
    std::string res;
    std::getline(f, res);
    return res;
}

TEST(Graph, BasicGraph)
{
    // Setup nodes
    using SourceNode = test::ClassWrapperNode<int>;
    using SumOpNode = test::AdditionNode<int>;
    using SubOpNode = test::SubtractionNode<int>;

    auto val1 = std::make_shared<SourceNode>();
    auto val2 = std::make_shared<SourceNode>();
    auto val4 = std::make_shared<SourceNode>();
    auto sumOp = std::make_shared<SumOpNode>();
    auto subOp = std::make_shared<SubOpNode>();

    // Make list of nodes
    Graph graph;
    graph.insertNode(val1);
    graph.insertNode(val2);
    graph.insertNode(val4);
    graph.insertNode(sumOp);
    graph.insertNode(subOp);

    // Init value nodes
    val1->set(1);
    val2->set(2);
    val4->set(4);

    // 1 + 2
    connect(val1->get, sumOp->lhs);
    connect(val2->get, sumOp->rhs);

    // 4 - (1 + 2)
    connect(val4->get, subOp->lhs);
    connect(sumOp->result, subOp->rhs);

    // Test init
    EXPECT_EQ(val1->get(), 0);
    EXPECT_EQ(val2->get(), 0);
    EXPECT_EQ(val4->get(), 0);
    EXPECT_EQ(sumOp->result(), 0);
    EXPECT_EQ(subOp->result(), 0);

    // Update all nodes
    graph.update();

    // Test results
    EXPECT_EQ(val1->get(), 1);
    EXPECT_EQ(val2->get(), 2);
    EXPECT_EQ(val4->get(), 4);
    EXPECT_EQ(sumOp->result(), 3);
    EXPECT_EQ(subOp->result(), 1);

    // Change source nodes
    val1->set(-1);
    val2->set(-2);
    val4->set(-4);

    // Update all nodes
    graph.update();

    // Test results
    EXPECT_EQ(val1->get(), -1);
    EXPECT_EQ(val2->get(), -2);
    EXPECT_EQ(val4->get(), -4);
    EXPECT_EQ(sumOp->result(), -3);
    EXPECT_EQ(subOp->result(), -1);
}

TEST(Graph, ComplexGraph)
{
    // Setup nodes + graph
    using SourceNode = test::PassThroughNode<int>;
    auto a = std::make_shared<SourceNode>(3);
    auto b = std::make_shared<SourceNode>(5);
    auto c = std::make_shared<SourceNode>(6);
    auto d = std::make_shared<SourceNode>(4);
    auto e = std::make_shared<SourceNode>(6);
    auto final = std::make_shared<SourceNode>(0);

    using SumOp = test::AdditionNode<int>;
    using SubOp = test::SubtractionNode<int>;
    auto a0 = std::make_shared<SumOp>();
    auto a1 = std::make_shared<SumOp>();
    auto a2 = std::make_shared<SumOp>();
    auto a3 = std::make_shared<SumOp>();
    auto s0 = std::make_shared<SubOp>();
    auto s1 = std::make_shared<SubOp>();

    // Build graph
    Graph graph;
    graph.insertNodes(a, b, c, d, e, a0, a1, a2, a3, s0, s1, final);

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

    // Confirm init
    EXPECT_EQ(final->get(), 0);

    // Update graph and check final value
    graph.update();
    EXPECT_EQ(final->get(), 16);

    // Make a change to the inputs and check output
    b->set(1);
    c->set(1);
    graph.update();
    EXPECT_EQ(final->get(), 7);
}

TEST(Graph, BasicCachingGraph)
{
    // Setup nodes
    using SourceNode = test::ClassWrapperNode<std::string>;
    using CacheNode = test::StringCachingNode;

    // Caching and serialization requires node registration
    RegisterNode<SourceNode>("smgl::test::ClassWrapperNode<std::string>");
    RegisterNode<CacheNode>();

    // Set cache file
    fs::path cacheFile{"BasicCachingGraph.json"};

    // Build graph
    Graph graph;
    graph.setEnableCache(true);
    graph.setCacheFile(cacheFile);

    // Cleanup old cache dir
    auto cacheDir = graph.cacheDir();
    if (fs::exists(cacheDir)) {
        for (const auto& entry : fs::directory_iterator(cacheDir)) {
            fs::remove_all(entry);
        }
    }

    // Create nodes
    auto src = graph.insertNode<SourceNode>();

    auto cache = std::make_shared<CacheNode>();
    std::string uuid("00000000-1111-2222-3333-444444444444");
    cache->setUuid(Uuid::FromString(uuid));
    graph.insertNode(cache);

    // Connect nodes
    src->get >> cache->value;

    // Post a starting value
    src->set("Starting value");
    graph.update();

    // Check that the cached value matches
    EXPECT_EQ(LoadCachedValue(cacheFile, uuid), src->get());

    // Post an update
    src->set("Second value");
    graph.update();

    // Check that the cached value matches
    EXPECT_EQ(LoadCachedValue(cacheFile, uuid), src->get());

    // Post an update we don't want cached
    src->set("Shouldn't be cached in StringCacheNode");
    graph.setEnableCache(false);
    graph.update();

    // Check that the cached value matches
    EXPECT_NE(LoadCachedValue(cacheFile, uuid), src->get());

    // Cleanup registration
    DeregisterNode<SourceNode>();
    DeregisterNode<CacheNode>();
}

TEST(Graph, SerializationDeserialization)
{
    // Setup nodes
    using SourceNode = test::ClassWrapperNode<int>;
    using SumOpNode = test::AdditionNode<int>;

    // Caching and serialization requires node registration
    RegisterNode<SourceNode>();
    RegisterNode<SumOpNode>();

    // Create nodes
    auto lhs = std::make_shared<SourceNode>();
    auto rhs = std::make_shared<SourceNode>();
    auto sumOp = std::make_shared<SumOpNode>();

    // Build graph
    Graph g;
    g.insertNodes(lhs, rhs, sumOp);

    // Setup connections
    lhs->set(1);
    rhs->set(1);
    connect(lhs->get, sumOp->lhs);
    connect(rhs->get, sumOp->rhs);

    // Compute result
    g.update();

    // Serialize the graph
    fs::path graphFile{"SerializedGraph.json"};
    Graph::Save(graphFile, g);

    // Deserialize the graph
    auto gClone = Graph::Load(graphFile);

    // Check graph equality
    EXPECT_EQ(gClone.uuid(), g.uuid());

    // Check LHS
    Node::Pointer nodeClone;
    EXPECT_NO_THROW(nodeClone = gClone[lhs->uuid()]);
    auto lhsClone = std::dynamic_pointer_cast<SourceNode>(nodeClone);
    EXPECT_EQ(lhsClone->get(), lhs->get());

    // Check RHS
    EXPECT_NO_THROW(nodeClone = gClone[rhs->uuid()]);
    auto rhsClone = std::dynamic_pointer_cast<SourceNode>(nodeClone);
    EXPECT_EQ(rhsClone->get(), rhs->get());

    // Check SumOp
    EXPECT_NO_THROW(nodeClone = gClone[sumOp->uuid()]);
    auto sumClone = std::dynamic_pointer_cast<SumOpNode>(nodeClone);
    EXPECT_EQ(sumClone->result(), sumOp->result());

    DeregisterNode<SourceNode>();
    DeregisterNode<SumOpNode>();
}

TEST(Graph, Scheduling)
{
    // Setup nodes
    using SourceNode = test::ClassWrapperNode<int>;
    using SumOpNode = test::AdditionNode<int>;

    // Build graph
    Graph g;

    // Create nodes
    auto lhs = g.insertNode<SourceNode>();
    auto rhs = g.insertNode<SourceNode>();

    // Generate UUID for our final node so we can identify it
    auto finalID = Uuid::FromString("00000000-1111-2222-3333-444444444444");
    auto sumOp = std::make_shared<SumOpNode>();
    sumOp->setUuid(finalID);
    g.insertNode(sumOp);

    // Setup connections
    lhs->set(1);
    rhs->set(1);
    connect(lhs->get, sumOp->lhs);
    connect(rhs->get, sumOp->rhs);

    // Sum Op must be final node
    auto schedule = Graph::Schedule(g);
    EXPECT_EQ(schedule[2]->uuid(), finalID);
}