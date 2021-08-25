namespace smgl
{

/////////////////////
///// InputPort /////
/////////////////////

// Must know what object will actually store posted values
template <typename T>
InputPort<T>::InputPort(T* target) : target_{[target](T v) { *target = v; }}
{
}

template <typename T>
InputPort<T>::InputPort(std::function<void(T)> target) : target_{target}
{
}

template <typename T>
template <class Obj, class ObjMemberFn>
InputPort<T>::InputPort(Obj* obj, ObjMemberFn&& fn)
    : target_{[=](T v) { return (*obj.*fn)(v); }}
{
}

template <typename T>
void InputPort<T>::post(const Update<T>& u)
{
    // TODO: Lock the queue
    queued_update_ = u;
    status_ = Status::Queued;
}

// For testing purposes only
template <typename T>
void InputPort<T>::post(T v, bool immediate)
{
    post({Clock::now(), v});
    if (immediate) {
        update();
    }
}

template <typename T>
void InputPort<T>::operator()(const Update<T>& u)
{
    post(u);
}

template <typename T>
void InputPort<T>::operator()(T v, bool immediate)
{
    post(v, immediate);
}

template <typename T>
InputPort<T>& InputPort<T>::operator=(T v)
{
    post(v, false);
    return *this;
}

template <typename T>
InputPort<T>& InputPort<T>::operator=(Output& op)
{
    Input::operator=(op);
    return *this;
}

// Update target with most recent compute
template <typename T>
bool InputPort<T>::update()
{
    // TODO: Lock the queue
    if (queued_update_.time > last_updated_) {
        target_(queued_update_.val);
        last_updated_ = queued_update_.time;
        status_ = Status::Idle;
        return true;
    }
    return false;
}

template <typename T>
void InputPort<T>::notify(Status s)
{
    last_updated_ = Clock::now();
    status_ = s;
}

template <typename T>
Metadata InputPort<T>::serialize()
{
    Metadata m;
    m["uuid"] = uuid_.string();
    // TODO: STATUS
    return m;
}

template <typename T>
void InputPort<T>::deserialize(const Metadata& m)
{
    uuid_ = Uuid::FromString(m["uuid"].get<std::string>());
}

//////////////////////
///// OutputPort /////
//////////////////////

template <typename T, typename... Args>
OutputPort<T, Args...>::~OutputPort()
{
    for (auto& c : connections_) {
        c.second.port->disconnect(this);
    }
}

// Object source: constant
template <typename T, typename... Args>
OutputPort<T, Args...>::OutputPort(T source)
    : source_{[source](Args...) { return source; }}
{
}
// Object source: pointer
template <typename T, typename... Args>
OutputPort<T, Args...>::OutputPort(T* source)
    : source_{[source](Args...) { return *source; }}
{
}

// Member fn source w/optional default arguments
template <typename T, typename... Args>
OutputPort<T, Args...>::OutputPort(
    std::function<T(Args...)> source, Args&&... args)
    : source_{source}, args_{Arguments(std::forward<Args>(args)...)}
{
}

template <typename T, typename... Args>
template <class Obj, class ObjMemberFn>
OutputPort<T, Args...>::OutputPort(Obj* obj, ObjMemberFn&& fn, Args&&... args)
    : source_{[=](Args... a) { return (*obj.*fn)(a...); }}
    , args_{Arguments(std::forward<Args>(args)...)}
{
}

template <typename T, typename... Args>
std::vector<Connection> OutputPort<T, Args...>::getConnections() const
{
    using ThisType = OutputPort<T, Args...>;
    std::vector<Connection> cns;
    for (const auto& c : connections_) {
        cns.emplace_back(
            parent_, const_cast<ThisType*>(this), c.second.node, c.second.port);
    }
    return cns;
}

template <typename T, typename... Args>
size_t OutputPort<T, Args...>::numConnections() const
{
    return connections_.size();
}

// Replace the default arguments
template <typename T, typename... Args>
void OutputPort<T, Args...>::setArgs(Args&&... args)
{
    args_ = Arguments(std::forward<Args>(args)...);
}
// Get the most recent value
template <typename T, typename... Args>
T OutputPort<T, Args...>::val()
{
    return run_(args_);
}
template <typename T, typename... Args>
T OutputPort<T, Args...>::operator()()
{
    return val();
}

// Send queued update to connected input ports
// Note: This is like calling emit() in Qt
template <typename T, typename... Args>
bool OutputPort<T, Args...>::update()
{
    Update<T> update{std::chrono::steady_clock::now(), val()};
    for (const auto& c : connections_) {
        c.second.port->post(update);
    }
    return connections_.size() > 0;
}

template <typename T, typename... Args>
void OutputPort<T, Args...>::notify(Status s)
{
    for (const auto& c : connections_) {
        c.second.port->notify(s);
    }
}

template <typename T, typename... Args>
Metadata OutputPort<T, Args...>::serialize()
{
    Metadata m;
    m["uuid"] = uuid_.string();
    return m;
}

template <typename T, typename... Args>
void OutputPort<T, Args...>::deserialize(const Metadata& m)
{
    uuid_ = Uuid::FromString(m["uuid"].get<std::string>());
}

// Redirection functions for calling with default args
template <typename T, typename... Args>
template <std::size_t... Is>
T OutputPort<T, Args...>::run_(
    std::tuple<Args...>& tup, std::index_sequence<Is...>)
{
    return source_(std::get<Is>(tup)...);
}

template <typename T, typename... Args>
T OutputPort<T, Args...>::run_(std::tuple<Args...>& tup)
{
    return run_(tup, std::index_sequence_for<Args...>{});
}

template <typename T, typename... Args>
void OutputPort<T, Args...>::connect(Input* ip)
{
    if (typeid(*ip) != typeid(InputPort<T>)) {
        throw bad_connection("Ports not of same type");
    }
    auto typedIP = static_cast<InputPort<T>*>(ip);
    connections_[ip->uuid()] = {ip->parent_, typedIP};
    if (status_ == Status::Idle) {
        Update<T> update{std::chrono::steady_clock::now(), val()};
        typedIP->post(update);
    }
}

template <typename T, typename... Args>
void OutputPort<T, Args...>::disconnect(Input* ip)
{
    if (connections_.erase(ip->uuid()) == 0) {
        // TODO: Throw error?
    }
}

}  // namespace smgl
