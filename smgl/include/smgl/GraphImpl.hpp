namespace smgl
{

template <typename N, typename... Ns>
void Graph::insertNodes(N& n0, Ns&&... nodes)
{
    insertNode(n0);

    // C++11 parameter unpacking
    // https://stackoverflow.com/a/17340003

#if __cplusplus >= 201703L
    // C++17 folding
    (insertNode(nodes), ...);
#elif __cplusplus > 201103L
    // C++11 expansion type
    detail::ExpandType{0, (insertNode(std::forward<Ns>(nodes)), 0)...};
#endif
}

}  // namespace smgl
