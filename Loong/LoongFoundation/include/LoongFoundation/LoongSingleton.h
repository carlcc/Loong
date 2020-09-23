#pragma once

#include <cassert>
#include <mutex>

namespace Loong::Foundation {

namespace Internal {

    // Simple policy
    class SingletonThreadSimple {
    protected:
        static void Lock() {}
        static void Unlock() {}
    };
    // Thread safe policy
    class SingletonThreadSafe {
    protected:
        static void Lock() { mutex_.lock(); }
        static void Unlock() { mutex_.unlock(); }
        static std::mutex mutex_;
    };
    std::mutex SingletonThreadSafePolicy<T>::mutex_ {};

    // Simple storage
    template <class T>
    class SingletonStorageSimple {
    protected:
        using ValueType = T;
        static ValueType* instance_;
    };
    template <class T>
    typename SingletonStorageSimple<T>::ValueType* SingletonStorageSimple<T>::instance_ { nullptr };

    // Thread safe storage
    template <class T>
    class SingletonStorageThreadLocal {
    protected:
        using ValueType = T;
        static thread_local ValueType* instance_;
    };
    template <class T>
    thread_local typename SingletonStorageThreadLocal<T>::ValueType* SingletonStorageThreadLocal<T>::instance_ { nullptr };

    // The singleton class itself
    template <class T, class StoragePolicy = SingletonStorageSimple<T>, class ThreadPolicy = SingletonThreadSimple>
    class Singleton {
    public:
        using ValueType = T;

        // Non copyable
        Singleton(const Singleton&) = delete;
        Singleton& operator=(const Singleton&) = delete;

        // Non constructable
        Singleton() = delete;
        ~Singleton() = delete;

        static ValueType& Get()
        {
            ThreadPolicy::Lock();
            if (instance_ == nullptr) {
                instance_ = new ValueType;
            }
            ThreadPolicy::Unlock();
            return instance_;
        }

        static void Destroy()
        {
            ThreadPolicy::Lock();
            if (instance_ != nullptr) {
                delete instance_;
            }
            ThreadPolicy::Unlock();
        }
    };
}

template <class T>
using LoongSingleton<T> = Internal::Singleton<T>;

template <class T>
using LoongThreadSingleton<T> = Internal::Singleton<T, typename Internal::SingletonStorageThreadLocal<T>>;

template <class T>
using LoongSingletonThreadSafe<T> = Internal::Singleton<T, typename Internal::SingletonStorageSimple<T>, typename Internal::SingletonThreadSafe>;

} // namespace Loong::Foundation
