#pragma once

#include <functional>
#include <memory>

#include "smeagol/Status.hpp"

namespace smeagol
{

// Ports Base Class
class Port
{
};

template <typename T, typename Ret = void>
class InputPort : public Port
{
public:
    explicit InputPort(T* target)
    {
        target_ = [target](T v) { *target = v; };
    }
    explicit InputPort(std::function<Ret(T)> target) { target_ = target; }

    void source(std::function<T()> src) { source_ = src; }
    void notify(T val) { target_(val); }
    void update() { target_(source_()); }

private:
    std::function<T()> source_;
    std::function<Ret(T)> target_;
};

// Generic Output Port Interface
template <typename T, typename... Args>
class OutputPort : public Port
{
public:
    explicit OutputPort(T val) : val_{[val]() { return val; }} {}
    explicit OutputPort(T* val) : val_{[val]() { return *val; }} {}
    explicit OutputPort(std::function<T(Args...)> val) : val_{val} {}

    T val() { return val_(); }

    Status status() { return status_; }
    void status(Status s) { status_ = s; }

    template <typename Ret>
    void connect(InputPort<T, Ret>& p)
    {
        p.source([this]() { return this->val(); });
    }

private:
    Status status_{Status::Ready};
    std::function<T(Args...)> val_;
};

}  // namespace smeagol
