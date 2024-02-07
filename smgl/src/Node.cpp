#include "smgl/Node.hpp"

#include "smgl/LoggingPrivate.hpp"
#include "smgl/Utilities.hpp"

using namespace smgl;

Node::Node() : usesCacheDir{[]() { return false; }} {}

Node::Node(bool usesCacheDir)
    : usesCacheDir{[usesCacheDir]() { return usesCacheDir; }}
{
}

void Node::update()
{
    // Check if inputs have updated
    LogDebug("[Node::update]", "Updating input ports");
    if (!update_input_ports_()) {
        LogDebug("[Node::update]", "Ports have no updates");
        return;
    }

    // Compute
    LogDebug("[Node::update]", "Notifying output ports");
    notify_output_ports_(Port::State::Waiting);
    if (compute) {
        LogDebug("[Node::update]", "Calling compute");
        compute();
    }

    // Update outputs
    LogDebug("[Node::update]", "Updating output ports");
    update_output_ports_();
}

auto Node::serialize(bool useCache, const filesystem::path& cacheRoot)
    -> Metadata
{
    LogDebug("[Node::serialize]", "Building metadata");
    Metadata meta;
    meta["type"] = NodeName(this);
    meta["uuid"] = uuid_.string();

    // Make sure that the Node specifies that it uses the cacheDir
    // This stops the node from reading from a non-existent directory if
    // usesCacheDir() == false and useCache == true
    useCache &= usesCacheDir();

    // Construct the cache dir if needed
    auto nodeCache = cacheRoot / uuid_.string();
    if (useCache and not filesystem::exists(nodeCache)) {
        LogDebug(
            "[Node::serialize]",
            "Creating cache directory:", nodeCache.string());
        filesystem::create_directories(nodeCache);
    }

    // Serialize port info
    LogDebug("[Node::serialize]", "Serializing input ports");
    meta["inputPorts"] = Metadata::object();
    for (const auto& ip : inputs_by_name_) {
        meta["inputPorts"][ip.first] = ip.second->serialize();
    }
    LogDebug("[Node::serialize]", "Serializing output ports");
    meta["outputPorts"] = Metadata::object();
    for (const auto& op : outputs_by_name_) {
        meta["outputPorts"][op.first] = op.second->serialize();
    }

    // Serialize the node
    LogDebug("[Node::serialize]", "Serializing child class");
    meta["data"] = serialize_(useCache, nodeCache);

    return meta;
}

void Node::deserialize(const Metadata& meta, const filesystem::path& cacheRoot)
{
    uuid_ = Uuid::FromString(meta["uuid"].get<std::string>());
    LogDebug("[Node::deserialize]", "Node:", uuid_.string());

    // Deserialize port info
    LogDebug("[Node::deserialize]", "Loading input ports");
    for (const auto& n : meta["inputPorts"].items()) {
        LoadAndRegisterPort(
            n.key(), n.value(), inputs_by_uuid_, inputs_by_name_);
    }
    LogDebug("[Node::deserialize]", "Loading output ports");
    for (const auto& n : meta["outputPorts"].items()) {
        LoadAndRegisterPort(
            n.key(), n.value(), outputs_by_uuid_, outputs_by_name_);
    }

    // Load custom node state
    auto nodeCache = cacheRoot / meta["uuid"].get<std::string>();
    LogDebug("[Node::serialize]", "Cache directory:", nodeCache.string());
    LogDebug("[Node::serialize]", "Deserializing child class");
    deserialize_(meta["data"], nodeCache);
}

auto Node::getInputPort(const Uuid& uuid) -> Input&
{
    return *inputs_by_uuid_.at(uuid);
}

auto Node::getInputPort(const std::string& name) -> Input&
{
    return *inputs_by_name_.at(name);
}

auto Node::getOutputPort(const Uuid& uuid) -> Output&
{
    return *outputs_by_uuid_.at(uuid);
}

auto Node::getOutputPort(const std::string& name) -> Output&
{
    return *outputs_by_name_.at(name);
}

auto Node::getInputPortsInfo() const -> std::vector<Node::Info>
{

    std::vector<Info> info;
    info.reserve(inputs_by_name_.size());
    for (const auto& n : inputs_by_name_) {
        info.emplace_back(n.first, n.second->uuid());
    }
    return info;
}

auto Node::getOutputPortsInfo() const -> std::vector<Node::Info>
{
    std::vector<Info> info;
    info.reserve(outputs_by_name_.size());
    for (const auto& n : outputs_by_name_) {
        info.emplace_back(n.first, n.second->uuid());
    }
    return info;
}

auto Node::getInputConnections() const -> std::vector<Connection>
{
    std::vector<Connection> cns;
    for (const auto& ip : inputs_by_name_) {
        auto pcns = ip.second->getConnections();
        cns.insert(cns.end(), pcns.begin(), pcns.end());
    }
    return cns;
}

auto Node::getNumberOfInputConnections() const -> size_t
{
    size_t cns{0};
    for (const auto& n : inputs_by_name_) {
        cns += n.second->numConnections();
    }
    return cns;
}

auto Node::getOutputConnections() const -> std::vector<Connection>
{
    std::vector<Connection> cns;
    for (const auto& op : outputs_by_name_) {
        auto pcns = op.second->getConnections();
        cns.insert(cns.end(), pcns.begin(), pcns.end());
    }
    return cns;
}

auto Node::getNumberOfOutputConnections() const -> size_t
{
    size_t cns{0};
    for (const auto& n : outputs_by_name_) {
        cns += n.second->numConnections();
    }
    return cns;
}

auto Node::state() -> Node::State
{
    // TODO: Lock internal state
    if (state_ == State::Updating or state_ == State::Error) {
        return state_;
    }

    // Check state of input ports
    auto queued = false;
    for (const auto& ip : inputs_by_name_) {
        auto status = ip.second->state();

        // Can break early if any are waiting
        if (status == Port::State::Waiting) {
            return State::Waiting;
        }

        // Give all ports a chance to be waiting, so keep track
        // of whether any port is queued
        queued |= status == Port::State::Queued;
    }

    // Return port statuses
    if (queued) {
        return State::Ready;
    } else {
        return State::Idle;
    }
}

auto Node::serialize_(bool useCache, const filesystem::path& cacheDir)
    -> Metadata
{
    return Metadata::object();
}

void Node::deserialize_(const Metadata& data, const filesystem::path& cacheDir)
{
}

auto Node::update_input_ports_() -> bool
{
    auto res = false;
    for (const auto& p : inputs_by_name_) {
        res |= p.second->update();
    }
    return res;
}

void Node::notify_output_ports_(Port::State s)
{
    for (const auto& p : outputs_by_name_) {
        p.second->notify(s);
    }
}

auto Node::update_output_ports_() -> bool
{
    auto res = false;
    for (const auto& p : outputs_by_name_) {
        p.second->setState(Port::State::Idle);
        res |= p.second->update();
    }
    return res;
}

auto smgl::DeregisterNode(const std::string& name) -> bool
{
    return detail::NodeFactoryType::Instance().Deregister(name);
}

auto smgl::CreateNode(const std::string& name) -> std::shared_ptr<Node>
{
    return detail::NodeFactoryType::Instance().CreateObject(name);
}

auto smgl::NodeName(const Node::Pointer& node) -> std::string
{
    if (node == nullptr) {
        throw std::invalid_argument("node is a nullptr");
    }
    auto& n = *node.get();
    return detail::NodeFactoryType::Instance().GetTypeIdentifier(typeid(n));
}

auto smgl::NodeName(const Node* node) -> std::string
{
    if (node == nullptr) {
        throw std::invalid_argument("node is a nullptr");
    }
    return detail::NodeFactoryType::Instance().GetTypeIdentifier(typeid(*node));
}

auto smgl::IsRegistered(const std::string& name) -> bool
{
    return detail::NodeFactoryType::Instance().IsRegistered(name);
}

auto smgl::IsRegistered(const Node::Pointer& node) -> bool
{
    if (node == nullptr) {
        throw std::invalid_argument("node is a nullptr");
    }
    auto& n = *node.get();
    return detail::NodeFactoryType::Instance().IsRegistered(typeid(n));
}

auto smgl::IsRegistered(const Node* node) -> bool
{
    if (node == nullptr) {
        throw std::invalid_argument("node is a nullptr");
    }
    return detail::NodeFactoryType::Instance().IsRegistered(typeid(*node));
}