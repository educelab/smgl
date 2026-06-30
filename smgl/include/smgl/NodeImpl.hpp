#include "smgl/Utilities.hpp"

namespace smgl
{

template <class PortType>
void Node::LoadAndRegisterPort(
    const std::string& name,
    const Metadata& data,
    std::unordered_map<Uuid, PortType*>& byUuid,
    std::map<std::string, PortType*>& byName)
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

template <typename T>
void Node::registerPort(const std::string& name, InputPort<T>& port)
{
    registerInputPort(name, port);
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

template <typename T, typename... Args>
void Node::registerPort(const std::string& name, OutputPort<T, Args...>& port)
{
    registerOutputPort(name, port);
}

template <class... Ts>
bool RegisterNodes(const NodeDesc<Ts>&... descs)
{
    // Reserve nodes
    using NF = detail::NodeFactoryType;
    NF::Instance().ReserveAdditional(sizeof...(Ts));
    bool res{true};
#if __cplusplus >= 201703L
    ((res = res & RegisterNode<Ts>(descs.key)), ...);
#elif __cplusplus > 201103L
    detail::ExpandType{0, res &= RegisterNode<Ts>(descs.key)...};
#endif
    return res;
}

template <class T>
bool RegisterNode(const std::string& name)
{
    return detail::NodeFactoryType::Instance().Register(
        name, []() { return std::make_shared<T>(); }, typeid(T));
}

template <class T, class... Ts>
bool DeregisterNode()
{
    // Lambda function for deregistering
    bool res{true};
    auto deregister = [&res](const std::type_info& i) {
        auto name = detail::NodeFactoryType::Instance().GetTypeIdentifier(i);
        res &= detail::NodeFactoryType::Instance().Deregister(name);
    };
    // Deregister the solo arg
    deregister(typeid(T));
    // Deregister the template argument pack
#if __cplusplus >= 201703L
    (deregister(typeid(Ts)), ...);
#elif __cplusplus > 201103L
    detail::ExpandType{0, (deregister(typeid(Ts)), 0)...};
#endif
    return res;
}

template <class... Ts>
bool DeregisterNodes(const NodeDesc<Ts>&... descs)
{
    bool res{true};
#if __cplusplus >= 201703L
    ((res = res & DeregisterNode(descs.key)), ...);
#elif __cplusplus > 201103L
    detail::ExpandType{0, res &= DeregisterNode(descs.key)...};
#endif
    return res;
}

template <class T>
std::string NodeName()
{
    return detail::NodeFactoryType::Instance().GetTypeIdentifier(typeid(T));
}

}  // namespace smgl