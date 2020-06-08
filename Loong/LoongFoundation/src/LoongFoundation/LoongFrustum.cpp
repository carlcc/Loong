//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#include "LoongFoundation/LoongFrustum.h"

namespace Loong::Foundation {

void Frustum::Reset(Math::Matrix4 projection)
{
    projection = Math::Transpose(projection);
    planes_[Planes::kLeft] = projection[3] + projection[0];
    planes_[Planes::kRight] = projection[3] - projection[0];
    planes_[Planes::kBottom] = projection[3] + projection[1];
    planes_[Planes::kTop] = projection[3] - projection[1];
    planes_[Planes::kNear] = projection[3] + projection[2];
    planes_[Planes::kFar] = projection[3] - projection[2];

    Math::Vector3 crosses[Planes::kCombinations] = {
        Math::Cross(Math::Vector3(planes_[Planes::kLeft]), Math::Vector3(planes_[Planes::kRight])),
        Math::Cross(Math::Vector3(planes_[Planes::kLeft]), Math::Vector3(planes_[Planes::kBottom])),
        Math::Cross(Math::Vector3(planes_[Planes::kLeft]), Math::Vector3(planes_[Planes::kTop])),
        Math::Cross(Math::Vector3(planes_[Planes::kLeft]), Math::Vector3(planes_[Planes::kNear])),
        Math::Cross(Math::Vector3(planes_[Planes::kLeft]), Math::Vector3(planes_[Planes::kFar])),
        Math::Cross(Math::Vector3(planes_[Planes::kRight]), Math::Vector3(planes_[Planes::kBottom])),
        Math::Cross(Math::Vector3(planes_[Planes::kRight]), Math::Vector3(planes_[Planes::kTop])),
        Math::Cross(Math::Vector3(planes_[Planes::kRight]), Math::Vector3(planes_[Planes::kNear])),
        Math::Cross(Math::Vector3(planes_[Planes::kRight]), Math::Vector3(planes_[Planes::kFar])),
        Math::Cross(Math::Vector3(planes_[Planes::kBottom]), Math::Vector3(planes_[Planes::kTop])),
        Math::Cross(Math::Vector3(planes_[Planes::kBottom]), Math::Vector3(planes_[Planes::kNear])),
        Math::Cross(Math::Vector3(planes_[Planes::kBottom]), Math::Vector3(planes_[Planes::kFar])),
        Math::Cross(Math::Vector3(planes_[Planes::kTop]), Math::Vector3(planes_[Planes::kNear])),
        Math::Cross(Math::Vector3(planes_[Planes::kTop]), Math::Vector3(planes_[Planes::kFar])),
        Math::Cross(Math::Vector3(planes_[Planes::kNear]), Math::Vector3(planes_[Planes::kFar])),
    };
    points_[0] = Intersection<Planes ::kLeft, Planes ::kBottom, Planes ::kNear>(crosses);
    points_[1] = Intersection<Planes ::kLeft, Planes ::kTop, Planes ::kNear>(crosses);
    points_[2] = Intersection<Planes ::kRight, Planes ::kBottom, Planes ::kNear>(crosses);
    points_[3] = Intersection<Planes ::kRight, Planes ::kTop, Planes ::kNear>(crosses);
    points_[4] = Intersection<Planes ::kLeft, Planes ::kBottom, Planes ::kFar>(crosses);
    points_[5] = Intersection<Planes ::kLeft, Planes ::kTop, Planes ::kFar>(crosses);
    points_[6] = Intersection<Planes ::kRight, Planes ::kBottom, Planes ::kFar>(crosses);
    points_[7] = Intersection<Planes ::kRight, Planes ::kTop, Planes ::kFar>(crosses);
}

// http://iquilezles.org/www/articles/frustumcorrect/frustumcorrect.htm
bool Frustum::IsBoxVisible(const Math::AABB& aabb) const
{
    // clang-format off
    // check box outside/inside of frustum
    for (auto plane : planes_)
    {
        if ((Math::Dot(plane, Math::Vector4(aabb.min.x, aabb.min.y, aabb.min.z, 1.0F)) < 0.0F) &&
            (Math::Dot(plane, Math::Vector4(aabb.max.x, aabb.min.y, aabb.min.z, 1.0F)) < 0.0F) &&
            (Math::Dot(plane, Math::Vector4(aabb.min.x, aabb.max.y, aabb.min.z, 1.0F)) < 0.0F) &&
            (Math::Dot(plane, Math::Vector4(aabb.max.x, aabb.max.y, aabb.min.z, 1.0F)) < 0.0F) &&
            (Math::Dot(plane, Math::Vector4(aabb.min.x, aabb.min.y, aabb.max.z, 1.0F)) < 0.0F) &&
            (Math::Dot(plane, Math::Vector4(aabb.max.x, aabb.min.y, aabb.max.z, 1.0F)) < 0.0F) &&
            (Math::Dot(plane, Math::Vector4(aabb.min.x, aabb.max.y, aabb.max.z, 1.0F)) < 0.0F) &&
            (Math::Dot(plane, Math::Vector4(aabb.max.x, aabb.max.y, aabb.max.z, 1.0F)) < 0.0F))
        {
            return false;
        }
    }

    // check frustum outside/inside box
    int out;
    out = 0; for (auto point : points_) out += ((point.x > aabb.max.x) ? 1 : 0); if (out == 8) return false;
    out = 0; for (auto point : points_) out += ((point.x < aabb.min.x) ? 1 : 0); if (out == 8) return false;
    out = 0; for (auto point : points_) out += ((point.y > aabb.max.y) ? 1 : 0); if (out == 8) return false;
    out = 0; for (auto point : points_) out += ((point.y < aabb.min.y) ? 1 : 0); if (out == 8) return false;
    out = 0; for (auto point : points_) out += ((point.z > aabb.max.z) ? 1 : 0); if (out == 8) return false;
    out = 0; for (auto point : points_) out += ((point.z < aabb.min.z) ? 1 : 0); if (out == 8) return false;
    // clang-format on

    return true;
}

bool Frustum::IsSphereVisible(const Math::Vector3& center, float radius) const
{
    for (auto plane : planes_) {
        if ((Math::Dot(plane, Math::Vector4(center, 1.0F)) < -radius)) {
            return false;
        }
    }
    return true;
}

}