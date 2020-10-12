#pragma once

/** @file */

#include <cassert>
#include <exception>
#include <functional>

namespace smgl
{

namespace policy
{
/** @brief Alias for a function type that can be passed to std::atexit */
using AtExitFunc = void (*)();

/** @brief Creation policy that uses new to allocate an object on the stack */
template <typename T>
struct CreateUsingNew {
    /** Create using call to new */
    static T* Create() { return new T; }
    /** Destroy using delete */
    static void Destroy(T* obj) { delete obj; }
};

/** @brief Creation policy which allocates a static object */
template <typename T>
struct CreateStatic {
    /** Create static instance with aligned buffer */
    static T* Create()
    {
        // Create aligned static buffer
        alignas(alignof(T)) static char buf[sizeof(T)];
        return new (&buf) T;
    }
    /** Destroy using object's destructor */
    static void Destroy(T* obj) { obj->~T(); }
};

/** @brief Lifetime policy which schedules destruction using std::atexit */
template <typename T>
struct DefaultLifetime {
    /** Schedule destruction using std::atexit */
    static void ScheduleDestruction(AtExitFunc destructionFn)
    {
        std::atexit(destructionFn);
    }
    /**
     * Handle dead reference to an object
     * @throws std::logic_error
     */
    static void OnDeadReference()
    {
        throw std::logic_error{"Dead reference detected"};
    }
};

/**
 * @brief Lifetime policy which reconstructs a destroyed singleton if reaccessed
 *
 * Note that this constructs a new object and does not restore the previous
 * state of the singleton.
 */
template <typename T>
class PhoenixLifetime
{
public:
    /** Schedule destruction using std::atexit if not destroyed already */
    static void ScheduleDestruction(AtExitFunc destructionFn)
    {
        if (not destroyed_) {
            std::atexit(destructionFn);
        }
    }

    /** Sets destroyed state to true */
    static void OnDeadReference() { destroyed_ = true; }

private:
    /** Whether singleton has been destroyed */
    static bool destroyed_;
};

template <typename T>
bool PhoenixLifetime<T>::destroyed_ = false;

/** @brief Threading policy for single-threaded applications */
template <typename T>
struct SingleThreaded {
    /** Underlying type */
    using VolatileType = T;
    /** @brief Single-threaded lock type which does nothing */
    struct Lock {
        /** Default constructor does nothing */
        Lock() = default;
        /** Lock for specific object does nothing */
        explicit Lock(T& /* unused */) {}
    };
};
}  // namespace policy

namespace detail
{
/**
 * @brief Class for constructing and managing the lifetime of a singleton object
 *
 * This class provides singleton management functionality to a provided template
 * type. The lifetime management behaviors of the singleton are managed by the
 * provided template policies: CreationPolicy manages allocation and
 * deallocation of the object, LifetimePolicy manages when the Singleton is
 * destroyed, and ThreadingModel manages any locking mechanics for multithreaded
 * applications. This class only supports class-level singletons.
 *
 * Derived from "Modern C++ Design" by Andrei Alexandrescu and the Loki library.
 */
// clang-format off
template <
    class T,
    template <class> class CreationPolicy = policy::CreateStatic,
    template <class> class LifetimePolicy = policy::DefaultLifetime,
    template <class> class ThreadingModel = policy::SingleThreaded
>
// clang-format on
class SingletonHolder
{
public:
    /**
     * Underlying type of the singleton object. Often, but not always, `T`. For
     * multi-threaded ThreadingModels, could be `volatile T`.
     */
    using InstanceType = typename ThreadingModel<T>::VolatileType;

    /** Instance cannot be constructed */
    SingletonHolder() = delete;

    /** @brief Access the singleton instance */
    static T& Instance();

private:
    /** Destroys the wrapped singleton object */
    static void DestroySingleton();
    /** Pointer to the Singleton instance */
    static InstanceType* instance_;
    /** Whether the Singleton has been destroyed */
    static bool destroyed_;
};

}  // namespace detail
}  // namespace smgl

#include "smgl/SingletonImpl.hpp"