//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#pragma once

#include "LoongFoundation/LoongMacros.h"
#include <string>

namespace Loong::Foundation {

class LoongPathUtils {
public:
    LoongPathUtils() = delete;

    LG_NODISCARD static std::string Normalize(std::string_view path);

    LG_NODISCARD static std::string GetParent(std::string_view path);

    LG_NODISCARD static std::string_view GetFileName(std::string_view path);

    // The input is pure name, not path
    // The return value contains dot (.),
    // e.g. Input: "a.tar.bz2" returns ".bz2"
    LG_NODISCARD static std::string_view GetFileExtension(std::string_view fileName);
};

}