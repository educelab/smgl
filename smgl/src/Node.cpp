#include "smgl/Node.hpp"

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
    if (!update_input_ports_()) {
        return;
    }

    // Compute
    notify_output_ports_(Port::Status::Waiting);
    if (compute) {
        compute();
    }

    // Update outputs
    update_output_ports_();
}

Metadata Node::serialize(bool useCache, const filesystem::path& cacheRoot)
{
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
        filesystem::create_directories(nodeCache);
    }

    // Serialize port info
    meta["inputPorts"] = Metadata::object();
    for (const auto& ip : inputs_by_name_) {
        meta["inputPorts"][ip.first] = ip.second->serialize();
    }
    meta["outputPorts"] = Metadata::object();
    for (const auto& op : outputs_by_name_) {
        meta["outputPorts"][op.first] = op.second->serialize();
    }

    // Serialize the node
    meta["data"] = serialize_(useCache, nodeCache);

    return meta;
}

void Node::deserialize(const Metadata& meta, const filesystem::path& cacheRoot)
{
    uuid_ = Uuid::FromString(meta["uuid"].get<std::string>());

    // Deserialize port info
    for (const auto& n : meta["inputPorts"].items()) {
        LoadAndRegisterPort(
            n.key(), n.value(), inputs_by_uuid_, inputs_by_name_);
    }
    for (const auto& n : meta["outputPorts"].items()) {
        LoadAndRegisterPort(
            n.key(), n.value(), outputs_by_uuid_, outputs_by_name_);
    }

    // Load custom node state
    auto nodeCache = cacheRoot / meta["uuid"].get<std::string>();
    deserialize_(meta["data"], nodeCache);
}

Input& Node::getInputPort(const Uuid& uuid)
{
    return *inputs_by_uuid_.at(uuid);
}

Input& Node::getInputPort(const std::string& name)
{
    return *inputs_by_name_.at(name);
}

Output& Node::getOutputPort(const Uuid& uuid)
{
    return *outputs_by_uuid_.at(uuid);
}

Output& Node::getOutputPort(const std::string& name)
{
    return *outputs_by_name_.at(name);
}

std::vector<Node::Info> Node::getInputPortsInfo() const
{

    std::vector<Info> info;
    for (const auto& n : inputs_by_name_) {
        info.emplace_back(n.first, n.second->uuid());
    }
    return info;
}

std::vector<Node::Info> Node::getOutputPortsInfo() const
{
    std::vector<Info> info;
    for (const auto& n : outputs_by_name_) {
        info.emplace_back(n.first, n.second->uuid());
    }
    return info;
}

std::vector<Connection> Node::getInputConnections() const
{
    std::vector<Connection> cns;
    for (const auto& ip : inputs_by_name_) {
        auto pcns = ip.second->getConnections();
        cns.insert(cns.end(), pcns.begin(), pcns.end());
    }
    return cns;
}

size_t Node::getNumberOfInputConnections() const
{
    size_t cns{0};
    for (const auto& n : inputs_by_name_) {
        cns += n.second->numConnections();
    }
    return cns;
}

std::vector<Connection> Node::getOutputConnections() const
{
    std::vector<Connection> cns;
    for (const auto& op : outputs_by_name_) {
        auto pcns = op.second->getConnections();
        cns.insert(cns.end(), pcns.begin(), pcns.end());
    }
    return cns;
}

size_t Node::getNumberOfOutputConnections() const
{
    size_t cns{0};
    for (const auto& n : outputs_by_name_) {
        cns += n.second->numConnections();
    }
    return cns;
}

Node::Status Node::status()
{
    // TODO: Lock internal status
    if (status_ == Status::Updating or status_ == Status::Error) {
        return status_;
    }

    // Check status of input ports
    auto queued = false;
    for (const auto& ip : inputs_by_name_) {
        auto status = ip.second->status();

        // Can break early if any are waiting
        if (status == Port::Status::Waiting) {
            return Status::Waiting;
        }

        // Give all ports a chance to be waiting, so keep track
        // of whether any port is queued
        queued |= status == Port::Status::Queued;
    }

    // Return port statuses
    if (queued) {
        return Status::Ready;
    } else {
        return Status::Idle;
    }
}

Metadata Node::serialize_(bool useCache, const filesystem::path& cacheDir)
{
    return Metadata::object();
}

void Node::deserialize_(const Metadata& data, const filesystem::path& cacheDir)
{
}

bool Node::update_input_ports_()
{
    auto res = false;
    for (const auto& p : inputs_by_name_) {
        res |= p.second->update();
    }
    return res;
}

void Node::notify_output_ports_(Port::Status s)
{
    for (const auto& p : outputs_by_name_) {
        p.second->notify(s);
    }
}

bool Node::update_output_ports_()
{
    auto res = false;
    for (const auto& p : outputs_by_name_) {
        p.second->setStatus(Port::Status::Idle);
        res |= p.second->update();
    }
    return res;
}

bool smgl::DeregisterNode(const std::string& name)
{
    return detail::NodeFactoryType::Instance().Deregister(name);
}

std::shared_ptr<Node> smgl::CreateNode(const std::string& name)
{
    return detail::NodeFactoryType::Instance().CreateObject(name);
}

std::string smgl::NodeName(const Node::Pointer& node)
{
    if (node == nullptr) {
        throw std::invalid_argument("node is a nullptr");
    }
    auto& n = *node.get();
    return detail::NodeFactoryType::Instance().GetTypeIdentifier(typeid(n));
}

std::string smgl::NodeName(const Node* node)
{
    if (node == nullptr) {
        throw std::invalid_argument("node is a nullptr");
    }
    return detail::NodeFactoryType::Instance().GetTypeIdentifier(typeid(*node));
}

bool smgl::IsRegistered(const std::string& name)
{
    return detail::NodeFactoryType::Instance().IsRegistered(name);
}

bool smgl::IsRegistered(const Node::Pointer& node)
{
    if (node == nullptr) {
        throw std::invalid_argument("node is a nullptr");
    }
    auto& n = *node.get();
    return detail::NodeFactoryType::Instance().IsRegistered(typeid(n));
}

bool smgl::IsRegistered(const Node* node)
{
    if (node == nullptr) {
        throw std::invalid_argument("node is a nullptr");
    }
    return detail::NodeFactoryType::Instance().IsRegistered(typeid(*node));
}