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

/** @brief Demangle std::type_info::name() */
std::string demangle(const char* name);

/** @brief Get the demangled name for type T */
template <class T>
std::string type_name();

/** @brief Get the demangled name for object t */
template <class T>
std::string type_name(const T& t);

/**
 * @brief Helper class for calling function iteratively on parameter pack
 *
 * Used until C++17, which includes parameter pack folding. See:
 * https://stackoverflow.com/a/17340003
 *
 * @code
 * template<typename... Ts>
 * void CallFooOnAll(Ts&&... ts) {
 *     ExpandType{0, (foo(std::forward<Ts>(ts)), 0)...}
 * }
 * @endcode
 */
struct ExpandType {
    /** Constructor */
    template <typename... T>
    explicit ExpandType(T&&...);
};

}  // namespace detail
}  // namespace smgl

#include "smgl/UtilitiesImpl.hpp"