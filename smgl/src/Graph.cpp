#include "smgl/Graph.hpp"

#include <functional>

#include "smgl/Metadata.hpp"
#include "smgl/Uuid.hpp"

using namespace smgl;
namespace fs = filesystem;

// Must declare const static member in cpp
// https://stackoverflow.com/a/53350948
#if __cplusplus < 201703L
constexpr uint32_t Graph::Version;
#endif

inline fs::path CacheDir(const fs::path& json, CacheType t)
{
    switch (t) {
        case CacheType::Adjacent:
            return json.parent_path();
        case CacheType::Subdirectory:
            return json.parent_path() / (json.stem().string() + "_cache");
    }
}

Node::Pointer Graph::operator[](const Uuid& uuid) const
{
    auto it = nodes_.find(uuid);
    if (it != nodes_.end()) {
        return it->second;
    } else {
        throw std::invalid_argument("Node not in graph: " + uuid.string());
    }
}

void Graph::insertNode(const Node::Pointer& n) { nodes_[n->uuid()] = n; }

void Graph::removeNode(const Node::Pointer& n)
{
    nodes_.erase(n->uuid());
    // TODO: Remove all Node connections
}

std::size_t Graph::size() const { return nodes_.size(); }

Graph::Status Graph::status() const { return status_; }

fs::path Graph::cacheFile() const
{
    if (cacheFile_.empty()) {
        return uuid().string() + ".json";
    } else {
        return cacheFile_;
    }
}

void Graph::setCacheFile(const fs::path& p) { cacheFile_ = p; }

CacheType Graph::cacheType() const { return cacheType_; }

void Graph::setCacheType(CacheType t) { cacheType_ = t; }

fs::path Graph::cacheDir() const { return CacheDir(cacheFile(), cacheType_); }

bool Graph::cacheEnabled() const { return cache_enabled_; }

void Graph::setEnableCache(bool enable) { cache_enabled_ = enable; }

void Graph::setProjectMetadata(const Metadata& m) { extraMetadata_ = m; }

const Metadata& Graph::projectMetadata() const { return extraMetadata_; }

Metadata& Graph::projectMetadata() { return extraMetadata_; }

Graph::Status Graph::update()
{
    // If already operating or in error, return
    if (status_ == Status::Updating or status_ == Status::Error) {
        return status_;
    }

    // Schedule nodes
    auto schedule = Schedule(*this);

    // Setup the cache info
    auto cacheJson = cacheFile();
    auto cacheDir = CacheDir(cacheJson, cacheType_);

    // Write the graph starting state
    Metadata meta;
    if (cache_enabled_) {
        meta = Serialize(*this, cache_enabled_, cacheDir);
        WriteMetadata(cacheJson, meta);
    }

    // Execute our schedule
    status_ = Status::Updating;
    for (auto& n : schedule) {
        auto status = n->status();
        if (status == Node::Status::Ready) {
            n->update();

            // Write to cache
            if (cache_enabled_) {
                // Write to the cache
                auto uuid = n->uuid().string();
                meta["nodes"][uuid] = n->serialize(cache_enabled_, cacheDir);
                WriteMetadata(cacheJson, meta);
            }
        } else if (
            status == Node::Status::Waiting or
            status == Node::Status::Updating) {
            throw std::runtime_error("Node not ready but scheduled for update");
        } else if (status == Node::Status::Error) {
            throw std::runtime_error("Node update error");
        }
    }
    status_ = Status::Idle;
    return status_;
}

Metadata Graph::Serialize(const Graph& g)
{
    return Serialize(g, g.cache_enabled_, CacheDir(g.cacheFile_, g.cacheType_));
}

Metadata Graph::Serialize(
    const Graph& g, bool useCache, const fs::path& cacheDir)
{
    Metadata meta{
        {"software", "smgl"},
        {"type", "graph"},
        {"version", Graph::Version},
        {"uuid", g.uuid().string()}};

    // Serialize the project metadata if available
    if (not g.projectMetadata().empty()) {
        meta["project"] = g.projectMetadata();
    }

    if (useCache) {
        switch (g.cacheType_) {
            case CacheType::Adjacent:
                meta["cacheDir"] = ".";
                break;
            case CacheType::Subdirectory:
                meta["cacheDir"] = cacheDir.filename().string();
                break;
        }
    }

    Metadata connections = Metadata::array();
    meta["nodes"] = Metadata::object();
    for (const auto& n : g.nodes_) {
        // Write node metadata
        auto uuid = n.first.string();
        meta["nodes"][uuid] = n.second->serialize(useCache, cacheDir);

        // Accumulate connections metadata
        for (const auto& c : n.second->getOutputConnections()) {
            // If any of these are nullptr, we have problems
            assert(c.srcNode != nullptr);
            assert(c.srcPort != nullptr);
            assert(c.destNode != nullptr);
            assert(c.destPort != nullptr);

            connections.push_back(
                {{"srcNode", c.srcNode->uuid().string()},
                 {"srcPort", c.srcPort->uuid().string()},
                 {"destNode", c.destNode->uuid().string()},
                 {"destPort", c.destPort->uuid().string()}});
        }
    }
    meta["connections"] = connections;

    return meta;
}

void Graph::Save(const fs::path& path, const Graph& g, bool writeCache)
{
    // Construct the metadata
    auto meta = Serialize(g, writeCache, CacheDir(path, g.cacheType_));

    WriteMetadata(path, meta);
}

Graph Graph::Load(const fs::path& path)
{
    // Load the metadata
    auto meta = LoadMetadata(path);

    // Validate the json file
    if (not(meta.contains("software") and meta["software"] == "smgl")) {
        throw std::runtime_error("File not generated by smgl");
    }
    if (not(meta.contains("type") and meta["type"] == "graph")) {
        throw std::runtime_error("File not a smgl Graph");
    }

    // Setup a new graph
    Graph g;
    g.cacheFile_ = path;

    // Load the cache directory
    fs::path cacheDir;
    if (meta.contains("cacheDir")) {
        cacheDir = meta["cacheDir"].get<std::string>();
        if (cacheDir == ".") {
            g.cacheType_ = CacheType::Adjacent;
            cacheDir = path.parent_path();
        } else {
            g.cacheType_ = CacheType::Subdirectory;
        }
    } else {
        cacheDir = path.parent_path();
    }

    // Load the graph UUID
    g.uuid_ = Uuid::FromString(meta["uuid"].get<std::string>());

    // Load the nodes
    for (const auto& node : meta["nodes"].items()) {
        auto nodeMeta = node.value();
        // Construct the node
        auto type = nodeMeta["type"].get<std::string>();
        auto n = CreateNode(type);

        // Load the node state
        n->deserialize(nodeMeta, cacheDir);

        // Add to the graph
        g.insertNode(n);
    }

    // Make connections
    for (const auto& c : meta["connections"]) {
        // Get the nodes
        auto srcNID = Uuid::FromString(c["srcNode"].get<std::string>());
        auto srcNode = g[srcNID];
        auto dstNID = Uuid::FromString(c["destNode"].get<std::string>());
        auto dstNode = g[dstNID];

        // Connect the ports
        auto srcPID = Uuid::FromString(c["srcPort"].get<std::string>());
        auto dstPID = Uuid::FromString(c["destPort"].get<std::string>());
        connect(srcNode->getOutputPort(srcPID), dstNode->getInputPort(dstPID));
    }

    return g;
}

std::vector<std::string> Graph::CheckRegistration(const fs::path& path)
{
    // Load the metadata
    auto meta = LoadMetadata(path);

    // Validate the json file
    if (not(meta.contains("software") and meta["software"] == "smgl")) {
        throw std::runtime_error("File not generated by smgl");
    }
    if (not(meta.contains("type") and meta["type"] == "graph")) {
        throw std::runtime_error("File not a smgl Graph");
    }

    // Check that all nodes are registered
    std::vector<std::string> ids;
    for (const auto& node : meta["nodes"].items()) {
        auto nodeMeta = node.value();
        auto type = nodeMeta["type"].get<std::string>();
        if (not IsRegistered(type)) {
            ids.emplace_back(type);
        }
    }
    return ids;
}

std::vector<std::string> Graph::CheckRegistration(const Graph& g)
{
    std::vector<std::string> ids;
    for (const auto& n : g.nodes_) {
        if (not IsRegistered(n.second)) {
            auto& nref = *n.second;
            ids.emplace_back(detail::type_name(nref));
        }
    }
    return ids;
}

std::vector<Node::Pointer> Graph::Schedule(const Graph& g)
{
    // Node helper struct
    struct NodeHelper {
        using Ptr = std::shared_ptr<NodeHelper>;
        Uuid uuid;
        Ptr parent;
        size_t inCns{0};
        std::vector<Uuid> cns;
        size_t start{0};
        size_t end{0};
        bool visited{false};
    };

    // TODO: Detect whether DAG

    // Setup node list
    std::unordered_map<Uuid, NodeHelper::Ptr> ns;
    for (const auto& node : g.nodes_) {
        // Make the node helper
        auto n = std::make_shared<NodeHelper>();
        n->uuid = node.first;
        n->inCns = node.second->getNumberOfInputConnections();
        // Add connections
        for (const auto& c : node.second->getOutputConnections()) {
            n->cns.push_back(c.destNode->uuid());
        }
        // Assign to the node list
        ns[node.first] = n;
    }

    // Setup DFS++
    size_t time{0};
    using VisitFnT = std::function<void(NodeHelper::Ptr&)>;
    VisitFnT visit = [&time, &visit, &ns](NodeHelper::Ptr& u) {
        if (u->visited) {
            return;
        }

        time++;
        u->start = time;
        u->visited = true;
        for (auto& c : u->cns) {
            auto v = ns[c];
            if (not v->visited) {
                v->parent = u;
                visit(v);
            }
        }
        time++;
        u->end = time;
    };

    // Run DFS++
    for (auto& n : ns) {
        if (n.second->inCns == 0) {
            visit(n.second);
        }
    }

    // Sort the results by decreasing end time
    std::vector<NodeHelper::Ptr> dfs;
    dfs.reserve(ns.size());
    for (const auto& n : ns) {
        if (not n.second->visited) {
            throw std::runtime_error("Unscheduled node: " + n.first.string());
        }
        dfs.emplace_back(n.second);
    }
    std::sort(dfs.begin(), dfs.end(), [](const auto& lhs, const auto& rhs) {
        return lhs->end > rhs->end;
    });

    // Convert to a schedule
    std::vector<Node::Pointer> schedule;
    schedule.reserve(dfs.size());
    for (const auto& n : dfs) {
        schedule.emplace_back(g[n->uuid]);
    }
    return schedule;
}
