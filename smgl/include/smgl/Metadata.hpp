#pragma once

/** @file */

#include <nlohmann/json.hpp>

#include "smgl/filesystem.hpp"

namespace smgl
{
/** @brief Metadata storage class */
using Metadata = nlohmann::ordered_json;

/** @brief Write Metadata to path in JSON format */
void WriteMetadata(const filesystem::path& path, const Metadata& m);

/** @brief Load Metadata from JSON file at path */
Metadata LoadMetadata(const filesystem::path& path);

}  // namespace smgl
