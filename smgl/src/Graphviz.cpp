#include "smgl/Graphviz.hpp"

#include <fstream>
#include <iomanip>
#include <regex>

#include "smgl/Node.hpp"

using namespace smgl;
namespace fs = filesystem;

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

// Write a Node n to of in Dot format
void WriteNode(std::ofstream& of, const Node::Pointer& n);

void smgl::WriteDotFile(const fs::path& path, const Graph& g)
{
    std::ofstream dot(path.string());
    // Open graph
    dot << "digraph " << Quote(ShortId(g.uuid())) << " {" << std::endl;

    // Write node and its connections
    dot << "node [shape=none,margin=0];" << std::endl;
    for (const auto& n : g.nodes_) {
        WriteNode(dot, n.second);
    }

    // Write graph label
    /* Disabled until there's more time to style this
    dot << "label=<" << std::endl;
    dot << "Graph<br/>" << std::endl;
    dot << "<font point-size=\"11\">uuid: <i>" << g.uuid().string()
        << "</i></font>" << std::endl;
    dot << ">;" << std::endl;
    dot << "labelloc=top" << std::endl;
    */

    // Close graph
    dot << "}" << std::endl;
    dot.close();
}

void WriteNode(std::ofstream& of, const Node::Pointer& n)
{
    auto inputInfo = n->getInputPortsInfo();
    auto outputInfo = n->getOutputPortsInfo();
    auto cols = std::max(inputInfo.size(), outputInfo.size());

    // Write node id
    of << Quote(ShortId(n->uuid()));

    // begin label
    of << " [label=<" << std::endl;

    // begin table
    of << "<table border=" << Quote(0);
    of << " cellborder=" << Quote(1);
    of << " cellspacing=" << Quote(0);
    of << " cellpadding=" << Quote(4);
    of << ">" << std::endl;

    // Input ports
    if (not inputInfo.empty()) {
        auto colspan = cols / inputInfo.size();
        of << "<tr>" << std::endl;
        for (const auto& i : inputInfo) {
            of << "<td port=" << Quote(ShortId(i.uuid)) << " ";
            of << "colspan=" << Quote(colspan);
            of << ">";
            of << EscapeAll(i.name);
            of << "</td>" << std::endl;
        }
        of << "</tr>" << std::endl;
    }

    // Node label
    of << "<tr>" << std::endl;
    of << "<td colspan=" << Quote(cols) << ">";
    Metadata meta;
    if (IsRegistered(n)) {
        of << EscapeAll(NodeName(n));
        meta = n->serialize(false, "");
    } else {
        of << n->uuid().string();
    }
    if (meta.contains("data") and not meta["data"].empty()) {
        for (const auto& p : meta["data"].items()) {
            of << "<br/> <i><sub>" << p.key() << ": ";
            of << EscapeAll(p.value().dump()) << "</sub></i>" << std::endl;
        }
    }
    of << "</td>" << std::endl;
    of << "</tr>" << std::endl;

    // Output ports
    if (not outputInfo.empty()) {
        auto colspan = cols / outputInfo.size();
        of << "<tr>" << std::endl;
        for (const auto& i : outputInfo) {
            of << "<td port=" << Quote(ShortId(i.uuid)) << " ";
            of << "colspan=" << Quote(colspan);
            of << ">";
            of << EscapeAll(i.name);
            of << "</td>" << std::endl;
        }
        of << "</tr>" << std::endl;
    }

    // end table
    of << "</table>" << std::endl;

    // end label
    of << ">];" << std::endl;

    // Write connections
    for (const auto& c : n->getOutputConnections()) {
        of << Quote(ShortId(c.srcNode->uuid())) << ":";
        of << Quote(ShortId(c.srcPort->uuid())) << ":s";
        of << " -> ";
        of << Quote(ShortId(c.destNode->uuid())) << ":";
        of << Quote(ShortId(c.destPort->uuid())) << ":n";
        of << ";" << std::endl;
    }
}