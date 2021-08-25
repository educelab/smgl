namespace smgl
{

template <typename NodeType, typename... Args>
std::shared_ptr<NodeType> Graph::insertNode(Args... args)
{
    auto n = std::make_shared<NodeType>(std::forward<Args>(args)...);
    insertNode(n);
    return n;
}

template <typename N, typename... Ns>
void Graph::insertNodes(N& n0, Ns&&... nodes)
{
    insertNode(n0);
#if __cplusplus >= 201703L
    (insertNode(nodes), ...);
#elif __cplusplus > 201103L
    detail::ExpandType{0, (insertNode(std::forward<Ns>(nodes)), 0)...};
#endif
}

}  // namespace smgl
