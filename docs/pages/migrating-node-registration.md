# Migrating Node registration to source-token naming {#migrating-node-registration}

[TOC]

This guide is for downstream projects (e.g. `volume-cartographer`,
`registration-toolkit`) upgrading across the change that replaced RTTI
demangling with source-token node naming. The rationale, evidence, and
trade-offs live in the ADR
(`docs/adr/0001-source-token-node-naming.md`); this page is the mechanical
"what do I change in my code" companion.

## What changed

The auto-naming `RegisterNode<T>()` overload (which derived a node's
serialization key from `demangle(typeid(T).name())`) has been **removed**.
Names are now captured from the **source token** via the `SMGL_NODE()` macro and
registered through a variadic `smgl::RegisterNodes()`. The custom-name primitive
is unchanged.

| Before (removed / unchanged)            | After                                            |
| --------------------------------------- | ------------------------------------------------ |
| `RegisterNode<T>()`                     | `RegisterNodes(SMGL_NODE(T))`                    |
| `RegisterNode<A, B, C>()`               | `RegisterNodes(SMGL_NODE(A), SMGL_NODE(B), SMGL_NODE(C))` |
| `RegisterNode<T>("Name")` *(unchanged)* | `RegisterNode<T>("Name")`                        |
| `DeregisterNode<T>()` *(unchanged)*     | `DeregisterNode<T>()` or `DeregisterNodes(SMGL_NODE(T))` |
| `DeregisterNode("Name")` *(unchanged)*  | `DeregisterNode("Name")`                         |
| `detail::demangle()`, `detail::type_name()` | *removed — no replacement* |

`SMGL_NODE` is variadic, so types whose spelling contains top-level commas
(multi-parameter templates) pass as a single argument:

```{.cpp}
smgl::RegisterNodes(SMGL_NODE(my::ns::PairNode<int, double>));
```

## Step 1 — Replace auto-named registration

Wrap each type in `SMGL_NODE()` and pass it to `RegisterNodes()`. **Always
fully-qualify the type**, even under a `using namespace` directive: the spelling
becomes the node's serialization key and must be byte-identical at every
registration site and on every platform.

```{.cpp}
// Before
smgl::RegisterNode<rt::graph::ReadImageNode>();

// After
smgl::RegisterNodes(SMGL_NODE(rt::graph::ReadImageNode));
```

## Step 2 — Collapse batched registration

The old variadic overload becomes one `RegisterNodes()` call with one
`SMGL_NODE()` per type:

```{.cpp}
// Before
smgl::RegisterNode<
    rt::graph::ReadImageNode,
    rt::graph::WriteImageNode,
    rt::graph::ResampleNode<float>>();

// After
smgl::RegisterNodes(
    SMGL_NODE(rt::graph::ReadImageNode),
    SMGL_NODE(rt::graph::WriteImageNode),
    SMGL_NODE(rt::graph::ResampleNode<float>));
```

## Custom names and deregistration

If you registered under an explicit string, nothing changes — keep using the
low-level primitive:

```{.cpp}
smgl::RegisterNode<rt::graph::MultiplyNode>("MyMultiplyNode");  // unchanged
```

`DeregisterNode<T>()` (by type) and `DeregisterNode("Name")` (by string) are
both retained. A `DeregisterNodes(SMGL_NODE(...))` counterpart is provided so a
batch can be registered and deregistered with the same call sites.

## Removed utilities

`smgl::detail::demangle()` and `smgl::detail::type_name()` are gone. They were
never part of the serialization path (which uses type-identity lookup, not
stringification). If you called them directly for your own diagnostics, replace
them with the registered name or raw RTTI:

```{.cpp}
// Registered node -> source-token key; otherwise raw (mangled) RTTI name
std::string name = smgl::IsRegistered(node) ? smgl::NodeName(node)
                                            : typeid(*node).name();
```

## Build / CMake

smgl no longer probes for `cxxabi.h`, and the Itanium C++ ABI is no longer a
requirement (Windows/MSVC is now supported). There was never a public
`SMGL_USE_CXXABI` define to remove. If your build scripts copied smgl's old
`cxxabi.h` requirement, you can drop it.

## Serialization compatibility (read this if you have saved graphs)

The `"type"` field stored in every serialized graph is the registration key. The
new key is exactly the `#T` source spelling, so whether old files still load
depends on how that spelling compares to the demangled name they were written
with:

- **Non-templated, namespace-qualified types** (e.g.
  `rt::graph::ReadImageNode`) — the demangled name already equalled the source
  spelling, so `SMGL_NODE(rt::graph::ReadImageNode)` produces an **identical**
  key. Legacy files load unchanged.

- **Single-argument, non-`std` templates** (e.g. `ResampleNode<float>`) —
  likewise typically identical. Verify nested templates: older demanglers
  emitted `Foo<Bar<int> >` (note the space before `>`), whereas the source token
  is `Foo<Bar<int>>`. Open one of your JSON files and compare the literal
  `"type"` string if in doubt.

- **`std`-templated node types** (e.g. `Node<std::string>`) — the old key was
  **platform-specific** (`std::__1::…` on libc++, `std::__cxx11::…` on
  libstdc++, plus `>>`-vs-`> >` spacing) and was never portable across
  Linux/macOS/Windows. The new key (`Node<std::string>`) is stable but **does
  not match** the old one, so graphs serialized with such types must be migrated
  once.

### Migrating affected legacy files

A type can be registered under only **one** name (the factory rejects a second
registration of the same type), so you can't keep an old key and a new key live
simultaneously. Migrate by loading under the old key, then re-saving under the
new one — registration is global and mutable, so swap it between `Load` and
`Save`:

```{.cpp}
// 1. Register the affected type under its OLD, verbatim key to LOAD the file.
//    Copy the exact string out of the legacy JSON's "type" field.
smgl::RegisterNode<my::ns::Node<std::string>>(
    "MyNode<std::__1::basic_string<char, ...>>");
auto g = smgl::Graph::Load("legacy.json");

// 2. Swap to the source-token key, then re-save.
smgl::DeregisterNode<my::ns::Node<std::string>>();
smgl::RegisterNodes(SMGL_NODE(my::ns::Node<std::string>));
smgl::Graph::Save("migrated.json", g);
```

After this one-time pass, all files carry portable, source-faithful keys.

> Serializing or saving a graph whose node types are **not** registered now
> throws `smgl::unknown_identifier` (there is no source-token key to write).
> Register every node type before `Graph::Save`. (Graphviz is the one exception:
> `WriteDotFile` falls back to the Node's UUID as its label.)

## Checklist

- [ ] Replace every `RegisterNode<T>()` with `RegisterNodes(SMGL_NODE(T))`.
- [ ] Collapse variadic `RegisterNode<A, B, …>()` into a single
      `RegisterNodes(SMGL_NODE(A), …)`.
- [ ] Fully-qualify every `SMGL_NODE()` argument.
- [ ] Leave `RegisterNode<T>("Name")` / `DeregisterNode` calls as-is.
- [ ] Replace any direct `demangle()` / `type_name()` uses.
- [ ] Drop any copied `cxxabi.h` build requirement.
- [ ] Re-save graphs that used `std`-templated (or otherwise non-portable) keys.
