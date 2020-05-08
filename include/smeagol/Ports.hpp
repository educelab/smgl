#pragma once

#include <functional>
#include <memory>

#include "smeagol/Status.hpp"

namespace smeagol
{

// Ports Base Class
class Port
{
public:
    using Ptr = std::shared_ptr<Port>;
};

template <typename T>
class OutputPort;

template <typename T, typename Ret = void>
class InputPort : public Port
{
public:
    InputPort(T* target)
    {
        target_ = [target](T v) { *target = v; };
    }
    InputPort(std::function<Ret(T)> target) { target_ = target; }

    void source(std::function<T()> src) { source_ = src; }
    void notify(T val) { target_(val); }
    void update() { target_(source_()); }

private:
    std::function<T()> source_;
    std::function<Ret(T)> target_;
};

// Generic Output Port Interface
template <typename T>
class OutputPort : public Port
{
public:
    using Ptr = std::shared_ptr<OutputPort<T>>;
    virtual T val() = 0;

    Status status() { return status_; }
    void status(Status s) { status_ = s; }

    template <typename Ret>
    void connect(InputPort<T, Ret>& p)
    {
        p.source([this]() { return this->val(); });
    }

protected:
    OutputPort() = default;
    Status status_{Status::Ready};
};

// Output port linked to values
template <typename T, typename... Args>
class ValuedOutputPort : public OutputPort<T>
{
public:
    ValuedOutputPort(T val) : val_{[val]() { return val; }} {}
    ValuedOutputPort(T* val) : val_{[val]() { return *val; }} {}
    ValuedOutputPort(std::function<T(Args...)> val) : val_{val} {}

    T val() override { return val_(); }

private:
    std::function<T(Args...)> val_;
};

}  // namespace smeagol
