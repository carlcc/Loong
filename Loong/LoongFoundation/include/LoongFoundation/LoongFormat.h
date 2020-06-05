//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#pragma once
#include "LoongFoundation/fmt/format.h"

namespace Loong::Foundation {

template <class... T>
std::string Format(T... args)
{
    return fmt::format(std::forward<T>(args)...);
}

}