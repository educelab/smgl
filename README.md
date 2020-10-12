![smeagol](./graphics/svg/text-sub-color.svg)

**smeagol** is a header-only C++14 library for creating custom dataflow 
pipelines that are instrumented for serialization. It was designed to make it 
easy to convert existing processing workflows into **repeatable** and 
**observable** pipelines for the purposes of experimental reporting, 
reliability, and validation.

## Requirements
- CMake 3.15+
- C++14 compiler with Itanium C++ ABI support (clang, gcc, etc.)
- [JSON for Modern C++](https://github.com/nlohmann/json) 3.9.1+
- (optional) Boost.Filesystem 1.58+
    - This project will automatically check if the compiler provides
      `std::filesystem`. If it is not found, then
      [Boost.Filesystem](https://www.boost.org/) is required. This behavior can
      be controlled with the `SMGL_USE_BOOSTFS` CMake flag.

## Build and Installation
### CMake
As a header-only library, this project does not need to be built, but it is 
easiest to install it using the CMake build system:

```shell
mkdir build
cd build/
cmake ..
make && make install
```

This will install the headers to your default system include path, and provide 
an easy method to include the smeagol headers in your own CMake project:

```cmake
# Find smeagol libraries
find_package(smeagol REQUIRED)

# Link to an executable
add_executable(MyTarget main.cpp)
target_link_libraries(MyTarget smgl::smgl)
```

The CMake project provides a number of flags for configuring the build:
 - `SMGL_BUILD_JSON`: Use an in-source build of the JSON library during 
    compilation. (Default: ON)
 - `SMGL_USE_BOOSTFS`: Use the `Boost::filesystem` library instead of
    `std::filesystem`. (Default: ON if `std::filesystem` is not found)
 - `SMGL_BUILD_TESTS`: Build project unit tests. This will download and build 
    the Google Test framework. (Default: OFF) 
 - `SMGL_BUILD_DOCS`: Build documentation. Dependencies: Doxygen, Graphviz
    (optional). (Default: ON if Doxygen is found)

## Usage
### Building Custom Nodes
```c++
#include <smgl/Node.hpp>
#include <smgl/Ports.hpp>

class SumNode : public smgl::Node 
{
private:
    // Internal state variables
    int lhs_{0};
    int rhs_{0};
    int res_{0};
public:
    // InputPort receives values and places them in targets
    smgl::InputPort<int> lhs{&lhs_};
    smgl::InputPort<int> rhs{&rhs_};
    // OutputPort gets values from a source and posts them to connections
    smgl::OutputPort<int> result{&res_};
    
    SumNode() : Node() {
        // Register all ports for serialization
        registerInputPort("lhs", lhs);
        registerInputPort("rhs", rhs);
        registerOutputPort("result", result);
        
        // Define computation function
        compute = [=]() { result = lhs + rhs; };
    }
private:
    // Write internal state to Metadata
    smgl::Metadata serialize_(bool useCache, const smgl::filesystem::path& cacheDir) override {
        return {
            {"lhs", lhs_},
            {"rhs", rhs_},
            {"result", result_},
        };
    }
    
    // Load internal state from Metadata 
    void deserialize_(const smgl::Metadata& data, const smgl::filesystem::path& cacheDir) override {
        lhs_ = data["lhs"].get<T>();
        rhs_ = data["rhs"].get<T>();
        result_ = data["result"].get<T>();
    }
};
```

### Building Graphs
```c++
#include <memory> // std::make_shared
#include <smgl/Graph.hpp> // smgl::graph
#include "MyNodes.hpp" // SumNode, MultiplyNode

// 1 + 2
auto sumOp = std::make_shared<SumNode>();
sumOp->lhs(1);
sumOp->rhs(2);

// (1 + 2) * 3
auto multOp = std::make_shared<MultiplyNode>();
sumOp->result >> multOp->lhs; // OR smgl::connect(sumOp->result, multOp->lhs)
multOp->rhs(3);

// Build a graph and update
smgl::Graph g;
g.insertNodes(sumOp, multOp);
g.update();

std::cout << multOp->result() << std::endl; // 9

// Change input and update
sumOp->lhs(2);
g.update();

std::cout << multOp->result() << std::endl; // 12
```

### Serialization
```c++
// Build a graph
smgl::Graph g;

... // Do things with the graph

// Register nodes for serialization
// Use default name: "SumNode" -> SumNode
smgl::RegisterNode<SumNode>();
// Use custom name: "MyMultiplyNode" -> MultiplyNode
smgl::RegisterNode<MultiplyNode>("MyMultiplyNode");

// Write graph to file
smgl::Graph::Save("Graph.json", g);

// Load graph from file
auto gClone = smgl::Graph::Load("Graph.json");
```

## Related Projects
This library is not meant for everyone. It's primary purpose is to build 
workflows that are instrumented with metadata, but there are many other use 
cases for dataflow libraries. If smeagol doesn't fit your needs, perhaps on of
these libraries will:
- [RaftLib](https://github.com/RaftLib/RaftLib)
- [DSPatch](https://github.com/cross-platform/dspatch)
- [The Boost Graph Library](https://www.boost.org/doc/libs/1_74_0/libs/graph/doc/index.html)
- [ITK](https://itk.org/)