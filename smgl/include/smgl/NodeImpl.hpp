#include "smgl/Utilities.hpp"

namespace smgl
{

template <class PortType>
void Node::LoadAndRegisterPort(
    const std::string& name,
    const Metadata& data,
    std::unordered_map<Uuid, PortType*>& byUuid,
    std::unordered_map<std::string, PortType*>& byName)
{
    // Get old port info
    auto* port = byName[name];
    auto oldUuid = port->uuid();

    // Load old port data
    port->deserialize(data);

    // Delete old port registration
    auto nIt = byName.find(name);
    assert(nIt != byName.end());
    byName.erase(nIt);

    auto uIt = byUuid.find(oldUuid);
    assert(uIt != byUuid.end());
    byUuid.erase(uIt);

    // Reregister ports
    byName[name] = port;
    byUuid[port->uuid()] = port;
}

template <typename T>
void Node::registerInputPort(const std::string& name, InputPort<T>& port)
{
    port.setParent(this);
    inputs_by_uuid_[port.uuid()] = &port;
    assert(inputs_by_name_.find(name) == inputs_by_name_.end());
    inputs_by_name_[name] = &port;
}

template <typename T, typename... Args>
void Node::registerOutputPort(
    const std::string& name, OutputPort<T, Args...>& port)
{
    port.setParent(this);
    outputs_by_uuid_[port.uuid()] = &port;
    assert(outputs_by_name_.find(name) == outputs_by_name_.end());
    outputs_by_name_[name] = &port;
}

template <class T>
bool RegisterNode()
{
    return RegisterNode<T>(detail::type_name<T>());
}

template <class T>
bool RegisterNode(const std::string& name)
{
    return detail::NodeFactoryType::Instance().Register(
        name, []() { return std::make_shared<T>(); }, typeid(T));
}

template <class T>
bool DeregisterNode()
{
    auto name =
        detail::NodeFactoryType::Instance().GetTypeIdentifier(typeid(T));
    return detail::NodeFactoryType::Instance().Deregister(name);
}

}  // namespace smgl