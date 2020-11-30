#pragma once

/** @file */

#include "smgl/Graph.hpp"
#include "smgl/filesystem.hpp"

namespace smgl
{

/** @brief Write Graph to a file in the Graphviz Dot format */
void WriteDotFile(const filesystem::path& path, const Graph& g);

}
