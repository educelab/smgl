#pragma once

/** @file */

#include <cstdint>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include "smgl/Graph.hpp"
#include "smgl/Utilities.hpp"
#include "smgl/filesystem.hpp"

namespace smgl
{

/** @brief Style struct for ports and labels */
struct ElementStyle {
    /** @brief Text alignment */
    std::string align;
    /** @brief Element background color */
    std::string bgcolor;
    /** @brief Element border thickness [0, 255] */
    std::uint8_t border{1};
    /** @brief Element border color */
    std::string color;
};

/** @brief Style struct for a node's base style */
struct BaseStyle : public ElementStyle {
    /** @brief Sub-element border thickness [0, 255] */
    std::uint8_t cellborder{1};
    /** @brief Sub-element border padding [0, 255] */
    std::uint8_t cellpadding{2};
    /** @brief Sub-element border spacing [0, 255] */
    std::uint8_t cellspacing{2};
};

/** @brief Style struct for a node's overall font style */
struct FontStyle {
    /** @brief Font color */
    std::string color;
    /** @brief Font typeface */
    std::string face;
    /** @brief Returns whether or not this style has an values set */
    auto empty() const -> bool { return color.empty() and face.empty(); }
};

/** @brief Style struct containing all sub-styles for a node */
struct NodeStyle {
    /** @brief Base style for the node */
    BaseStyle base;
    /** @brief Base font style for this node */
    FontStyle font;
    /** @brief InputPort-specific style */
    ElementStyle inputPorts;
    /** @brief Label-specific style */
    ElementStyle label;
    /** @brief OutputPort-specific style */
    ElementStyle outputPorts;
};

/**
 * @brief Class defining a Graph's style when writing to a Dot file
 *
 * This class enables global, class, and instance level styling of Nodes in a
 * Dot file by setting a default style and, optionally, a class-specific or
 * instance-specific style. When determining the final style, class-specific
 * properties take precedence over default properties, and instance-specific
 * properties take precedence over class-specific properties.
 *
 * ```{.cpp}
 * // Setup a graph
 * smgl::Graph g;
 * auto a = g.insertNode<IntNode>();
 * auto b = g.insertNode<IntNode>();
 * auto c = g.insertNode<FloatNode>();
 *
 * // Global style
 * GraphStyle style;
 * style.defaultStyle().base.bgcolor = "red";
 *
 * // Class-specific style
 * style.classStyle<IntNode>().base.bgcolor = "white";
 *
 * // Instance-specific style
 * style.instanceStyle(a).base.bgcolor = "blue";
 * style.instanceStyle(a).font.color = "white";
 *
 * // Write the graph
 * WriteDotFile("StyledGraph.gv", g, style);
 * ```
 * @image html graphviz-styled.png
 *
 * @warning Neither this class nor smgl::WriteDotFile makes any effort to
 * validate the values set in style properties. For more information on valid
 * property values, see the Doxygen documentation for
 * [HTML-like labels](https://graphviz.org/doc/info/shapes.html#html).
 */
class GraphStyle
{
public:
    /** @brief Default constructor */
    GraphStyle();

    /**
     * @name Default Node Style
     * The properties defined in the default node style are applied to all
     * nodes.
     */
    /** @{ */
    /** @brief Set the default node style */
    void setDefaultStyle(const NodeStyle& style);
    /** @brief Access the default node style */
    auto defaultStyle() -> NodeStyle&;
    /** @copydoc defaultStyle() */
    auto defaultStyle() const -> const NodeStyle&;
    /** @} */

    /**
     * @name Class-specific Node Styles
     * The properties defined in a class-specific node style are applied to all
     * nodes of a specific class and take precedence over properties defined in
     * the default node style.
     */
    /** @{ */
    /**
     * @brief Set a class-specific node style
     *
     * Sets the style for all nodes of a particular class. Nodes of a particular
     * class are identified using the serialization name register. If unknown,
     * the node's class name in the register can be determined using
     * smgl::NodeName.
     */
    void setClassStyle(const std::string& name, const NodeStyle& style);
    /** @copydoc setClassStyle() */
    template <class NodeT>
    void setClassStyle(const NodeStyle& style);
    /** @brief Check if a class of nodes has an associated style */
    auto hasClassStyle(const std::string& name) const -> bool;
    /** @copydoc hasClassStyle() */
    template <class NodeT>
    auto hasClassStyle() const -> bool;
    /** @brief Erase the style associated with a class of nodes */
    void eraseClassStyle(const std::string& name);
    /** @copydoc eraseClassStyle() */
    template <class NodeT>
    void eraseClassStyle() const;
    /**
     * @brief Access the style for a class of nodes
     *
     * If a style has not been set, one is created and returned.
     */
    auto classStyle(const std::string& name) -> NodeStyle&;
    /** @copydoc classStyle(const std::string&) */
    template <class NodeT>
    auto classStyle() -> NodeStyle&;
    /**
     * @copybrief classStyle(const std::string&)
     *
     * @throws std::out_of_range if a style has not been set
     */
    auto classStyle(const std::string& name) const -> const NodeStyle&;
    /** @copydoc classStyle(const std::string&) const */
    template <class NodeT>
    auto classStyle() const -> const NodeStyle&;
    /** @} */

    /**
     * @name Instance-specific Node Styles
     * The properties defined in an instance-specific node style are only
     * applied to a specific node instance as determined by its smgl::Uuid.
     * These properties take precedence over the properties defined in the
     * default and class-specific node styles.
     */
    /** @{ */
    /**
     * @brief Set a instance-specific node style
     *
     * Sets the style for a single node instance. A node instance is identified
     * using its smgl::Uuid.
     */
    void setInstanceStyle(const Node::Pointer& node, const NodeStyle& style);
    /** @copydoc setInstanceStyle() */
    void setInstanceStyle(const Node* node, const NodeStyle& style);
    /** @brief Check if a node has an associated instance style */
    auto hasInstanceStyle(const Node::Pointer& node) const -> bool;
    /** @copydoc hasInstanceStyle() */
    auto hasInstanceStyle(const Node* node) const -> bool;
    /** @brief Erase the style associated with a node instance */
    void eraseInstanceStyle(const Node::Pointer& node);
    /** @copydoc eraseInstanceStyle */
    void eraseInstanceStyle(const Node* node);
    /**
     * @brief Access the style for a node instance
     *
     * If a style has not been set, one is created and returned.
     */
    auto instanceStyle(const Node::Pointer& node) -> NodeStyle&;
    /**
     * @copybrief instanceStyle
     *
     * @throws std::out_of_range if a style has not been set
     */
    auto instanceStyle(const Node::Pointer& node) const -> const NodeStyle&;
    /** @copydoc instanceStyle() */
    auto instanceStyle(const Node* node) -> NodeStyle&;
    /** @copydoc instanceStyle(const Node::Pointer&) const */
    auto instanceStyle(const Node* node) const -> const NodeStyle&;
    /** @} */

    /**
     * @name Evaluated Node Style
     * The evaluated node style represents the final style for a specific node
     * after all property precedence relationships have been resolved. This is
     * the style used by smgl::WriteDotFile.
     */
    /** @{ */
    /** @brief Get the evaluated node style for a specific node */
    auto nodeStyle(const Node::Pointer& node) const -> NodeStyle;
    /** @copybrief nodeStyle() */
    auto nodeStyle(const Node* node) const -> NodeStyle;
    /** @} */

    /**
     * @name Node Placement
     * Allows basic placement of nodes according to manually defined rank rules.
     * See the
     * [Graphviz rank documentation](https://graphviz.org/docs/attrs/rank/)
     * for more information.
     */
    /** @{ */
    /** @brief Assign node(s) to the minimum rank */
    template <typename... Args>
    void setRankMin(const Args&... args);

    /**
     * @brief Assign node(s) to the minimum rank and limits the minimum rank to
     * nodes with rank=min or rank=source.
     */
    template <typename... Args>
    void setRankSource(const Args&... args);

    /** @brief Assigns node(s) to the maximum rank */
    template <typename... Args>
    void setRankMax(const Args&... args);

    /**
     * @brief Assign node(s) to the maximum rank and limits the maximum rank to
     * nodes with rank=max or rank=sink.
     */
    template <typename... Args>
    void setRankSink(const Args&... args);

    /**
     * @brief Assign node(s) to a new group ranking
     *
     * Creates a new rank group and assigns the node(s) to that group. Returns
     * the index of the group ranking so that nodes can be appended to the group
     * with appendRankSame().
     */
    template <typename... Args>
    auto setRankSame(const Args&... args) -> std::size_t;

    /**
     * @brief Assign node(s) to an existing group ranking
     *
     * Assign the node(s) to an existing group ranking created with
     * setRankSame().
     */
    template <typename T, typename... Args>
    void appendRankSame(T idx, const Args&... args);

    /** @brief Get the set of nodes assigned to the minimum rank */
    auto rankMin() -> std::unordered_set<Uuid>&;
    /** @copydoc rankMin() */
    auto rankMin() const -> const std::unordered_set<Uuid>&;

    /** @brief Get the set of nodes assigned to the source rank */
    auto rankSource() -> std::unordered_set<Uuid>&;
    /** @copydoc rankSource() */
    auto rankSource() const -> const std::unordered_set<Uuid>&;

    /** @brief Get the set of nodes assigned to the maximum rank */
    auto rankMax() -> std::unordered_set<Uuid>&;
    /** @copydoc rankMax() */
    auto rankMax() const -> const std::unordered_set<Uuid>&;

    /** @brief Get the set of nodes assigned to the sink rank */
    auto rankSink() -> std::unordered_set<Uuid>&;
    /** @copydoc rankSink() */
    auto rankSink() const -> const std::unordered_set<Uuid>&;

    /** @brief Get the set of rank groups */
    auto rankSame() -> std::vector<std::vector<Uuid>>&;
    /** @copydoc rankSame() */
    auto rankSame() const -> const std::vector<std::vector<Uuid>>&;
    /** @} */

private:
    /** Default node style */
    NodeStyle defaultStyle_;
    /** Class-specific node styles */
    std::unordered_map<std::string, NodeStyle> classStyles_;
    /** Instance-specific node styles */
    std::unordered_map<Uuid, NodeStyle> instanceStyles_;
    /** rank=min nodes */
    std::unordered_set<Uuid> rankMin_;
    /** rank=source nodes */
    std::unordered_set<Uuid> rankSrc_;
    /** rank=max nodes */
    std::unordered_set<Uuid> rankMax_;
    /** rank=sink nodes */
    std::unordered_set<Uuid> rankSink_;
    /** rank=same node groups */
    std::vector<std::vector<Uuid>> rankSame_;
};

/** @brief Write Graph to a file in the Graphviz Dot format */
void WriteDotFile(
    const filesystem::path& path,
    const Graph& g,
    const GraphStyle& style = GraphStyle());

} // namespace smgl

#include "smgl/GraphvizImpl.hpp"