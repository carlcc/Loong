//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#pragma once

#include "LoongCore/scene/LoongComponent.h"
#include "LoongRenderer/LoongCamera.h"

namespace Loong::Core {

class LoongCCamera final : public LoongComponent {
public:
    explicit LoongCCamera(LoongActor* owner);

    ~LoongCCamera() override;

    std::string GetName() override { return "Camera"; }

    void SetFov(float value)
    {
        camera_.SetFov(Math::Clamp(value, 0.0F, float(Math::Pi)));
    }

    void SetNear(float value)
    {
        camera_.SetNear(Math::Clamp(value, 0.01F, GetFar()));
    }

    void SetFar(float value)
    {
        camera_.SetFar(Math::Max(value, GetNear()));
    }

    void SetClearColor(const Math::Vector3& clearColor) { camera_.SetClearColor(clearColor); }

    float GetFov() const { return camera_.GetFov(); }

    float GetNear() const { return camera_.GetNear(); }

    float GetFar() const { return camera_.GetFar(); }

    const Math::Vector3& GetClearColor() const { return camera_.GetClearColor(); }

    Renderer::LoongCamera& GetCamera() { return camera_; }

private:
    Renderer::LoongCamera camera_;
};

}