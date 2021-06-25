#pragma once

/** @file */

#include <string>
#include <unordered_map>

#include "smgl/Graph.hpp"
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
    uint8_t border{1};
    /** @brief Element border color */
    std::string color;
};

/** @brief Style struct for a node's base style */
struct BaseStyle : public ElementStyle {
    /** @brief Sub-element border thickness [0, 255] */
    uint8_t cellborder{1};
    /** @brief Sub-element border padding [0, 255] */
    uint8_t cellpadding{2};
    /** @brief Sub-element border spacing [0, 255] */
    uint8_t cellspacing{2};
};

/** @brief Style struct for a node's overall font style */
struct FontStyle {
    /** @brief Font color */
    std::string color;
    /** @brief Font typeface */
    std::string face;
    /** @brief Returns whether or not this style has an values set */
    bool empty() const { return color.empty() and face.empty(); }
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
 * validate the values set in properties. For more information on valid property
 * values, see the Doxygen documentation for
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
    NodeStyle& defaultStyle();
    /** @copydoc defaultStyle() */
    const NodeStyle& defaultStyle() const;
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
    void setClassStyle(const NodeStyle& style)
    {
        setClassStyle(NodeName<NodeT>(), style);
    }
    /** @brief Check if a class of nodes has an associated style */
    bool hasClassStyle(const std::string& name) const;
    /** @copydoc hasClassStyle() */
    template <class NodeT>
    bool hasClassStyle() const
    {
        return hasClassStyle(NodeName<NodeT>());
    }
    /** @brief Erase the style associated with a class of nodes */
    void eraseClassStyle(const std::string& name);
    /** @copydoc eraseClassStyle() */
    template <class NodeT>
    void eraseClassStyle() const
    {
        eraseClassStyle(NodeName<NodeT>());
    }
    /**
     * @brief Access the style for a class of nodes
     *
     * If a style has not been set, one is created and returned.
     */
    NodeStyle& classStyle(const std::string& name);
    /** @copydoc classStyle(const std::string&) */
    template <class NodeT>
    NodeStyle& classStyle()
    {
        return classStyle(NodeName<NodeT>());
    }
    /**
     * @copybrief classStyle(const std::string&)
     *
     * @throws std::out_of_range if a style has not been set
     */
    const NodeStyle& classStyle(const std::string& name) const;
    /** @copydoc classStyle(const std::string&) const */
    template <class NodeT>
    const NodeStyle& classStyle() const
    {
        return classStyle(NodeName<NodeT>());
    }
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
    bool hasInstanceStyle(const Node::Pointer& node) const;
    /** @copydoc hasInstanceStyle() */
    bool hasInstanceStyle(const Node* node) const;
    /** @brief Erase the style associated with a node instance */
    void eraseInstanceStyle(const Node::Pointer& node);
    /** @copydoc eraseInstanceStyle */
    void eraseInstanceStyle(const Node* node);
    /**
     * @brief Access the style for a node instance
     *
     * If a style has not been set, one is created and returned.
     */
    NodeStyle& instanceStyle(const Node::Pointer& node);
    /**
     * @copybrief instanceStyle
     *
     * @throws std::out_of_range if a style has not been set
     */
    const NodeStyle& instanceStyle(const Node::Pointer& node) const;
    /** @copydoc instanceStyle() */
    NodeStyle& instanceStyle(const Node* node);
    /** @copydoc instanceStyle(const Node::Pointer&) const */
    const NodeStyle& instanceStyle(const Node* node) const;
    /** @} */

    /**
     * @name Evaluated Node Style
     * The evaluated node style represents the final style for a specific node
     * after all property precedence relationships have been resolved. This is
     * the style used by smgl::WriteDotFile.
     */
    /** @{ */
    /** @brief Get the evaluated node style for a specific node */
    NodeStyle nodeStyle(const Node::Pointer& node) const;
    /** @copybrief nodeStyle() */
    NodeStyle nodeStyle(const Node* node) const;
    /** @} */

private:
    /** Default node style */
    NodeStyle defaultStyle_;
    /** Class-specific node styles */
    std::unordered_map<std::string, NodeStyle> classStyles_;
    /** Instance-specific node styles */
    std::unordered_map<Uuid, NodeStyle> instanceStyles_;
};

/** @brief Write Graph to a file in the Graphviz Dot format */
void WriteDotFile(
    const filesystem::path& path,
    const Graph& g,
    const GraphStyle& style = GraphStyle());

} // namespace smgl
