//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#pragma once

#include <string>

namespace Loong::Foundation {

class LoongPathUtils {
public:
    LoongPathUtils() = delete;

    static std::string Normalize(const std::string_view& path);

    static std::string GetParent(const std::string_view& path);
};

}