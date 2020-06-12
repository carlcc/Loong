//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#pragma once

#include <string>

namespace Loong::Foundation {

class LoongPathUtils {
public:
    LoongPathUtils() = delete;

    static std::string Normalize(std::string_view path);

    static std::string GetParent(std::string_view path);

    static std::string_view GetFileName(std::string_view path);

    // The input is pure name, not path
    // The return value contains dot (.),
    // e.g. Input: "a.tar.bz2" returns ".bz2"
    static std::string_view GetFileExtension(std::string_view fileName);
};

}