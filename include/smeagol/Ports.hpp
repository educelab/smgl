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

// Generic Output Port Interface
template <typename T>
class OutputPort : public Port
{
public:
    using Ptr = std::shared_ptr<OutputPort<T>>;
    virtual T val() = 0;

    Status status() { return status_; }
    void status(Status s) { status_ = s; }

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

    T val() { return val_(); }

private:
    std::function<T(Args...)> val_;
};

}  // namespace smeagol
