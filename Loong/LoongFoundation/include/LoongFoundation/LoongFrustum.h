//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#pragma once

#include "LoongFoundation/LoongMath.h"
#include "LoongFoundation/LoongTransform.h"

namespace Loong::Foundation {

// from https://gist.github.com/podgorskiy/e698d18879588ada9014768e3e82a644
class Frustum {
public:
    enum Planes {
        kLeft = 0,
        kRight,
        kBottom,
        kTop,
        kNear,
        kFar,
        kCount,
        kCombinations = kCount * (kCount - 1) / 2
    };

    void Reset(Math::Matrix4 projection);

    bool IsBoxVisible(const Math::AABB& aabb) const;

    bool IsSphereVisible(const Math::Vector3& center, float radius) const;

    const Math::Vector3* GetPoints() const { return points_; }

private:
    template <Planes i, Planes j>
    struct ij2k {
        enum { k = i * (9 - i) / 2 + j - 1 };
    };

    template <Frustum::Planes a, Frustum::Planes b, Frustum::Planes c>
    inline Math::Vector3 Intersection(const Math::Vector3* crosses) const
    {
        float D = Math::Dot(Math::Vector3(planes_[a]), crosses[ij2k<b, c>::k]);
        Math::Vector3 res = Math::Matrix3(crosses[ij2k<b, c>::k], -crosses[ij2k<a, c>::k], crosses[ij2k<a, b>::k]) * Math::Vector3(planes_[a].w, planes_[b].w, planes_[c].w);
        return res * (-1.0f / D);
    }

    Math::Vector4 planes_[6];
    Math::Vector3 points_[8];
};

}