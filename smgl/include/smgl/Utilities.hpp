#pragma once

/** @file */

#include <string>

namespace smgl
{

/** @brief Wrap a member function w/ arguments into a lambda function */
template <class Obj, class ObjMemberFn, typename... Args>
auto WrapFunc(Obj* obj, ObjMemberFn&& fn, Args&&... args);

namespace detail
{

/**
 * @brief Helper class for calling function iteratively on parameter pack
 *
 * Used until C++17, which includes parameter pack folding. See:
 * https://stackoverflow.com/a/17340003
 *
 * ```{.cpp}
 * // C++17 built-in folding expression
 * template<typename... Ts>
 * void CallFooOnAll(Ts&&... ts) {
 *     (foo(std::forward<Ts>(ts)), ...);
 * }
 *
 * // C++11 "folding" using ExpandType
 * template<typename... Ts>
 * void CallFooOnAll(Ts&&... ts) {
 *     ExpandType{0, (foo(std::forward<Ts>(ts)), 0)...}
 * }
 * ```
 */
struct ExpandType {
    /** Constructor */
    template <typename... T>
    explicit ExpandType(T&&...);
};

}  // namespace detail
}  // namespace smgl

#include "smgl/UtilitiesImpl.hpp"