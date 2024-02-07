#pragma once

/** @file */

#include <cstdint>
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
    constexpr static std::uint32_t Version{1};

    /** Graph state */
    enum class State { Idle, Updating, Error };

    /** @brief Get a Node by Uuid */
    auto operator[](const Uuid& uuid) const -> Node::Pointer;

    /** @brief Add a Node to the Graph */
    void insertNode(const Node::Pointer& n);

    /** @brief Construct a Node and add it to the graph */
    template <typename NodeType, typename... Args>
    auto insertNode(Args... args) -> std::shared_ptr<NodeType>;

    /** @brief Add one or more Nodes to the Graph */
    template <typename N, typename... Ns>
    void insertNodes(N& n0, Ns&&... nodes);

    /** @brief Remove a Node from the Graph */
    void removeNode(const Node::Pointer& n);

    /** @brief Get the number of nodes in the graph */
    auto size() const -> std::size_t;

    /** @brief Get the current graph state */
    auto state() const -> State;

    /**
     * @brief Get the root cache file for the Graph
     *
     * If cacheEnabled() is true, Nodes are allowed to write data to a unique
     * subdirectory in cacheDir(). The exact location is relative to cacheFile()
     * and is determined by setCacheType().
     */
    auto cacheFile() const -> filesystem::path;

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
    auto cacheType() const -> CacheType;

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
    auto cacheDir() const -> filesystem::path;

    /**
     * @brief Whether or not graph caching is enabled
     *
     * @copydetails cacheFile()
     */
    auto cacheEnabled() const -> bool;

    /**
     * @brief Set whether or not graph caching is enabled
     *
     * @copydetails cacheFile()
     */
    void setEnableCache(bool enable);

    /** @brief Set the project metadata */
    void setProjectMetadata(const Metadata& m);

    /** @brief Get the project metadata */
    auto projectMetadata() const -> const Metadata&;

    /** @copydoc projectMetadata() const */
    auto projectMetadata() -> Metadata&;

    /**
     * @brief Update the Graph's nodes
     *
     * Schedules the Nodes of the Graph and update them as needed.
     */
    auto update() -> State;

    /**
     * @brief Serialize a Graph to a Metadata object
     *
     * @warning This function honors the value of cacheEnabled() and will write
     * data to cacheDir().
     */
    static auto Serialize(const Graph& g) -> Metadata;

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
    static auto Save(
        const filesystem::path& path, const Graph& g, bool writeCache = false)
        -> void;

    /**
     * @brief Load a Graph from a JSON file
     *
     * @note Loaded Graph does not share state with any preexisting Graph. See
     * <a href="https://code.cs.uky.edu/csparker247/smgl/-/issues/9">issue
     * #9</a>.
     *
     * @param path Path to input file in the JSON format
     */
    static auto Load(const filesystem::path& path) -> Graph;

    /**
     * @brief Checks that a Graph JSON file can be loaded
     *
     * Checks that every Node in the Graph is registered with the serialization
     * system. Returns the list of unregistered types as strings so that missing
     * registrations can be handled appropriately before calling Graph::Load.
     *
     * @param path Path to input file in the JSON format
     * @return List of Node types that are not registered for serialization
     */
    static auto CheckRegistration(const filesystem::path& path)
        -> std::vector<std::string>;

    /**
     * @brief Checks that a Graph can be serialized
     *
     * Checks that every Node in the Graph is registered with the serialization
     * system. Returns the list of unregistered types as strings so that missing
     * registrations can be handled appropriately before calling Graph::Load.
     * The returned type strings are the automatically generated names used when
     * calling RegisterNode() with no arguments.
     *
     * @param g Graph to be checked
     * @return List of Node types that are not registered for serialization
     */
    static auto CheckRegistration(const Graph& g) -> std::vector<std::string>;

    /**
     * @brief Generate an update schedule for the provided Graph
     *
     * Traverses the Graph and produces an ordered schedule for updating
     * Nodes. Graph must have at least one Node with no input connections and
     * must be acyclic.
     */
    static auto Schedule(const Graph& g) -> std::vector<Node::Pointer>;

private:
    /** Cache file */
    filesystem::path cacheFile_;
    /** Cache type */
    CacheType cacheType_{CacheType::Subdirectory};
    /** Cache enabled state */
    bool cache_enabled_{false};
    /** List of Graph's nodes */
    std::unordered_map<Uuid, Node::Pointer> nodes_;
    /** Graph state */
    State state_{State::Idle};
    /** Extra metadata */
    Metadata extraMetadata_;

    /** Perform graph serialization */
    static auto Serialize(
        const Graph& g, bool useCache, const filesystem::path& cacheDir)
        -> Metadata;

    /** Friend function: Graphviz WriteDotFile */
    friend auto WriteDotFile(
        const filesystem::path& path, const Graph& g, const GraphStyle& style)
        -> void;
};

}  // namespace smgl

#include "smgl/GraphImpl.hpp"