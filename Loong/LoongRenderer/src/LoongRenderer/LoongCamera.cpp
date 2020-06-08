//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//
#include "LoongRenderer/LoongCamera.h"

namespace Loong::Renderer {

void LoongCamera::UpdateMatrices(uint32_t viewportWidth, uint32_t viewportHeight, const Math::Vector3& position, const Math::Quat& rotation)
{
    UpdateProjectionMatrix(viewportWidth, viewportHeight);
    UpdateViewMatrix(position, rotation);
    UpdateFrustum(viewMatrix_, projectionMatrix_);
}

void LoongCamera::UpdateProjectionMatrix(uint32_t viewportWidth, uint32_t viewportHeight)
{
    // TODO: alternative ortho projection
    projectionMatrix_ = Math::Perspective(fov_, float(viewportWidth), float(viewportHeight), near_, far_);
}

void LoongCamera::UpdateViewMatrix(const Math::Vector3& position, const Math::Quat& rotation)
{
    auto up = rotation * Math::kUp;
    auto forward = rotation * Math::kForward;
    viewMatrix_ = Math::LookAt(position, position + forward, up);
}

void LoongCamera::UpdateFrustum(const Math::Matrix4& view, const Math::Matrix4& projection)
{
    frustum_.Reset(projection * view);
}

}