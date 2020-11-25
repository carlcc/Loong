//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#pragma once

#include "LoongFoundation/LoongMacros.h"
#include <algorithm>
#include <cctype>
#include <sstream>
#include <string>
#include <string_view>

namespace Loong::Foundation {

// clang-format off
template <class T> inline size_t StringLength(const T& t) { return t.length(); }
template <>        inline size_t StringLength<char>(const char& t) { return 1; }
// clang-format on

template <class Separator>
class StringSplitor {
public:
    template <class Container>
    LG_NODISCARD operator Container() const // NOLINT(google-explicit-constructor)
    {
        Container result;
        Split(result);
        return result;
    }

    template <class Container>
    void Split(Container& result) const
    {
        if (StringLength(separator_) == 0) {
            result.insert(result.end(), typename Container::value_type(str_));
            return;
        }
        size_t start = 0;
        auto index = str_.find(separator_);
        while (index != std::string_view::npos) {
            result.insert(result.end(), typename Container::value_type(str_.substr(start, index - start)));
            start = index + StringLength(separator_);
            index = str_.find(separator_, start);
        }
        result.insert(result.end(), typename Container::value_type(str_.substr(start)));
    }

    const std::string_view str_;
    std::conditional_t<
        std::is_same_v<std::remove_cv_t<Separator>, std::string_view> || !std::is_class_v<Separator>,
        const Separator,
        const Separator&>
        separator_;
};

class LoongStringUtils {
public:
    LoongStringUtils() = delete;

    template <class Str, class Separator>
    LG_NODISCARD static StringSplitor<Separator> Split(const Str& str, const Separator& separator)
    {
        return { str, separator };
    }

    template <class Container, class Str, class Separator>
    static void Split(Container& container, const Str& str, const Separator& separator)
    {
        StringSplitor<Separator> { str, separator }.Split(container);
    }

    template <class Str>
    LG_NODISCARD static StringSplitor<std::string_view> Split(const Str& str, const char* separator)
    {
        return { str, separator };
    }

    template <class Container, class Str>
    static void Split(Container& container, const Str& str, const char* separator)
    {
        StringSplitor<std::string_view> { str, separator }.Split(container);
    }

    LG_NODISCARD static bool StartsWith(std::string_view str, std::string_view sub)
    {
        return str.size() > sub.size() && str.compare(0, sub.size(), sub) == 0;
    }

    LG_NODISCARD static bool EndsWith(const std::string_view str, const std::string_view sub)
    {
        return str.size() > sub.size() && str.compare(str.size() - sub.size(), sub.size(), sub) == 0;
    }

    template <class Strings>
    LG_NODISCARD static std::string Join(const Strings& strs, std::string_view delimiter = ",")
    {
        auto it = strs.begin();
        std::stringstream ss;
        ss << *it;
        for (++it; it != strs.end(); ++it) {
            ss << delimiter;
            ss << *it;
        }
        return ss.str();
    }

    static void ToLower(std::string& s)
    {
        std::transform(std::begin(s), std::end(s), std::begin(s), [](char c) -> char { return (char)std::tolower(c); });
    }

    static void ToUpper(std::string& s)
    {
        std::transform(std::begin(s), std::end(s), std::begin(s), [](char c) -> char { return (char)std::toupper(c); });
    }

    template <class T>
    static T ParseTo(std::string_view s)
    {
        std::stringstream ss;
        ss << s;
        T t;
        ss >> t;
        return t;
    }

    static int Compare(std::string_view s1, std::string_view s2)
    {
        return s1.compare(s2);
    }

    static int CompareIgnoreCase(std::string_view s1, std::string_view s2)
    {
        auto n = std::min(s1.length(), s2.length());
        for (size_t i = 0; i < n; ++i) {
            auto ch1 = (uint8_t)std::tolower(s1[0]);
            auto ch2 = (uint8_t)std::tolower(s2[0]);
            if (ch1 < ch2) {
                return -1;
            } else if (ch1 > ch2) {
                return 1;
            }
        }
        if (s1.length() < s2.length()) {
            return -1;
        } else if (s1.length() > s2.length()) {
            return 1;
        }
        return 0;
    }

    static bool Equals(std::string_view s1, std::string_view s2)
    {
        return s1 == s2;
    }

    static bool EqualsIgnoreCase(std::string_view s1, std::string_view s2)
    {
        return s1.length() == s2.length() && CompareIgnoreCase(s1, s2) == 0;
    }
};

}