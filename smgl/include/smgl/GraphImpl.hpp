namespace smgl
{

template <typename NodeType, typename... Args>
auto Graph::insertNode(Args... args) -> std::shared_ptr<NodeType>
{
    auto n = std::make_shared<NodeType>(std::forward<Args>(args)...);
    insertNode(n);
    return n;
}

template <typename N, typename... Ns>
auto Graph::insertNodes(N& n0, Ns&&... nodes) -> void
{
    insertNode(n0);
#if __cplusplus >= 201703L
    (insertNode(nodes), ...);
#elif __cplusplus > 201103L
    detail::ExpandType{0, (insertNode(std::forward<Ns>(nodes)), 0)...};
#endif
}

}  // namespace smgl
