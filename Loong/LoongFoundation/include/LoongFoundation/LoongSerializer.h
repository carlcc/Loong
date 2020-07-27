#pragma once

#include "LoongFoundation/LoongMath.h"

// TODO: This is absolutely not a portable serializer, we should rewrite it in the future
namespace Loong::Foundation {

// Derive the following class and implement `bool operator()(void* d, size_t l)` function
class LoongArchiveOutputStream {
};
class LoongArchiveInputStream {
};

template <class T, class Stream, bool isPod = std::is_pod_v<T>>
struct LoongArchiver;

namespace Internal {
    template <class Stream>
    struct ArchiveHelper {
    public:
        explicit ArchiveHelper(Stream& s)
            : stream_(s)
        {
        }
        template <class T>
        bool operator()(T& t)
        {
            return LoongArchiver<T, Stream>()(t, stream_);
        }
        template <class Arg, class... Args>
        bool operator()(Arg& arg, Args&... args)
        {
            return operator()(arg) && operator()(args...);
        }

    private:
        Stream& stream_;
    };
}

template <class T, class Stream>
struct LoongArchiver<T, Stream, true> {
    bool operator()(T& t, Stream& stream)
    {
        if constexpr (std::is_pointer_v<T>) {
            return LoongArchiver<typename std::pointer_traits<T>::element_type, Stream>()(*t, stream);
        } else {
            return stream(&t, sizeof(T));
        }
    }
};

template <class T, class Stream>
struct LoongArchiver<T, Stream, false> {
    bool operator()(T& t, Stream& stream)
    {
        Internal::ArchiveHelper<Stream> helper(stream);
        return t.Serialize(helper);
    }
};

template <class Stream>
struct LoongArchiver<Math::Vector2, Stream> {
    bool operator()(Math::Vector2& t, Stream& stream)
    {
        return stream(&t, sizeof(t));
    }
};

template <class Stream>
struct LoongArchiver<Math::AABB, Stream> {
    bool operator()(Math::AABB& t, Stream& stream)
    {
        return stream(&t, sizeof(t));
    }
};

template <class Stream>
struct LoongArchiver<Math::Vector3, Stream> {
    bool operator()(Math::Vector3& t, Stream& stream)
    {
        return stream(&t, sizeof(t));
    }
};

template <class Stream>
struct LoongArchiver<Math::Vector4, Stream> {
    bool operator()(Math::Vector4& t, Stream& stream)
    {
        return stream(&t, sizeof(t));
    }
};

template <class Stream>
struct LoongArchiver<Math::IVector3, Stream> {
    bool operator()(Math::IVector3& t, Stream& stream)
    {
        return stream(&t, sizeof(t));
    }
};

template <class Stream>
struct LoongArchiver<Math::IVector4, Stream> {
    bool operator()(Math::IVector4& t, Stream& stream)
    {
        return stream(&t, sizeof(t));
    }
};

template <class Stream>
struct LoongArchiver<std::string, Stream> {
    bool operator()(std::string& t, Stream& stream)
    {
        if constexpr (std::is_base_of_v<LoongArchiveOutputStream, Stream>) {
            if (t.length() > std::numeric_limits<uint32_t>::max()) {
                // TODO: LOG
                return false;
            }
            auto size = uint32_t(t.length());
            return LoongArchiver<uint32_t, Stream>()(size, stream) && stream(t.data(), t.length());
        } else if constexpr (std::is_base_of_v<LoongArchiveInputStream, Stream>) {
            uint32_t size;
            if (!LoongArchiver<uint32_t, Stream>()(size, stream)) {
                return false;
            }
            t.resize(size);
            return stream(t.data(), t.length());
        } else {
            static_assert(std::is_base_of_v<LoongArchiveOutputStream, Stream>);
        }
    }
};

template <class T, class Stream>
struct LoongArchiver<std::vector<T>, Stream, false> {
    bool operator()(std::vector<T>& array, Stream& stream)
    {
        if constexpr (std::is_base_of_v<LoongArchiveOutputStream, Stream>) {
            if (array.size() > std::numeric_limits<uint32_t>::max()) {
                // TODO: LOG
                return false;
            }
            auto size = uint32_t(array.size());
            if (!LoongArchiver<uint32_t, Stream>()(size, stream)) {
                return false;
            }
            for (auto& t : array) {
                if (!LoongArchiver<T, Stream>()(t, stream)) {
                    return false;
                }
            }
            return true;
        } else if constexpr (std::is_base_of_v<LoongArchiveInputStream, Stream>) {
            uint32_t size;
            if (!LoongArchiver<uint32_t, Stream>()(size, stream)) {
                return false;
            }
            if constexpr (std::is_pointer_v<T>) {
                for (auto t : array) {
                    delete t;
                }
                array.clear();
            }
            array.resize(size);
            if constexpr (std::is_pointer_v<T>) {
                for (auto& t : array) {
                    t = new typename std::pointer_traits<T>::element_type;
                }
            }
            for (auto& t : array) {
                if (!LoongArchiver<T, Stream>()(t, stream)) {
                    return false;
                }
            }
            return true;
        } else {
            static_assert(std::is_base_of_v<LoongArchiveOutputStream, Stream>);
        }
    }
};

template <class K, class V, class Stream>
struct LoongArchiver<std::map<K, V>, Stream, false> {
    bool operator()(std::map<K, V>& mp, Stream& stream)
    {
        static_assert(!std::is_pointer_v<K>, "Key of a map should not be a pointer");
        static_assert(!std::is_pointer_v<V>, "Value of a map should not be a pointer");
        if constexpr (std::is_base_of_v<LoongArchiveOutputStream, Stream>) {
            if (mp.size() > std::numeric_limits<uint32_t>::max()) {
                // TODO: LOG
                return false;
            }
            auto size = uint32_t(mp.size());
            if (!LoongArchiver<uint32_t, Stream>()(size, stream)) {
                return false;
            }
            for (auto& entry : mp) {
                if (!LoongArchiver<K, Stream>()(const_cast<K&>(entry.first), stream)) {
                    return false;
                }
                if (!LoongArchiver<V, Stream>()(entry.second, stream)) {
                    return false;
                }
            }
            return true;
        } else if constexpr (std::is_base_of_v<LoongArchiveInputStream, Stream>) {
            uint32_t size;
            if (!LoongArchiver<uint32_t, Stream>()(size, stream)) {
                return false;
            }
            for (uint32_t i = 0; i < size; ++i) {
                K k;
                V v;
                if (!LoongArchiver<K, Stream>()(k, stream)) {
                    return false;
                }
                if (!LoongArchiver<V, Stream>()(v, stream)) {
                    return false;
                }
                mp.insert({ std::move(k), std::move(v) });
            }
            return true;
        } else {
            static_assert(std::is_base_of_v<LoongArchiveOutputStream, Stream>);
        }
    }
};

template <class T, class Stream>
bool Serialize(T& t, Stream& s)
{
    return LoongArchiver<T, Stream>()(t, s);
}

}