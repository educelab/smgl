#pragma once

/** @file */

#include <memory>
#include <type_traits>

namespace smgl
{
namespace type_traits
{

/**
 * @brief Removes const-qualifications and references from type
 *
 * Implementation of std::remove_cvref which is not available until C++20.
 */
template <class T>
struct remove_cvref {
    /** Type T with cv-qualifications and references removed */
    using type = std::remove_cv_t<std::remove_reference_t<T>>;
};

/** @copydoc remove_cvref */
template <typename T>
using remove_cvref_t = typename remove_cvref<T>::type;

/**
 * Type templated by list of bools
 * See: https://stackoverflow.com/a/28253503
 */
template <bool...>
struct bool_pack;

/**
 * @brief Checks if all template conditions are true
 *
 * See: https://stackoverflow.com/a/28253503
 */
template <bool... v>
using all_true = std::is_same<bool_pack<true, v...>, bool_pack<v..., true>>;

/** @copydoc all_true */
template <bool... v>
constexpr bool all_true_v = all_true<v...>::value;

/**
 * Checks if Ptr element type can be converted to Base. Assumes templated types
 * are not cv-qualified.
 */
template <typename Ptr, typename Base>
struct is_ptr_to_base_helper
    : std::integral_constant<
          bool,
          std::is_convertible<
              std::add_pointer_t<
                  typename std::pointer_traits<Ptr>::element_type>,
              std::add_pointer_t<Base>>::value> {
};

/** @brief Checks if Ptr element type can be converted to Base */
template <typename Ptr, typename Base>
struct is_ptr_to_base
    : is_ptr_to_base_helper<remove_cvref_t<Ptr>, remove_cvref_t<Base>> {
};

/** @copydoc is_ptr_to_base */
template <typename Ptr, typename Base>
constexpr bool is_ptr_to_base_v = is_ptr_to_base<Ptr, Base>::value;

}  // namespace type_traits
}  // namespace smgl