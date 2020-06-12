//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#include "LoongFoundation/LoongPathUtils.h"
#include "LoongFoundation/LoongStringUtils.h"
#include <cassert>
#include <string>
#include <string_view>
#include <vector>

namespace Loong::Foundation {

bool IsSeparatorInternal(char c)
{
    return c == '/' || c == '\\';
}

const char kPathSeparator = '/';

void NormalizeSeparatorInternal(std::string& path)
{
    uint32_t i = 0, j = 0;
    while (j < path.length()) {
        if (IsSeparatorInternal(path[j])) {
            path[i] = kPathSeparator;
            ++i;
            ++j;
            // Skip the rest continuous separators
            while (j < path.length() && IsSeparatorInternal(path[j])) {
                ++j;
            }
        } else {
            path[i] = path[j];
            ++i;
            ++j;
        }
    }
    assert(i > 0);
    if (path[path.size() - 1] == kPathSeparator)
        --i;
    path.resize(i);
}

std::string RemoveDotDotInternal(std::string_view path)
{
    std::vector<std::string_view> paths = LoongStringUtils::Split(path, kPathSeparator);
    std::vector<const std::string_view*> stack;
    const std::string_view kDotDot = "..";
    const std::string_view kDot = ".";

    for (auto& sv : paths) {
        if (sv == kDotDot) {
            if (!stack.empty() && *stack[stack.size() - 1] != kDotDot) {
                stack.pop_back();
            } else {
                stack.push_back(&kDotDot);
            }
        } else if (sv == kDot) {
        } else {
            stack.push_back(&sv);
        }
    }

    std::string result;
    if (!stack.empty()) {
        auto it = stack.begin();
        result += **it;
        for (++it; it < stack.end(); ++it) {
            result += kPathSeparator;
            result += **it;
        }
    }
    return result;
}

bool IsAbsolutePath(const std::string_view& path)
{
#ifdef WIN32
    return path.length() >= 3 && std::isalpha(path[0]) && path[1] == ':' && IsSeparatorInternal(path[2]);
#else
    return path.length() >= 1 && IsSeparatorInternal(path[0]);
#endif
}

std::string Foundation::LoongPathUtils::Normalize(std::string_view path)
{
    if (path.empty()) {
        return "";
    }

    bool isAbsolute = IsAbsolutePath(path);
    std::string p(path.data(), path.length());

    NormalizeSeparatorInternal(p);

    if (isAbsolute) {
#ifdef WIN32
        return p.substr(0, 2) + kPathSeparator + RemoveDotDotInternal(p.substr(3));
#else
        return kPathSeparator + (p.empty() ? std::string() : RemoveDotDotInternal(p.substr(1)));
#endif
    } else {
        std::string result = RemoveDotDotInternal(p);
        if (result.empty()) {
            result += ".";
        }
        return result;
    }
}

std::string LoongPathUtils::GetParent(std::string_view path)
{
    std::string p;
    p.reserve((3 + path.length()));
    p += path;
    p += "/..";
    return Normalize(p);
}

std::string_view LoongPathUtils::GetFileName(std::string_view path)
{
    auto index1 = path.rfind('/');
    auto index2 = path.rfind('\\');
    if (index1 == std::string_view::npos) {
        if (index2 == std::string_view::npos) {
            return path;
        } else {
            return path.substr(index2 + 1);
        }
    } else {
        if (index2 == std::string_view::npos) {
            return path.substr(index1 + 1);
        } else {
            return path.substr((index1 > index2 ? index1 : index2) + 1);
        }
    }
}

std::string_view LoongPathUtils::GetFileExtension(std::string_view fileName)
{
    auto index = fileName.rfind('.');
    if (index == std::string_view::npos) {
        return fileName;
    }
    return fileName.substr(index);
}

}
