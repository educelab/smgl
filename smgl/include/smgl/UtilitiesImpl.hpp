#include <cstdlib>
#include <memory>
#include <typeinfo>
#include <cxxabi.h>

namespace smgl
{

template <class Obj, class ObjMemberFn, typename... Args>
auto WrapFunc(Obj* obj, ObjMemberFn&& fn, Args&&... args)
{
    return [=]() { return (*obj.*fn)(args...); };
}

namespace detail
{
// From https://stackoverflow.com/a/4541470
std::string demangle(const char* name)
{
    int status{-1};
    std::unique_ptr<char, void (*)(void*)> res{
        abi::__cxa_demangle(name, nullptr, nullptr, &status), std::free};
    return (status == 0) ? res.get() : name;
}

template <class T>
std::string type_name()
{
    return demangle(typeid(T).name());
}

template <class T>
std::string type_name(const T& t)
{
    return demangle(typeid(t).name());
}

template <typename... T>
ExpandType::ExpandType(T&&...)
{
}

}  // namespace detail
}  // namespace smgl
