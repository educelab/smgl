namespace smgl
{
namespace policy
{

template <class I, class P>
DefaultFactoryError<I, P>::unknown_identifier::unknown_identifier(
    const I& unknownId)
    : unknownId_{unknownId}
{
}

template <class I, class P>
const char* DefaultFactoryError<I, P>::unknown_identifier::what() const noexcept
{
    return "Unknown object type passed to Factory";
}

template <class I, class P>
P DefaultFactoryError<I, P>::OnUnknownType(const I& id)
{
    throw unknown_identifier(id);
}

template <class I, class P>
I DefaultFactoryError<I, P>::OnUnnamedType(const std::type_info& info)
{
    throw unknown_identifier(info.name());
}
}  // namespace policy

namespace detail
{

template <
    class B,
    typename I,
    class A,
    typename P,
    template <typename, class>
    class E>
bool Factory<B, I, A, P, E>::Register(
    const I& id, P creator, const std::type_info& info)
{
    auto res = idToCreatorMap_.insert({id, creator}).second;
    if (res) {
        res = typeToIDMap_.insert({info.hash_code(), id}).second;
        if (not res) {
            idToCreatorMap_.erase(id);
        }
    }
    return res;
}

template <
    class B,
    typename I,
    class A,
    typename P,
    template <typename, class>
    class E>
bool Factory<B, I, A, P, E>::Deregister(const I& id)
{
    // emulate std::erase_if from C++20
    auto oldSize = typeToIDMap_.size();
    for (auto it = typeToIDMap_.begin(), last = typeToIDMap_.end();
         it != last;) {
        if (it->second == id) {
            it = typeToIDMap_.erase(it);
        } else {
            ++it;
        }
    }
    auto cnt = oldSize - typeToIDMap_.size();
    return cnt == 1 and idToCreatorMap_.erase(id) == 1;
}

template <
    class B,
    typename I,
    class A,
    typename P,
    template <typename, class>
    class E>
A Factory<B, I, A, P, E>::CreateObject(const I& id)
{
    auto it = idToCreatorMap_.find(id);
    if (it != idToCreatorMap_.end()) {
        return (it->second)();
    }
    return this->OnUnknownType(id);
}
template <
    class B,
    typename I,
    class A,
    typename P,
    template <typename, class>
    class E>
I Factory<B, I, A, P, E>::GetTypeIdentifier(const std::type_info& info)
{
    auto it = typeToIDMap_.find(info.hash_code());
    if (it != typeToIDMap_.end()) {
        return it->second;
    }
    return this->OnUnnamedType(info);
}

template <
    class B,
    typename I,
    class A,
    typename P,
    template <typename, class>
    class E>
std::vector<I> Factory<B, I, A, P, E>::GetRegisteredIdentifiers() const
{
    std::vector<I> keys;
    for (const auto& t : idToCreatorMap_) {
        keys.push_back(t.first);
    }
    return keys;
}

template <
    class B,
    typename I,
    class A,
    typename P,
    template <typename, class>
    class E>
bool Factory<B, I, A, P, E>::IsRegistered(const IDType& id)
{
    return idToCreatorMap_.find(id) != idToCreatorMap_.end();
}

template <
    class B,
    typename I,
    class A,
    typename P,
    template <typename, class>
    class E>
bool Factory<B, I, A, P, E>::IsRegistered(const std::type_info& info)
{
    return typeToIDMap_.find(info.hash_code()) != typeToIDMap_.end();
}

}  // namespace detail
}  // namespace smgl