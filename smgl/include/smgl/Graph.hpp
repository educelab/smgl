#pragma once

/** @file */

#include <unordered_map>
#include <vector>

#include "smgl/Node.hpp"
#include "smgl/Uuid.hpp"
#include "smgl/filesystem.hpp"

namespace smgl
{

/**
 * @brief A collection of Nodes for managing pipeline state and serialization
 */
class Graph : public UniquelyIdentifiable
{
public:
    /** Graph update status */
    enum class Status { Idle, Updating, Error };

    /** @brief Get a Node by Uuid */
    Node::Pointer operator[](const Uuid& uuid) const;

    /** @brief Add a Node to the Graph */
    void insertNode(const Node::Pointer& n);

    /** @brief Add one or more Nodes to the Graph */
    template <typename N, typename... Ns>
    void insertNodes(N& n0, Ns&&... nodes);

    /** @brief Remove a Node from the Graph */
    void removeNode(const Node::Pointer& n);

    /** @brief Get the current update status */
    Status status() const;

    /**
     * @brief Get the root cache directory for the Graph
     *
     * If cacheEnabled() is true, Nodes are allowed to write data to a unique
     * subdirectory inside cacheDir. If cacheDir() is empty, the current working
     * directory will be used as the cache root.
     */
    filesystem::path cacheDir() const;

    /**
     * @brief Set the root cache directory for the Graph
     *
     * @copydetails cacheDir()
     */
    void setCacheDir(const filesystem::path& dir);

    /**
     * @brief Whether or not graph caching is enabled
     *
     * @copydetails cacheDir()
     */
    bool cacheEnabled() const;

    /**
     * @brief Set whether or not graph caching is enabled
     *
     * @copydetails cacheDir()
     */
    void setEnableCache(bool enable);

    /**
     * @brief Update the Graph's nodes
     *
     * Schedules the Nodes of the Graph and update them as needed.
     */
    Status update();

    /**
     * @brief Save a Graph to a JSON file
     *
     * @copydetails cacheDir()
     *
     * @param path Path to output file
     * @param g Graph to be saved
     */
    static void Save(const filesystem::path& path, const Graph& g);

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
    /** Cache directory */
    filesystem::path cacheDir_;
    /** Cache enabled status */
    bool cache_enabled_{false};
    /** List of Graph's nodes */
    std::unordered_map<Uuid, Node::Pointer> nodes_;
    /** Graph's update status */
    Status status_{Status::Idle};
};

}  // namespace smgl

#include "smgl/GraphImpl.hpp"