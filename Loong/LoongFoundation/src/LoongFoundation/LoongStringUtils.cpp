//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#pragma once

#include "LoongFoundation/LoongStringUtils.h"
#include <string>
#include <vector>

namespace Loong::Foundation {

std::string LoongStringUtils::ReplaceAll(const std::string_view str, const std::string_view from, const std::string_view to)
{
    std::vector<std::string_view> subStrs = Split(str, from);
    return Join(subStrs, to);
}

}