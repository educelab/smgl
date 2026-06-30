namespace smgl
{

template <class Obj, class ObjMemberFn, typename... Args>
auto WrapFunc(Obj* obj, ObjMemberFn&& fn, Args&&... args)
{
    return [=]() { return (*obj.*fn)(args...); };
}

namespace detail
{
template <typename... T>
ExpandType::ExpandType(T&&...)
{
}

}  // namespace detail
}  // namespace smgl
