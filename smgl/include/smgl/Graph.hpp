#pragma once

/** @file */

#include <memory>
#include <unordered_map>
#include <vector>

#include "smgl/Node.hpp"
#include "smgl/Uuid.hpp"
#include "smgl/filesystem.hpp"

namespace smgl
{

// Pre-declare for Graphviz
class GraphStyle;

/** @brief Cache Location Type */
enum class CacheType {
    /** Cache directory is the cache file's parent directory */
    Adjacent = 0,
    /** Cache directory is a subdirectory derived from the cache filename */
    Subdirectory
};

/**
 * @brief A collection of Nodes for managing pipeline state and serialization
 */
class Graph : public UniquelyIdentifiable
{
public:
    /** Graph version */
    constexpr static uint32_t Version{1};

    /** Graph update status */
    enum class Status { Idle, Updating, Error };

    /** @brief Get a Node by Uuid */
    Node::Pointer operator[](const Uuid& uuid) const;

    /** @brief Add a Node to the Graph */
    void insertNode(const Node::Pointer& n);

    /** @brief Construct a Node and add it to the graph */
    template <typename NodeType, typename... Args>
    std::shared_ptr<NodeType> insertNode(Args... args);

    /** @brief Add one or more Nodes to the Graph */
    template <typename N, typename... Ns>
    void insertNodes(N& n0, Ns&&... nodes);

    /** @brief Remove a Node from the Graph */
    void removeNode(const Node::Pointer& n);

    /** @brief Get the current update status */
    Status status() const;

    /**
     * @brief Get the root cache file for the Graph
     *
     * If cacheEnabled() is true, Nodes are allowed to write data to a unique
     * subdirectory in cacheDir(). The exact location is relative to cacheFile()
     * and is determined by setCacheType().
     */
    filesystem::path cacheFile() const;

    /**
     * @brief Set the root cache file for the Graph
     *
     * @copydetails cacheFile()
     */
    void setCacheFile(const filesystem::path& p);

    /**
     * @brief Get the graph's CacheType
     *
     * A graph's CacheType determines the location of the cache relative to
     * cacheFile().
     */
    CacheType cacheType() const;

    /**
     * @brief Set the graph's CacheType
     *
     * @copydetails cacheType()
     */
    void setCacheType(CacheType t);

    /**
     * @brief Returns the cache directory as configured by cacheFile() and
     * cacheType()
     */
    filesystem::path cacheDir() const;

    /**
     * @brief Whether or not graph caching is enabled
     *
     * @copydetails cacheFile()
     */
    bool cacheEnabled() const;

    /**
     * @brief Set whether or not graph caching is enabled
     *
     * @copydetails cacheFile()
     */
    void setEnableCache(bool enable);

    /**
     * @brief Update the Graph's nodes
     *
     * Schedules the Nodes of the Graph and update them as needed.
     */
    Status update();

    /**
     * @brief Serialize a Graph to a Metadata object
     *
     * @warning This function honors the value of cacheEnabled() and will write
     * data to cacheDir().
     */
    static Metadata Serialize(const Graph& g);

    /**
     * @brief Save a Graph to a JSON file
     *
     * If writeCache is true, cache information will be written adjacent to
     * the provided path honoring the cacheType() configuration.
     *
     * @param path Path to output file
     * @param g Graph to be saved
     * @param writeCache Whether or not to write cache files
     */
    static void Save(
        const filesystem::path& path, const Graph& g, bool writeCache = false);

    /**
     * @brief Load a Graph from a JSON file
     *
     * @note Loaded Graph does not share state with any preexisting Graph. See
     * <a href="https://code.cs.uky.edu/csparker247/smeagol/-/issues/9">issue
     * #9</a>.
     *
     * @param path Path to input file in the JSON format
     */
    static Graph Load(const filesystem::path& path);

    /**
     * @brief Generate an update schedule for the provided Graph
     *
     * Traverses the Graph and produces an ordered schedule for updating
     * Nodes. Graph must have at least one Node with no input connections and
     * must be acyclic.
     */
    static std::vector<Node::Pointer> Schedule(const Graph& g);

private:
    /** Cache file */
    filesystem::path cacheFile_;
    /** Cache type */
    CacheType cacheType_{CacheType::Subdirectory};
    /** Cache enabled status */
    bool cache_enabled_{false};
    /** List of Graph's nodes */
    std::unordered_map<Uuid, Node::Pointer> nodes_;
    /** Graph's update status */
    Status status_{Status::Idle};

    /** Perform graph serialization */
    static Metadata Serialize(
        const Graph& g, bool useCache, const filesystem::path& cacheDir);

    /** Friend function: Graphviz WriteDotFile */
    friend void WriteDotFile(
        const filesystem::path& path, const Graph& g, const GraphStyle& style);
};

}  // namespace smgl

#include "smgl/GraphImpl.hpp"