//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//
#pragma once

#include "LoongFoundation/LoongMath.h"
#include <string>
#include <vector>

namespace Loong::Renderer {

class LoongLight {
public:
    enum Type : int {
        kTypeDirectional,
        kTypePoint,
        kTypeSpot,
    };

    const static std::vector<std::string> kTypeNames;

    static std::string LightTypeToString(Type type);

    static Type StringToLightType(const std::string& type);

public:
    Type type { Type::kTypeDirectional };
    Math::Vector3 color { 1.0F, 1.0F, 1.0F };
    float intensity = 1.0F;

    // for point light and spot light
    float falloffRadius { 20.0F };

    // for spot light
    float innerAngle { 0.7F };
    float outerAngle { 1.2F };
};

}