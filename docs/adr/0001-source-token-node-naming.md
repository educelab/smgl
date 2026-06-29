# ADR 0001: Source-token node naming (replacing RTTI demangling)

- Status: Accepted
- Date: 2026-06-29
- Supersedes: PR #14 ("Cross-platform demangling", branch `win-demangle`)

## Context

`smgl` exists to produce **automatically serializable computational graphs for
scientific reproducibility**. A graph is serialized as a set of nodes, each
stored under a `"type"` identifier that the `Factory` uses to reconstruct the
node on load (`Graph::Load` → `CreateNode(meta["type"])`).

Until now, node types could be registered with an auto-generated name derived
from RTTI:

```cpp
RegisterNode<T>();  // name = demangle(typeid(T).name())
```

`demangle()` wrapped `abi::__cxa_demangle` (the Itanium C++ ABI demangler). This
had two consequences:

1. **It does not build where `cxxabi.h` is unavailable** (notably MSVC/Windows),
   which is what PR #14 originally set out to fix by stubbing `demangle()`.
2. **The auto-generated name is neither source-faithful nor reproducible across
   toolchains.** This is the deeper problem, and it makes the auto-naming path
   unfit for a reproducibility library.

### Evidence: RTTI demangling is not reproducible

`typeid` erases typedefs *before* a name is ever produced — `typeid(std::string)`
is literally `typeid(std::basic_string<...>)`. The demangler can only reconstruct
the canonical type, and the canonical spelling **differs across standard library
implementations**. Same program, same C++ standard:

| Type | libc++ (Clang/macOS) | libstdc++ (GCC 13) |
|---|---|---|
| `std::string` | `std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char>>` | `std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >` |
| `std::vector<int>` | `std::__1::vector<int, std::__1::allocator<int>>` | `std::vector<int, std::allocator<int> >` |
| `PassThroughNode<int>` | `smgl::test::PassThroughNode<int>` | `smgl::test::PassThroughNode<int>` |

The outputs diverge on three independent axes — inline-namespace token
(`__1` vs `__cxx11`), where it is injected, and closing-bracket spacing
(`>>` vs `> >`) — *before MSVC is even considered*. Consequently, any node
registered with a `std`-templated type already produces **different
serialization keys on Linux vs macOS today**. Windows was only the first crack
to become visible.

The single reproducible region is exactly where the name contains no typedef and
no standard-library type (e.g. `PassThroughNode<int>`). That is not a contract we
can offer through demangling.

### Why no language standard fixes this

The source spelling (`std::string`, not `std::basic_string<...>`) exists only at
the **preprocessor** level. Templates see the type *after* typedef resolution, so
they cannot recover it. Even static reflection (C++26) yields the canonical type
and inherits the same erasure. Upgrading the standard does not help; the
preprocessor `#` operator is the only mechanism that captures the as-written
spelling.

## Decision

Capture node names from the **source token**, not from RTTI.

1. Introduce a descriptor + macro:

   ```cpp
   namespace smgl {
   template <class T>
   struct NodeDesc {
       explicit NodeDesc(std::string key) : key{std::move(key)} {}
       std::string key;
   };

   template <class... Ts>
   auto RegisterNodes(const NodeDesc<Ts>&... descs) -> bool;  // C++14 fold
   }  // namespace smgl

   #define SMGL_NODE(T) ::smgl::NodeDesc<T>(#T)
   ```

   Usage preserves the batched style consumers already rely on:

   ```cpp
   smgl::RegisterNodes(
       SMGL_NODE(rt::graph::ReadImageNode),
       SMGL_NODE(rt::graph::WriteImageNode));
   ```

2. **The macro is the sole auto-naming path.** Remove the RTTI-auto-named
   `RegisterNode<T>()` overload (and the C++14 `detail::ExpandType` fold
   emulation it used). Keep `RegisterNode<T>("name")` as the primitive that
   `RegisterNodes` folds onto. Add a parallel `SMGL_NODE`-based `DeregisterNode`.

3. **Remove `demangle()` and `type_name()` entirely.** Serialization never used
   them: `meta["type"]` comes from `Factory::GetTypeIdentifier(typeid(node))`,
   a `std::type_info`-*identity* lookup (no stringification, process-local,
   platform-safe). The only remaining callers are three diagnostic sites in
   `Graph.cpp`, rerouted as:
   - registered nodes → `NodeName()` (the registered key);
   - unregistered nodes → raw `typeid().name()`.

   Delete the `check_include_file_cxx(cxxabi.h)` CMake probe and the
   `SMGL_USE_CXXABI` compile definition. Nothing in `smgl` demangles anymore, so
   there is no Windows port to maintain — the original `win-demangle` problem is
   dissolved rather than patched.

4. **Keys are verbatim `#T`, fully qualified.** Authors must qualify the type
   inside `SMGL_NODE` even under a `using namespace` directive. A node's
   registered spelling is part of its serialization identity. Whitespace
   canonicalization is intentionally deferred (see Consequences).

5. **Target C++14.** A bump to C++17 is a separate, optional cleanup (the
   `std::filesystem` path already requires 17 transitively, and it would remove
   the `ExpandType` emulation). C++20 is explicitly avoided — its only benefit
   here would be a variadic `FOR_EACH` macro, an approach this ADR rejects, and
   it would raise the MSVC floor on the very platform we are enabling.

## Consequences

### Positive
- Node keys are source-faithful (`std::string` stays `std::string`) and
  byte-identical across Linux, macOS, and Windows.
- The cxxabi dependency and the Windows build blocker are removed outright.
- One naming path → one key format. No silent, toolchain-dependent divergence.

### Negative / accepted trade-offs
- **Breaking API change.** `RegisterNode<T>()` (no name) is removed; consumers
  migrate to `RegisterNodes(SMGL_NODE(...))`. Acceptable pre-v1.
- **Author discipline required.** The key reflects how the type is spelled at the
  `SMGL_NODE` site; inconsistent spelling/spacing across sites would fork
  identity. Mitigated by documentation; a normalizer is deferred until a
  templated node actually needs spacing-insensitivity.
- **Diagnostics for *unregistered* types** (`CheckRegistration(const Graph&)` and
  two debug logs) report raw `typeid().name()` — mangled on GCC/Clang, readable
  on MSVC. This is an error/setup path; registered types still report their clean
  key.

### Out of scope
- Normalizing MSVC names to match cxxabi (proven unwinnable — there is no single
  canonical demangled spelling).
- Downstream migration of `volume-cartographer` and `registration-toolkit`
  (handled separately).

## Implementation plan

1. **Naming layer** (`Node.hpp`/`NodeImpl.hpp`, `Factory` as needed)
   - Add `smgl::NodeDesc<T>` and `RegisterNodes(NodeDesc<Ts>...)` (C++14
     `std::initializer_list` fold over the existing `RegisterNode<T>(name)`).
   - Add the `SMGL_NODE(T)` macro (public header; document fully-qualified usage).
   - Add `SMGL_NODE`-based `DeregisterNode` symmetry.
   - Remove the RTTI-auto-named `RegisterNode<T>()` overload and the
     `detail::ExpandType` fold emulation.
2. **Remove demangling**
   - Delete `demangle()` / `type_name()` from `Utilities.hpp`/`UtilitiesImpl.hpp`
     and `Utilities.cpp`.
   - Reroute `Graph.cpp:112`, `:317`, `:320` (registered → `NodeName()`,
     unregistered → raw `typeid().name()`).
   - Remove `check_include_file_cxx(cxxabi.h)` from `cmake/FindDependencies.cmake`
     and the `SMGL_USE_CXXABI` definition from `smgl/CMakeLists.txt`.
3. **Tests** (`tests/src/TestNode.cpp`, `TestGraphviz.cpp`)
   - Migrate registrations to `RegisterNodes(SMGL_NODE(...))`. Existing exact-name
     assertions already use fully-qualified strings, so expected keys are
     unchanged.
   - Add a test asserting a `std`-templated node key is source-faithful and equal
     across platforms (the case that motivated this ADR).
4. **Docs**
   - Update `CLAUDE.md` ("auto-named from demangled RTTI" is no longer true).
   - Update the registration tutorial under `docs/pages/`.
</content>
</invoke>
