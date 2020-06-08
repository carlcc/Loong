//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#include "LoongRenderer/LoongLight.h"
#include <string>

namespace Loong::Renderer {

// clang-format off
const std::vector<std::string> LoongLight::kTypeNames { // NOLINT(cert-err58-cpp)
    "Directional",
    "Point",
    "Spot",
};
// clang-format on

std::string LoongLight::LightTypeToString(LoongLight::Type type)
{
    assert(int(type) < kTypeNames.size() && int(type) >= 0); // new light type?
    return kTypeNames[int(type)];
}

LoongLight::Type LoongLight::StringToLightType(const std::string& type)
{
    for (size_t i = 0; i < kTypeNames.size(); ++i) {
        if (kTypeNames[i] == type) {
            return LoongLight::Type(i);
        }
    }
    assert(false); // new light type?
    return LoongLight::Type::kTypeDirectional; // just used to disable warning thrown by gcc
}

}
