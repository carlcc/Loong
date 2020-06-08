//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#include "LoongFoundation/LoongMath.h"
#include <algorithm>

namespace Loong::Math {

AABB AABB::Transformed(const Math::Matrix4& transform) const
{
    Math::Vector4 corners[8] = {
        transform * Math::Vector4 { min.x, min.y, min.z, 1.0F },
        transform * Math::Vector4 { min.x, min.y, max.z, 1.0F },
        transform * Math::Vector4 { min.x, max.y, min.z, 1.0F },
        transform * Math::Vector4 { min.x, max.y, max.z, 1.0F },
        transform * Math::Vector4 { max.x, min.y, min.z, 1.0F },
        transform * Math::Vector4 { max.x, min.y, max.z, 1.0F },
        transform * Math::Vector4 { max.x, max.y, min.z, 1.0F },
        transform * Math::Vector4 { max.x, max.y, max.z, 1.0F },
    };
    AABB result { corners[0], corners[0] };
    for (int i = 1; i < 8; ++i) {
        auto& corner = corners[i];
        result.min.x = std::min(result.min.x, corner.x);
        result.min.y = std::min(result.min.y, corner.y);
        result.min.z = std::min(result.min.z, corner.z);
        result.max.x = std::max(result.max.x, corner.x);
        result.max.y = std::max(result.max.y, corner.y);
        result.max.z = std::max(result.max.z, corner.z);
    }
    return result;
}

}