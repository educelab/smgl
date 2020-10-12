#pragma once

/** @file */

#ifdef SMGL_USE_BOOSTFS
#include <boost/filesystem.hpp>
namespace smgl
{
/**
 * @namespace smgl::filesystem
 * @brief Alias for `boost::filesystem`
 *
 * If CMake variable `SMGL_USE_BOOSTFS` is false, then `smgl::filesystem` is an
 * alias for `std::filesystem`. Otherwise, it is an alias for
 * `boost::filesystem`. This enables filesystem compatibility for compilers
 * which do not provide `std::filesystem`.
 */
namespace filesystem = boost::filesystem;
}  // namespace smgl
#else
#include <filesystem>
namespace smgl
{
/**
 * @namespace smgl::filesystem
 * @brief Alias for `std::filesystem`
 *
 * If CMake variable `SMGL_USE_BOOSTFS` is false, then `smgl::filesystem` is an
 * alias for `std::filesystem`. Otherwise, it is an alias for
 * `boost::filesystem`. This enables filesystem compatibility for compilers
 * which do not provide `std::filesystem`.
 */
namespace filesystem = std::filesystem;
}  // namespace smgl
#endif