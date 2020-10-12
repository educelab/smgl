#pragma once

#include <fstream>
#include <typeinfo>

#include <gtest/gtest.h>

#include "smgl/Node.hpp"
#include "smgl/Ports.hpp"

namespace smgl
{
namespace test
{

template <typename T>
struct TrivialClass {
    void target(T i) { result = i; }
    T return_value_target(T i)
    {
        result = i;
        return result;
    }
    T source() const { return result; }
    T result{0};
};

int FreeFnSource() { return 1; }

template <typename T>
class AdditionNode : public Node
{
public:
    InputPort<T> lhs{&lhs_};
    InputPort<T> rhs{&rhs_};
    OutputPort<T> result{&result_};

    AdditionNode() : Node()
    {
        registerInputPort("lhs", lhs);
        registerInputPort("rhs", rhs);
        registerOutputPort("result", result);
        compute = [=]() { result_ = lhs_ + rhs_; };
    }

private:
    Metadata serialize_(
        bool useCache, const filesystem::path& cacheDir) override
    {
        return {
            {"lhs", lhs_},
            {"rhs", rhs_},
            {"result", result_},
        };
    }

    void deserialize_(
        const Metadata& data, const filesystem::path& cacheDir) override
    {
        lhs_ = data["lhs"].get<T>();
        rhs_ = data["rhs"].get<T>();
        result_ = data["result"].get<T>();
    }

    T lhs_{0};
    T rhs_{0};
    T result_{lhs_ + rhs_};
};

template <typename T>
class SubtractionNode : public Node
{
public:
    InputPort<T> lhs{&lhs_};
    InputPort<T> rhs{&rhs_};
    OutputPort<T> result{&result_};

    SubtractionNode() : Node()
    {
        registerInputPort("lhs", lhs);
        registerInputPort("rhs", rhs);
        registerOutputPort("result", result);
        compute = [=]() { result_ = lhs_ - rhs_; };
    }

private:
    Metadata serialize_(
        bool useCache, const filesystem::path& cacheDir) override
    {
        return {
            {"lhs", lhs_},
            {"rhs", rhs_},
            {"result", result_},
        };
    }

    void deserialize_(
        const Metadata& data, const filesystem::path& cacheDir) override
    {
        lhs_ = data["lhs"].get<T>();
        rhs_ = data["rhs"].get<T>();
        result_ = data["result"].get<T>();
    }

    T lhs_{0};
    T rhs_{0};
    T result_{lhs_ - rhs_};
};

template <typename T>
class MultiplyNode : public Node
{
public:
    InputPort<T> lhs{&lhs_};
    InputPort<T> rhs{&rhs_};
    OutputPort<T> result{&result_};

    MultiplyNode() : Node()
    {
        registerInputPort("lhs", lhs);
        registerInputPort("rhs", rhs);
        registerOutputPort("result", result);
        compute = [=]() { result_ = lhs_ * rhs_; };
    }

private:
    Metadata serialize_(
        bool useCache, const filesystem::path& cacheDir) override
    {
        return {
            {"lhs", lhs_},
            {"rhs", rhs_},
            {"result", result_},
        };
    }

    void deserialize_(
        const Metadata& data, const filesystem::path& cacheDir) override
    {
        lhs_ = data["lhs"].get<T>();
        rhs_ = data["rhs"].get<T>();
        result_ = data["result"].get<T>();
    }

    T lhs_{0};
    T rhs_{0};
    T result_{lhs_ * rhs_};
};

template <typename T>
class ClassWrapperNode : public Node
{
public:
    InputPort<T> set{[=](T i) { hidden_.target(i); }};
    OutputPort<T> get{[=]() { return hidden_.source(); }};

    ClassWrapperNode() : Node()
    {
        registerInputPort("set", set);
        registerOutputPort("get", get);
    }

private:
    Metadata serialize_(
        bool useCache, const filesystem::path& cacheDir) override
    {
        return {{"result", hidden_.result}};
    }

    void deserialize_(
        const Metadata& data, const filesystem::path& cacheDir) override
    {
        hidden_.result = data["result"].get<T>();
    }
    TrivialClass<T> hidden_;
};

template <class T>
class PassThroughNode : public Node
{
public:
    PassThroughNode() : Node()
    {
        registerInputPort("set", set);
        registerOutputPort("get", get);
    }
    explicit PassThroughNode(T val) : Node(), val_(val)
    {
        registerInputPort("set", set);
        registerOutputPort("get", get);
        set(val);
    }

    InputPort<T> set{&val_};
    OutputPort<T> get{&val_};

private:
    Metadata serialize_(
        bool useCache, const filesystem::path& cacheDir) override
    {
        return {{"value", val_}};
    }

    void deserialize_(
        const Metadata& data, const filesystem::path& cacheDir) override
    {
        val_ = data["value"].get<T>();
    }

    T val_;
};

class StringCachingNode : public Node
{
public:
    StringCachingNode() { registerInputPort("value", value); }

    InputPort<std::string> value{&value_};

private:
    Metadata serialize_(
        bool useCache, const filesystem::path& cacheDir) override
    {
        filesystem::path path = cacheDir / "value.txt";
        if (useCache) {
            std::ofstream fs(path.string());
            fs << value_;
            fs.close();
        }
        return {{"cacheFile", path.filename().string()}};
    }

    void deserialize_(
        const Metadata& data, const filesystem::path& cacheDir) override
    {
        filesystem::path path = cacheDir / "value.txt";
        std::ifstream fs(path.string());
        std::getline(fs, value_);
        fs.close();
    }

    std::string value_;
};

}  // namespace test
}  // namespace smgl