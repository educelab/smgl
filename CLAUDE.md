# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Overview

**smgl** ("Structured Metadata Engine and Graph object Library") is a
header-and-source C++14 library for
building **dataflow pipelines that are instrumented for serialization**. The
goal is to turn an existing processing workflow into a repeatable, observable,
and serializable graph of computation `Node`s connected through typed `Port`s.

The library is consumed via CMake as `smgl::smgl` and exposes a single
convenience header `<smgl/smgl.hpp>`.

## Build, Test, and Format

```shell
# Configure + build the library (tests/docs off by default)
cmake -S . -B build -GNinja
cmake --build build

# Configure with tests (downloads GoogleTest v1.17.0 via FetchContent)
cmake -S . -B build -GNinja -DSMGL_BUILD_TESTS=ON
cmake --build build
ctest --test-dir build -V

# Run a single test binary directly (one executable per Test*.cpp file,
# named smgl_<filename>, output to build/bin/)
./build/bin/smgl_TestGraph
ctest --test-dir build -R smgl_TestGraph        # by ctest name

# Build documentation (requires Doxygen; Graphviz optional)
cmake -S . -B build -DSMGL_BUILD_DOCS=ON
cmake --build build --target docs
```

Key CMake flags: `SMGL_BUILD_TESTS` (off), `SMGL_BUILD_DOCS` (on if Doxygen
found), `SMGL_BUILD_JSON` (on; in-source nlohmann/json build),
`SMGL_USE_BOOSTFS` (on only if `std::filesystem` is unavailable — selects
Boost.Filesystem and defines the `SMGL_USE_BOOSTFS` macro).

Code style is enforced by `clang-format` (`.clang-format` at root); CI rejects
unformatted code. Format staged changes with `git clang-format`, or a whole
branch with `git clang-format main`. Note headers use trailing-return-type
declarations (`auto f() -> T`) as the house style.

## Architecture

The dependency order of the core abstractions (low-level → high-level):
`Uuid` → `Metadata` → `Ports` → `Node` → `Graph`, with `Factory`/`Singleton`
supporting serialization and `Graphviz`/`Logging` as add-ons.

- **Ports** (`Ports.hpp`) — Typed connection endpoints. `InputPort<T>` wraps a
  pointer to a member variable; `OutputPort<T>` broadcasts `Update<T>` values to
  connected inputs. `connect()` / `disconnect()` (and the `>>` / `<<` operators)
  join an `Output` to an `Input`, throwing `bad_connection` on type mismatch.
  Ports communicate via queued, tick-stamped `Update<T>` messages — this is how
  the graph knows what is stale.

- **Node** (`Node.hpp`) — Abstract base (`UniquelyIdentifiable`). Subclasses add
  public `InputPort`/`OutputPort` members, call `registerInputPort`/
  `registerOutputPort` in their constructor (failing to register ports causes
  silent scheduling bugs), and assign the `compute` `std::function<void()>`.
  Each Node tracks a `State` (Idle/Waiting/Ready/Updating/Error). To participate
  in serialization, override the private virtuals `serialize_` /`deserialize_`;
  if they need a scratch directory, construct via `Node(bool usesCacheDir)` or
  reassign the `usesCacheDir` function.

- **Graph** (`Graph.hpp`) — Owns nodes (`insertNode<T>(...)`, `removeNode`).
  `update()` topologically schedules and updates only stale nodes (see the
  scheduling helpers near the bottom of `Graph.hpp`). Supports two serialization
  modes: **explicit** (`Graph::Save` / `Graph::Load`) and **automatic caching**
  (`setEnableCache(true)` + `setCacheFile` + `update()`), with cache layout
  controlled by `CacheType` (`Adjacent` vs `Subdirectory`).

- **Factory + Singleton** (`Factory.hpp`, `Singleton.hpp`) — Serialization
  requires node types to be registered in a **global** factory singleton via
  `RegisterNodes(SMGL_NODE(T), ...)` (key captured from the source token `#T`)
  or the lower-level `RegisterNode<T>("Name")`. The `SMGL_NODE` key must be
  fully qualified and is byte-identical across compilers/platforms — RTTI
  demangling was removed (see `docs/adr/0001-source-token-node-naming.md`).
  `CreateNode(name)` reconstructs instances during `Graph::Load`. Registration
  is process-global and only needs to happen once; unregistered types fall back
  to their UUID and won't round-trip cleanly.

- **Metadata** (`Metadata.hpp`) — Alias over nlohmann/json used as the on-disk
  representation for node state and graph structure.

- **Graphviz** (`Graphviz.hpp`) — `WriteDotFile()` emits a `.dot` graph (uses
  registered node names when available); `GraphStyle` customizes rendering.

### Header / Impl convention

Templated and inline definitions live in a sibling `*Impl.hpp` file that is
`#include`d at the **bottom** of its public header (e.g. `Node.hpp` ends with
`#include "smgl/NodeImpl.hpp"`). When changing a templated declaration, edit the
declaration in `X.hpp` and the definition in `XImpl.hpp`. Non-template code is
compiled from `smgl/src/*.cpp`. All public headers are listed explicitly in
`smgl/CMakeLists.txt` and installed — add new headers there.

## Layout

- `smgl/include/smgl/` — public headers (+ `*Impl.hpp` template defs)
- `smgl/src/` — non-template compiled sources
- `tests/src/Test*.cpp` — GoogleTest/GoogleMock unit tests (one binary each);
  `tests/include/smgl/TestLib.hpp` holds shared test node definitions
- `cmake/` — `FindDependencies`, `InstallProject` modules
- `docs/` — Doxygen config and tutorials

## Contributing notes

Development happens on GitHub (`github.com/educelab/smgl`) via Pull Requests;
open work-in-progress PRs as **draft** PRs. Branch names addressing an issue
should start with the issue number (e.g. `9-fixes-a-bug`). The project keeps a
citable Zenodo record (`.zenodo.json`).
