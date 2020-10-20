#include "smgl/Utilities.hpp"

// From https://stackoverflow.com/a/4541470
std::string smgl::detail::demangle(const char* name)
{
    int status{-1};
    std::unique_ptr<char, void (*)(void*)> res{
        abi::__cxa_demangle(name, nullptr, nullptr, &status), std::free};
    return (status == 0) ? res.get() : name;
}
