#include "smgl/Graphviz.hpp"

#include <fstream>
#include <iomanip>
#include <regex>
#include <sstream>

#include "smgl/Node.hpp"

using namespace smgl;
namespace fs = filesystem;

static ElementStyle MergeElementStyles(
    const ElementStyle& a, const ElementStyle& b)
{
    ElementStyle result;
    result.align = (b.align.empty()) ? a.align : b.align;
    result.bgcolor = (b.bgcolor.empty()) ? a.bgcolor : b.bgcolor;
    result.border = (b.border == 1) ? a.border : b.border;
    result.color = (b.color.empty()) ? a.color : b.color;
    return result;
}

static BaseStyle MergeTableStyles(const BaseStyle& a, const BaseStyle& b)
{
    BaseStyle result;
    result.align = (b.align.empty()) ? a.align : b.align;
    result.bgcolor = (b.bgcolor.empty()) ? a.bgcolor : b.bgcolor;
    result.border = (b.border == 1) ? a.border : b.border;
    result.color = (b.color.empty()) ? a.color : b.color;
    result.cellborder = (b.cellborder == 1) ? a.cellborder : b.cellborder;
    result.cellpadding = (b.cellpadding == 2) ? a.cellpadding : b.cellpadding;
    result.cellspacing = (b.cellspacing == 2) ? a.cellspacing : b.cellspacing;
    return result;
}

static FontStyle MergeFontStyles(const FontStyle& a, const FontStyle& b)
{
    FontStyle result;
    result.color = (b.color.empty()) ? a.color : b.color;
    result.face = (b.face.empty()) ? a.face : b.face;
    return result;
}

static NodeStyle MergeNodeStyles(const NodeStyle& a, const NodeStyle& b)
{
    NodeStyle result;
    result.base = MergeTableStyles(a.base, b.base);
    result.font = MergeFontStyles(a.font, b.font);
    result.inputPorts = MergeElementStyles(a.inputPorts, b.inputPorts);
    result.label = MergeElementStyles(a.label, b.label);
    result.outputPorts = MergeElementStyles(a.outputPorts, b.outputPorts);
    return result;
}

GraphStyle::GraphStyle()
{
    defaultStyle_.base.border = 0;
    defaultStyle_.base.cellpadding = 4;
    defaultStyle_.base.cellspacing = 0;
}

void GraphStyle::setDefaultStyle(const NodeStyle& style)
{
    defaultStyle_ = style;
}

NodeStyle& GraphStyle::defaultStyle() { return defaultStyle_; }

const NodeStyle& GraphStyle::defaultStyle() const { return defaultStyle_; }

void GraphStyle::setNodeClassStyle(
    const std::string& name, const NodeStyle& style)
{
    classStyles_[name] = style;
}

void GraphStyle::setNodeInstanceStyle(
    const Node::Pointer& node, const NodeStyle& style)
{
    instanceStyles_[node->uuid()] = style;
}

void GraphStyle::setNodeInstanceStyle(const Node* node, const NodeStyle& style)
{
    instanceStyles_[node->uuid()] = style;
}

bool GraphStyle::hasNodeClassStyle(const std::string& name) const
{
    return classStyles_.count(name) > 0;
}

void GraphStyle::eraseNodeClassStyle(const std::string& name)
{
    classStyles_.erase(name);
}

bool GraphStyle::hasNodeInstanceStyle(const Node::Pointer& node) const
{
    return instanceStyles_.count(node->uuid()) > 0;
}

bool GraphStyle::hasNodeInstanceStyle(const Node* node) const
{
    return instanceStyles_.count(node->uuid()) > 0;
}

NodeStyle& GraphStyle::nodeClassStyle(const std::string& name)
{
    return classStyles_[name];
}

const NodeStyle& GraphStyle::nodeClassStyle(const std::string& name) const
{
    return classStyles_.at(name);
}

NodeStyle& GraphStyle::nodeInstanceStyle(const Node::Pointer& node)
{
    return instanceStyles_[node->uuid()];
}

const NodeStyle& GraphStyle::nodeInstanceStyle(const Node::Pointer& node) const
{
    return instanceStyles_.at(node->uuid());
}

NodeStyle& GraphStyle::nodeInstanceStyle(const Node* node)
{
    return instanceStyles_[node->uuid()];
}

const NodeStyle& GraphStyle::nodeInstanceStyle(const Node* node) const
{
    return instanceStyles_.at(node->uuid());
}

void GraphStyle::eraseNodeInstanceStyle(const Node::Pointer& node)
{
    instanceStyles_.erase(node->uuid());
}

void GraphStyle::eraseNodeInstanceStyle(const Node* node)
{
    instanceStyles_.erase(node->uuid());
}

NodeStyle GraphStyle::nodeStyle(const Node::Pointer& node) const
{
    auto style = defaultStyle_;

    auto className = NodeName(node);
    if (hasNodeClassStyle(className)) {
        style = MergeNodeStyles(style, nodeClassStyle(className));
    }

    if (hasNodeInstanceStyle(node)) {
        style = MergeNodeStyles(style, nodeInstanceStyle(node));
    }
    return style;
}

NodeStyle GraphStyle::nodeStyle(const Node* node) const
{
    auto style = defaultStyle_;

    auto className = NodeName(node);
    if (hasNodeClassStyle(className)) {
        style = MergeNodeStyles(style, nodeClassStyle(className));
    }

    if (hasNodeInstanceStyle(node)) {
        style = MergeNodeStyles(style, nodeInstanceStyle(node));
    }
    return style;
}

// Get the Short UUID (first 8 digits)
inline std::string ShortId(const Uuid& u) { return u.string().substr(0, 8); }

// Create quoted string for type
template <typename T>
inline std::string Quote(const T& s)
{
    return "\"" + std::to_string(s) + "\"";
}

// Add quotes to a string
template <>
inline std::string Quote(const std::string& s)
{
    return "\"" + s + "\"";
}

// Escape " for Graphviz HTML Labels
static std::string EscapeQuote(const std::string& s) {
    return std::regex_replace(s, std::regex{"\""}, "&quot;");
}

// Escape & for Graphviz HTML Labels
static std::string EscapeAmpersand(const std::string& s)
{
    return std::regex_replace(s, std::regex{"&"}, "&amp;");
}

// Escape <> for Graphviz HTML Labels
static std::string EscapeHTMLTag(const std::string& s) {
    // <
    auto res = std::regex_replace(s, std::regex{"<"}, "&lt;");

    // >
    res = std::regex_replace(res, std::regex{">"}, "&gt;");

    return res;
}

// Escape all special characters for Graphviz HTML Labels
static std::string EscapeAll(const std::string& s)
{
    // &
    auto res = EscapeAmpersand(s);

    // < >
    res = EscapeHTMLTag(res);

    // "
    return EscapeQuote(res);
}

static std::string FontTagString(const FontStyle& style)
{
    std::stringstream ss;
    ss << "<font";
    if (not style.color.empty()) {
        ss << " color=" + Quote(style.color);
    }
    if (not style.face.empty()) {
        ss << " face=" + Quote(style.face);
    }
    ss << ">\n";
    return ss.str();
}

static std::string ElementStyleString(const ElementStyle& style)
{
    std::stringstream ss;
    if (not style.align.empty()) {
        ss << " align=" << Quote(style.align);
    }
    if (not style.bgcolor.empty()) {
        ss << " bgcolor=" << Quote(style.bgcolor);
    }
    if (style.border != 1) {
        ss << " border=" << Quote(style.border);
    }
    if (not style.color.empty()) {
        ss << " color=" << Quote(style.color);
    }
    return ss.str();
}

static std::string TableStyleString(const BaseStyle& style)
{
    std::stringstream ss;
    ss << ElementStyleString(style);
    if (style.cellborder != 1 or style.border == 0) {
        ss << " cellborder=" << Quote(style.cellborder);
    }
    if (style.cellspacing != 2) {
        ss << " cellspacing=" << Quote(style.cellspacing);
    }
    if (style.cellpadding != 2) {
        ss << " cellpadding=" << Quote(style.cellpadding);
    }
    return ss.str();
}

// Write a Node n to of in Dot format
void WriteNode(
    std::ofstream& of, const Node::Pointer& n, const GraphStyle& node);

void smgl::WriteDotFile(
    const fs::path& path, const Graph& g, const GraphStyle& style)
{
    std::ofstream dot(path.string());

    // Open graph
    dot << "digraph " << Quote(ShortId(g.uuid())) << " {\n";

    // Write node and its connections
    dot << "node [shape=plain];\n";
    for (const auto& n : g.nodes_) {
        WriteNode(dot, n.second, style);
    }

    // Write graph label
    /* Disabled until there's more time to style this
    dot << "label=<" << "\n";
    dot << "Graph<br/>" << "\n";
    dot << "<font point-size=\"11\">uuid: <i>" << g.uuid().string()
        << "</i></font>" << "\n";
    dot << ">;" << "\n";
    dot << "labelloc=top" << "\n";
    */

    // Close graph
    dot << "}\n";
    dot.close();
}

void WriteNode(
    std::ofstream& of, const Node::Pointer& n, const GraphStyle& style)
{
    auto inputInfo = n->getInputPortsInfo();
    auto outputInfo = n->getOutputPortsInfo();
    auto cols = std::max(inputInfo.size(), outputInfo.size());
    auto nodeStyle = style.nodeStyle(n);

    // Write node id
    of << Quote(ShortId(n->uuid()));

    // begin label
    of << " [label=<\n";

    // style: font begin
    if (not nodeStyle.font.empty()) {
        of << FontTagString(nodeStyle.font);
    }

    // begin table
    of << "<table";
    of << TableStyleString(nodeStyle.base);
    of << ">\n";

    // Input ports
    auto inputStyle = ElementStyleString(nodeStyle.inputPorts);
    if (not inputInfo.empty()) {
        auto colspan = cols / inputInfo.size();
        of << "<tr>\n";
        for (const auto& i : inputInfo) {
            of << "<td";
            of << " port=" << Quote(ShortId(i.uuid));
            of << " colspan=" << Quote(colspan);
            of << inputStyle;
            of << ">";
            of << EscapeAll(i.name);
            of << "</td>\n";
        }
        of << "</tr>\n";
    }

    // Node label
    of << "<tr>\n";
    of << "<td";
    of << " colspan=" << Quote(cols);
    of << ElementStyleString(nodeStyle.label);
    of << ">";
    Metadata meta;
    if (IsRegistered(n)) {
        of << EscapeAll(NodeName(n));
        meta = n->serialize(false, "");
    } else {
        of << n->uuid().string();
    }
    if (meta.contains("data") and not meta["data"].empty()) {
        // Dump each printable data element
        // indent = 1 to pretty print arrays
        for (const auto& p : meta["data"].items()) {
            of << "<br/> <i><sub>" << p.key() << ": ";
            of << EscapeAll(p.value().dump(1)) << "</sub></i>\n";
        }
    }
    of << "</td>\n";
    of << "</tr>\n";

    // Output ports
    auto outputStyle = ElementStyleString(nodeStyle.outputPorts);
    if (not outputInfo.empty()) {
        auto colspan = cols / outputInfo.size();
        of << "<tr>\n";
        for (const auto& i : outputInfo) {
            of << "<td";
            of << " port=" << Quote(ShortId(i.uuid));
            of << " colspan=" << Quote(colspan);
            of << outputStyle;
            of << ">";
            of << EscapeAll(i.name);
            of << "</td>\n";
        }
        of << "</tr>\n";
    }

    // end table
    of << "</table>\n";

    // style: font end
    if (not nodeStyle.font.empty()) {
        of << "</font>\n";
    }

    // end label
    of << ">];\n";

    // Write connections
    for (const auto& c : n->getOutputConnections()) {
        of << Quote(ShortId(c.srcNode->uuid())) << ":";
        of << Quote(ShortId(c.srcPort->uuid())) << ":s";
        of << " -> ";
        of << Quote(ShortId(c.destNode->uuid())) << ":";
        of << Quote(ShortId(c.destPort->uuid())) << ":n";
        of << ";\n";
    }
}