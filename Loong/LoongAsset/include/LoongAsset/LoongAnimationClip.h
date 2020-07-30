//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//
#pragma once

#include "LoongFoundation/LoongMath.h"
#include <string>
#include <vector>

namespace Loong::Asset {

class LoongAnimationClip {
public:
    struct Channel {
        std::string nodeName {};
        std::vector<float> positionKeyTimes {};
        std::vector<Math::Vector3> positionKeys {};
        std::vector<float> rotationKeyTimes {};
        std::vector<Math::Quat> rotationKeys {};
        std::vector<float> scaleKeyTimes {};
        std::vector<Math::Vector3> scaleKeys {};

        template <class Archive>
        bool Serialize(Archive& archive)
        {
            return archive(nodeName, positionKeyTimes, positionKeys, rotationKeyTimes,
                rotationKeys, scaleKeyTimes, scaleKeys);
        }
    };

    template <class Archive>
    bool Serialize(Archive& archive) { return archive(name, duration, ticksPerSecond, channels); }

    std::string name {};
    float duration { 0.0F };
    float ticksPerSecond { 0.0F };
    std::vector<Channel> channels {};
};

} // namespace Loong::Asset
