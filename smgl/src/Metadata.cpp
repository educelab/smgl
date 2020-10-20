#include "smgl/Metadata.hpp"

#include <fstream>
#include <iomanip>

using namespace smgl;

void smgl::WriteMetadata(const filesystem::path& path, const Metadata& m)
{
    // TODO: string() not needed with std::filesystem
    std::ofstream o(path.string());
    o << std::setw(4) << m << std::endl;
}

Metadata smgl::LoadMetadata(const filesystem::path& path)
{
    Metadata m;
    std::ifstream i(path.string());
    i >> m;
    return m;
}