namespace smgl
{
namespace detail
{

/* Initialize the instance pointer */
// clang-format off
template <
    class T,
    template <class> class C,
    template <class> class L,
    template <class> class M
>
// clang-format on
typename SingletonHolder<T, C, L, M>::InstanceType*
    SingletonHolder<T, C, L, M>::instance_ = nullptr;

/* Initialize the destroyed boolean */
// clang-format off
template <
    class T,
    template <class> class C,
    template <class> class L,
    template <class> class M
>
// clang-format on
bool SingletonHolder<T, C, L, M>::destroyed_ = false;

/* Instance retrieval implementation */
// clang-format off
template <
    class T,
    template <class> class C,
    template <class> class L,
    template <class> class M
>
// clang-format on
T& SingletonHolder<T, C, L, M>::Instance()
{
    // If we don't have an instance
    if (!instance_) {
        // Use the threading model to get a lock
        typename M<T>::Lock guard;
        // Double-checked locking pattern
        if (!instance_) {
            // Handle the singleton already being destroyed
            if (destroyed_) {
                destroyed_ = false;
                L<T>::OnDeadReference();
            }
            // Construct the singleton
            instance_ = C<T>::Create();
            L<T>::ScheduleDestruction(&DestroySingleton);
        }
    }
    return *instance_;
}

/* Instance destruction implementation */
// clang-format off
template <
    class T,
    template <class> class C,
    template <class> class L,
    template <class> class M
>
//clang-format on
void SingletonHolder<T, C, L, M>::
DestroySingleton()
{
    assert(!destroyed_);
    C<T>::Destroy(instance_);
    instance_ = nullptr;
    destroyed_ = true;
}

}
}