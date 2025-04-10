#include "smgl/Utilities.hpp"

#ifdef SMGL_USE_CXXABI
#include <cxxabi.h>
// From https://stackoverflow.com/a/4541470
auto smgl::detail::demangle(const char* name) -> std::string
{
    int status{-1};
    std::unique_ptr<char, void (*)(void*)> res{
        abi::__cxa_demangle(name, nullptr, nullptr, &status), std::free};
    return status == 0 ? res.get() : name;
}
#else
auto smgl::detail::demangle(const char* name) -> std::string { return name; }
#endif
