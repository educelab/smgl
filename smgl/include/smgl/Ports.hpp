#pragma once

/** @file */

#include <chrono>
#include <exception>
#include <functional>
#include <tuple>
#include <unordered_map>
#include <vector>

#include "smgl/Metadata.hpp"
#include "smgl/Uuid.hpp"

namespace smgl
{

/** @brief Exception thrown for port connection failures */
struct bad_connection : public std::exception {
public:
    /** Construct with message */
    explicit bad_connection(const char* msg) : msg_(msg) {}
    /** Construct with message */
    explicit bad_connection(std::string msg) : msg_(std::move(msg)) {}
    /** Get message */
    const char* what() const noexcept override { return msg_.c_str(); }

protected:
    /** Exception message */
    std::string msg_;
};

/** Timestamp clock type */
using Clock = std::chrono::steady_clock;
/** Timestamp type */
using Timestamp = Clock::time_point;

/** @brief Typed update sent over connections */
template <typename T>
struct Update {
    /** Time of update generation */
    Timestamp time;
    /** Value of update */
    T val;
};

/** @cond */
class Input;
class Output;
template <typename T>
class InputPort;
template <typename T, typename... Args>
class OutputPort;
class Node;
/** @endcond */

/**
 * @brief Connect an output port to an input port
 *
 * @throws smgl::bad_connection if port instances do not share underlying type T
 */
void connect(Output& op, Input& ip);

/**
 * @brief Disconnect an output port and input port
 *
 * Does nothing if ports are not connected
 */
void disconnect(Output& op, Input& ip);

/** @copydoc connect() */
void operator>>(Output& op, Input& ip);
/** @copydoc connect() */
void operator<<(Input& ip, Output& op);

/** @brief Describes untyped connections from an output port */
struct Connection {
    /** Default constructor */
    Connection() = default;
    /** Construct with node/port end points */
    Connection(Node* s, Output* sp, Node* d, Input* dp)
        : srcNode{s}, srcPort{sp}, destNode{d}, destPort{dp}
    {
    }
    /** Source node pointer */
    Node* srcNode{nullptr};
    /** Source port pointer */
    Output* srcPort{nullptr};
    /** Destination node pointer */
    Node* destNode{nullptr};
    /** Destination port pointer */
    Input* destPort{nullptr};
};

/** @brief Generic port interface */
class Port : public UniquelyIdentifiable
{
public:
    /** Port status types */
    enum class Status { Idle, Waiting, Queued, Error };

    /** Update the port's target with any posted values */
    virtual bool update() = 0;

    /** Notify a port of source status */
    virtual void notify(Status s) = 0;

    /** Return the port's current status */
    Status status() const;

    /** Set the port's status */
    void setStatus(Status s);

    /** Set the port's parent node */
    void setParent(Node* p);

    /** Serialize the port */
    virtual Metadata serialize() = 0;

    /** Deserialize the port */
    virtual void deserialize(const Metadata& m) = 0;

protected:
    /** Default constructor */
    Port() = default;
    /** Construct with status */
    explicit Port(Status s);
    /** Default destructor */
    virtual ~Port() = default;
    /** Current status */
    Status status_{Status::Idle};
    /** Parent node */
    Node* parent_{nullptr};
};

/** @brief Generic input port interface */
class Input : public Port
{
public:
    /** Get a list of port connections */
    std::vector<Connection> getConnections() const;
    /**
     * @brief Get the number of connections
     *
     * 1 if connected to an output port, 0 otherwise.
     */
    std::size_t numConnections() const;

protected:
    /** Default constructor */
    Input();
    /** Destructor performs auto-disconnect */
    ~Input() override;

    /** Connect to an output port */
    virtual void connect(Output* op) final;
    /** Disconnect from an output port */
    virtual void disconnect(Output* op) final;

    /** Pointer to connected output port */
    Output* src_{nullptr};
    /** Timestamp of last received update */
    Timestamp last_updated_;

private:
    /** Friend: smgl::connect */
    friend void connect(Output& op, Input& ip);
    /** Friend: smgl::disconnect */
    friend void disconnect(Output& op, Input& ip);
    /** Friend: typed OutputPort class */
    template <typename OutputValType, typename... OutputArgs>
    friend class OutputPort;
};

/** @brief Generic output port interface */
class Output : public Port
{
public:
    /** Get a list of port connections */
    virtual std::vector<Connection> getConnections() const = 0;
    /** Get the number of port connections */
    virtual std::size_t numConnections() const = 0;

protected:
    /** Default constructor */
    Output();
    /** Default destructor */
    ~Output() override = default;

    /** Connect to an input port */
    virtual void connect(Input* ip) = 0;
    /** Disconnect from an input port */
    virtual void disconnect(Input* ip) = 0;

private:
    /** Friend: smgl::connect */
    friend void connect(Output& op, Input& ip);
    /** Friend: smgl::disconnect */
    friend void disconnect(Output& op, Input& ip);
    /** Friend: Generic Input port class */
    friend Input;
};

/**
 * @brief Typed InputPort class
 *
 * An InputPort receives values via the post() function and passes them
 * to a target object or function. It is similar to a setter function.
 * T should be default constructible.
 *
 * @tparam T Type of object received by this port
 */
template <typename T>
class InputPort : public Input
{
public:
    /** Default destructor */
    ~InputPort() override = default;

    /** @brief Construct with pointer to target object */
    explicit InputPort(T* target);

    /** @brief Construct with std::function target */
    explicit InputPort(std::function<void(T)> target);

    /** @brief Construct with member function target */
    template <class Obj, class ObjMemberFn>
    explicit InputPort(Obj* obj, ObjMemberFn&& fn);

    /** Disable copy */
    InputPort(const InputPort&) = delete;
    /** Disable copy */
    InputPort& operator=(const InputPort&) = delete;
    /** Disable move */
    InputPort(const InputPort&&) = delete;
    /** Disable move */
    InputPort& operator=(const InputPort&&) = delete;

    /**
     * @brief Post an update to the port
     *
     * The target is not updated until update() has been called.
     */
    void post(const Update<T>& u);

    /**
     * @brief Post an update to the port
     *
     * If `immediate = true`, the target will be updated immediately.
     *
     * @warning Should only be used when testing or debugging.
     */
    void post(T v, bool immediate = false);

    /** @copydoc post(const Update<T>& u) */
    void operator()(const Update<T>& u);

    /** @copydoc post(T v, bool immediate = false) */
    void operator()(T v, bool immediate = false);

    /** @brief Update the target with the most recently posted update */
    bool update() override;

    /** @brief Receive a status update from a connected port */
    void notify(Status s) override;

    /** @brief Get port metadata */
    Metadata serialize() override;

    /** @brief Load port metadata */
    void deserialize(const Metadata& m) override;

private:
    /** Target object updated by std::function */
    std::function<void(T)> target_;
    /** Most recently posted update */
    Update<T> queued_update_;
};

/**
 * @brief Typed OutputPort class
 *
 * An OutputPort provides access to the value of its source. It is similar to a
 * getter function. On `update()`, it posts the current value of source to all
 * connected InputPorts. T should be default constructible.
 *
 * @tparam T Type of object received by this port
 */
template <typename T, typename... Args>
class OutputPort : public Output
{
public:
    /**
     * If source_ is a function, a tuple storing a list of arguments that can
     * be passed to source_
     */
    using Arguments = std::tuple<Args...>;

    /** Destructor performs auto-disconnect */
    ~OutputPort() override;

    /**
     * @brief Construct with a copy of source. Source is effectively constant.
     */
    explicit OutputPort(T source);

    /** @brief Construct with a pointer to a source */
    explicit OutputPort(T* source);

    /**
     * @brief Construct with a function source.
     *
     * If calling source requires arguments, the initial value for these
     * arguments must be passed as the last parameter. Argument values can be
     * changed with setArgs().
     */
    explicit OutputPort(std::function<T(Args...)> source, Args&&... args);

    /**
     * @brief Construct with a member function source
     *
     * If calling source requires arguments, the initial value for these
     * arguments must be passed as the last parameter. Argument values can be
     * changed with setArgs().
     */
    template <class Obj, class ObjMemberFn>
    explicit OutputPort(Obj* obj, ObjMemberFn&& fn, Args&&... args);

    /** Disable copy */
    OutputPort(const OutputPort&) = delete;
    /** Disable copy */
    OutputPort& operator=(const OutputPort&) = delete;
    /** Disable move */
    OutputPort(const OutputPort&&) = delete;
    /** Disable move */
    OutputPort& operator=(const OutputPort&&) = delete;

    /** @brief Get a list of active connections */
    std::vector<Connection> getConnections() const override;

    /** @brief Get the number of active connections */
    std::size_t numConnections() const override;

    /** @brief Set the arguments passed to a function source */
    void setArgs(Args&&... args);

    /** @brief Get the current value of source */
    T val();
    /** @brief Get the current value of source */
    T operator()();

    /** @brief Update all active connections with the value of the source */
    bool update() override;

    /** @brief Notify all active connections of this port's status */
    void notify(Status s) override;

    /** @brief Get port metadata */
    Metadata serialize() override;

    /** @brief Load port metadata */
    void deserialize(const Metadata& m) override;

private:
    /** Value source */
    std::function<T(Args...)> source_;

    /** If source is a function, the arguments passed to source */
    Arguments args_;

    /** Get source value with arguments */
    template <std::size_t... Is>
    T run_(std::tuple<Args...>& tup, std::index_sequence<Is...>);

    /** Redirect function to get source value with arguments */
    T run_(std::tuple<Args...>& tup);

    /**
     * Connect to an input port. Uses RTTI to determine if ip is actually of
     * type InputPort<T>.
     *
     * @throws smgl::bad_connection if ip is not of type InputPort<T>
     */
    void connect(Input* ip) final;

    /** Disconnect from an input port */
    void disconnect(Input* ip) final;

    /** @brief Describes typed connections from an output port */
    struct TypedConnection {
        /** Pointer to connected port's parent */
        Node* node;
        /** Pointer to connected port */
        InputPort<T>* port;
    };

    /** Stores all outgoing connections */
    std::unordered_map<Uuid, TypedConnection> connections_;
};

}  // namespace smgl

#include "smgl/PortsImpl.hpp"