#include <iomanip>
#include <fstream>

namespace smgl
{
void WriteMetadata(const filesystem::path& path, const Metadata& m)
{
    // TODO: string() not needed with std::filesystem
    std::ofstream o(path.string());
    o << std::setw(4) << m << std::endl;
}

Metadata LoadMetadata(const filesystem::path& path)
{
    Metadata m;
    std::ifstream i(path.string());
    i >> m;
    return m;
}
}  // namespace smgl