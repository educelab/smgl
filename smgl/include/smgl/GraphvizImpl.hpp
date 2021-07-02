#include "smgl/TypeTraits.hpp"

namespace smgl
{

template <class NodeT>
void GraphStyle::setClassStyle(const NodeStyle& style)
{
    setClassStyle(NodeName<NodeT>(), style);
}

template <class NodeT>
bool GraphStyle::hasClassStyle() const
{
    return hasClassStyle(NodeName<NodeT>());
}

template <class NodeT>
void GraphStyle::eraseClassStyle() const
{
    eraseClassStyle(NodeName<NodeT>());
}

template <class NodeT>
NodeStyle& GraphStyle::classStyle()
{
    return classStyle(NodeName<NodeT>());
}

template <class NodeT>
const NodeStyle& GraphStyle::classStyle() const
{
    return classStyle(NodeName<NodeT>());
}

template <typename... Args>
void GraphStyle::setRankMin(const Args&... args)
{
    static_assert(sizeof...(args) > 0, "Missing arguments");
    using namespace type_traits;
#if __cplusplus >= 201703L
    static_assert(
        (is_ptr_to_base_v<decltype(args), Node> && ...),
        "Argument is not a Node pointer");
    (rankMin_.insert(args->uuid()), ...);
#elif __cplusplus > 201103L
    static_assert(
        all_true_v<(is_ptr_to_base_v<decltype(args), Node>)...>,
        "Argument is not a Node pointer");
    detail::ExpandType{0, (rankMin_.insert(args->uuid()), 0)...};
#endif
}

template <typename... Args>
void GraphStyle::setRankSource(const Args&... args)
{
    static_assert(sizeof...(args) > 0, "Missing arguments");
    using namespace type_traits;
#if __cplusplus >= 201703L
    static_assert(
        (is_ptr_to_base_v<decltype(args), Node> && ...),
        "Argument is not a Node pointer");
    (rankSrc_.insert(args->uuid()), ...);
#elif __cplusplus > 201103L
    static_assert(
        all_true_v<(is_ptr_to_base_v<decltype(args), Node>)...>,
        "Argument is not a Node pointer");
    detail::ExpandType{0, (rankSrc_.insert(args->uuid()), 0)...};
#endif
}

template <typename... Args>
void GraphStyle::setRankMax(const Args&... args)
{
    static_assert(sizeof...(args) > 0, "Missing arguments");
    using namespace type_traits;
#if __cplusplus >= 201703L
    static_assert(
        (is_ptr_to_base_v<decltype(args), Node> && ...),
        "Argument is not a Node pointer");
    (rankMax_.insert(args->uuid()), ...);
#elif __cplusplus > 201103L
    static_assert(
        all_true_v<(is_ptr_to_base_v<decltype(args), Node>)...>,
        "Argument is not a Node pointer");
    detail::ExpandType{0, (rankMax_.insert(args->uuid()), 0)...};
#endif
}

template <typename... Args>
void GraphStyle::setRankSink(const Args&... args)
{
    static_assert(sizeof...(args) > 0, "Missing arguments");
    using namespace type_traits;
#if __cplusplus >= 201703L
    static_assert(
        (is_ptr_to_base_v<decltype(args), Node> && ...),
        "Argument is not a Node pointer");
    (rankSink_.insert(args->uuid()), ...);
#elif __cplusplus > 201103L
    static_assert(
        all_true_v<(is_ptr_to_base_v<decltype(args), Node>)...>,
        "Argument is not a Node pointer");
    detail::ExpandType{0, (rankSink_.insert(args->uuid()), 0)...};
#endif
}

template <typename... Args>
std::size_t GraphStyle::setRankSame(const Args&... args)
{
    static_assert(sizeof...(args) > 0, "Missing arguments");
    using namespace type_traits;
    auto idx = rankSame_.size();
    std::vector<Uuid> same;
#if __cplusplus >= 201703L
    static_assert(
        (is_ptr_to_base_v<decltype(args), Node> && ...),
        "Argument is not a Node pointer");
    (same.push_back(args->uuid()), ...);
#elif __cplusplus > 201103L
    static_assert(
        all_true_v<(is_ptr_to_base_v<decltype(args), Node>)...>,
        "Argument is not a Node pointer");
    detail::ExpandType{0, (same.push_back(args->uuid()), 0)...};
#endif
    rankSame_.push_back(same);
    return idx;
}

template <typename T, typename... Args>
void GraphStyle::appendRankSame(T idx, const Args&... args)
{
    static_assert(sizeof...(args) > 0, "Missing arguments");
    using namespace type_traits;
#if __cplusplus >= 201703L
    static_assert(
        (is_ptr_to_base_v<decltype(args), Node> && ...),
        "Argument is not a Node pointer");
    (rankSame_.at(idx).push_back(args->uuid()), ...);
#elif __cplusplus > 201103L
    static_assert(
        all_true_v<(is_ptr_to_base_v<decltype(args), Node>)...>,
        "Argument is not a Node pointer");
    detail::ExpandType{0, (rankSame_.at(idx).push_back(args->uuid()), 0)...};
#endif
}

}  // namespace smgl