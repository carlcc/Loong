//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//
#pragma once

#include "LoongFoundation/LoongFrustum.h"
#include "LoongFoundation/LoongMath.h"
#include <cstdint>

namespace Loong::Renderer {

class LoongCamera {
public:
    void UpdateMatrices(uint32_t viewportWidth, uint32_t viewportHeight, const Math::Vector3& position, const Math::Quat& rotation);

    float GetFov() const { return fov_; }

    float GetNear() const { return near_; }

    float GetFar() const { return far_; }

    const Math::Vector3& GetClearColor() const { return clearColor_; }

    const Math::Matrix4& GetProjectionMatrix() const { return projectionMatrix_; }

    const Math::Matrix4& GetViewMatrix() const { return viewMatrix_; }

    const Foundation::Frustum& GetFrustum() const { return frustum_; }

    // radian
    void SetFov(float value) { fov_ = value; }

    void SetNear(float value) { near_ = value; }

    void SetFar(float value) { far_ = value; }

    void SetClearColor(const Math::Vector3& clearColor) { clearColor_ = clearColor; }

private:
    void UpdateProjectionMatrix(uint32_t viewportWidth, uint32_t viewportHeight);

    void UpdateViewMatrix(const Math::Vector3& position, const Math::Quat& rotation);

    void UpdateFrustum(const Math::Matrix4& view, const Math::Matrix4& projection);

private:
    Foundation::Frustum frustum_ {};
    Math::Matrix4 viewMatrix_ {};
    Math::Matrix4 projectionMatrix_ {};

    float fov_ { Math::DegreeToRad(45.0F) };
    float near_ { 0.1F };
    float far_ { 1000.0F };

    Math::Vector3 clearColor_ {};
};

}
