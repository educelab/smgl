# Building custom nodes {#building-custom-nodes}

[TOC]

Implementing a custom smgl::Node requires a few simple steps. In the following 
steps, we'll implement a custom node from scratch which calculates the sum of
two inputs.

### Inherit from smgl::Node

First, our custom node type should inherit from smgl::Node:

```{.cpp}
template<class T>
class SumNode : public smgl::Node {};
```

### Add input/output ports

Nodes in a graph communicate with each other via ports. When a node has been 
updated, all of its output ports signal their downstream connections that new 
data is available. When an input port receives new data, the node is queued 
for updates. To ensure proper serialization, all ports should be registered 
with the Node metadata system in the class constructor:

```{.cpp}
template<class T>
class SumNode : public smgl::Node
{
private:
    // Internal state variables
    T lhs_{0};
    T rhs_{0};
    T res_{0};
    
public:
    // InputPort receives values and places them in targets
    smgl::InputPort<T> lhs{&lhs_};
    smgl::InputPort<T> rhs{&rhs_};
    
    // OutputPort gets values from a source and posts them to connections
    smgl::OutputPort<T> result{&res_};

    SumNode() {
        // Register all ports for serialization
        registerInputPort("lhs", lhs);
        registerInputPort("rhs", rhs);
        registerOutputPort("result", result);
    }
};
```

### Implement a compute function

Every smgl::Node has a std::function member object named `compute` which is 
executed whenever the node is updated by the graph scheduler. To define your 
node's computation behavior, assign a lambda function or function reference to 
smgl::Node::compute, generally in the class constructor:

```{.cpp}
SumNode() {
    // Register all ports for serialization
    registerInputPort("lhs", lhs);
    registerInputPort("rhs", rhs);
    registerOutputPort("result", result);
    
    // Define computation function
    compute = [this]() { res_ = lhs_ + rhs_; };
}
```

### Enable state (de)serialization

To enable serialization and deserialization for your node, override the virtual
member functions smgl::Node::serialize_ and smgl::Node::deserialize_:

```{.cpp}
private:
    // Write internal state to Metadata
    smgl::Metadata serialize_(bool /* useCache */, const smgl::filesystem::path& /* cacheDir */) override {
        return {
            {"lhs", lhs_},
            {"rhs", rhs_},
            {"result", result_},
        };
    }
    
    // Load internal state from Metadata 
    void deserialize_(const smgl::Metadata& meta, const smgl::filesystem::path& /* cacheDir */) override {
        lhs_ = meta["lhs"].get<T>();
        rhs_ = meta["rhs"].get<T>();
        result_ = meta["result"].get<T>();
    }
```

The smgl::Metadata class is a dict-like type which provides easy serialization 
of many built-in types to the JSON format used by smeagol. For data which is 
not easily serialized to JSON, every smgl::Node can request that smeagol 
create a cache directory where arbitrary data can be saved and loaded during 
(de)serialization. 

There are two methods for indicating that your custom node will need a cache 
directory. If your class will always write to the cache directory during 
serialization, call the smgl::Node::Node(bool) constructor when defining your 
constructor implementation:

```{.cpp}
// Call special constructor
Foo() : Node{true} {}

// Write data when caching is requested
Foo::serialize_(bool useCache, const smgl::filesystem::path& cacheDir) {
    smgl::Metadata meta;
    if(useCache) {
        WriteData(cacheDir / "data.bin");
        meta["data"] = "data.bin";
    }
}
```

This sets smgl::Node::usesCacheDir to return the value passed to the Node 
constructor. If your class conditionally uses the cache directory based on 
program state, assign an appropriate test function to smgl::Node::usesCacheDir:

```{.cpp}
struct Foo {
    std::unique_ptr<ComplexType> bar;
    Foo() {
        usesCacheDir = [&bar = bar](){ return bar != nullptr;};
    }
}
```

If smgl::Node::usesCacheDir returns `true`, the `cacheDir` directory passed 
to smgl::Node::serialize_ is guaranteed to exist. Add custom serialization code 
to your serialization functions to write/read data to/from this location:

```{.cpp}
class Foo : public smgl::Node
{
public:
    // bar_ is guaranteed to never be nullptr
    Foo() : Node{true}, bar_{std::make_unique<ComplexType>()} {}
    
    /* Alternative: Redefine usesCacheDir if bar_ might by nullptr
    Foo() {
        usesCacheDir = [&bar = bar](){ return bar != nullptr;};
    }
    */
    
private:
    // Some complex type
    std::unique_ptr<ComplexType> bar_;
    
    // Serialize the node
    smgl::Metadata serialize_(bool useCache, const smgl::filesystem::path& cacheDir) override {
        // Setup metadata
        smgl::Metadata meta;
        
        // Only write to the cache directory when requested
        if(useCache) {
        
            // Write the complex type to the cache directory
            WriteComplex(cacheDir / "foo.bin", foo_);
            
            // Add the file name to the serialized metadata
            meta["foo"] = "foo.bin";
        }
        return meta;
    }
    
    // Deserialize the node
    void deserialize_(const smgl::Metadata& meta, const smgl::filesystem::path& cacheDir) override {
        // Check if we serialized the file name
        if(meta.contains("foo")) {
            
            // Get the cached file's name
            file_name = meta["foo"].get<std::string>();
            
            // Restore the complex type from the cache
            foo_ = LoadComplex(cacheDir / file_name);
        }
    }
};
```

### Complete code
Below is a full implementation of a templated SumNode:

```{.cpp}
#include <smgl/Node.hpp>
#include <smgl/Ports.hpp>

template<class T>
class SumNode : public smgl::Node 
{
private:
    // Internal state variables
    T lhs_{0};
    T rhs_{0};
    T res_{0};
public:
    // InputPort receives values and places them in targets
    smgl::InputPort<T> lhs{&lhs_};
    smgl::InputPort<T> rhs{&rhs_};
    
    // OutputPort gets values from a source and posts them to connections
    smgl::OutputPort<T> result{&res_};
    
    SumNode() {
        // Register all ports for serialization
        registerInputPort("lhs", lhs);
        registerInputPort("rhs", rhs);
        registerOutputPort("result", result);
        
        // Define computation function
        compute = [this]() { res_ = lhs_ + rhs_; };
    }
private:
    // Write internal state to Metadata
    smgl::Metadata serialize_(bool /* useCache */, const smgl::filesystem::path& /* cacheDir */) override {
        return {
            {"lhs", lhs_},
            {"rhs", rhs_},
            {"result", result_},
        };
    }
    
    // Load internal state from Metadata 
    void deserialize_(const smgl::Metadata& meta, const smgl::filesystem::path& /* cacheDir */) override {
        lhs_ = meta["lhs"].get<T>();
        rhs_ = meta["rhs"].get<T>();
        result_ = meta["result"].get<T>();
    }
};
```

