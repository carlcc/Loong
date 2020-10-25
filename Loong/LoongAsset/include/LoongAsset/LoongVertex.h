//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#pragma once

#include "LoongFoundation/LoongMath.h"

namespace Loong::Asset {

struct LoongVertex {
    Math::Vector3 position;
    Math::Vector2 uv0;
    Math::Vector2 uv1;
    Math::Vector3 normal;
    Math::Vector3 tangent;
    Math::Vector3 bitangent;
};

static_assert(sizeof(LoongVertex) == sizeof(float) * 16);

}