//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#pragma once

#include "LoongCore/scene/LoongComponent.h"
#include "LoongRenderer/LoongLight.h"

namespace Loong::Core {

class LoongCLight : public LoongComponent {
public:
    using Type = Renderer::LoongLight::Type;

    explicit LoongCLight(LoongActor* owner)
        : LoongComponent(owner)
    {
    }

    std::string GetName() override { return "Light"; }

    Type GetType() const { return light_.type; }

    void SetType(Type type) { light_.type = type; }

    const Math::Vector3& GetColor() const { return light_.color; }

    void SetColor(const Math::Vector3& color) { light_.color = color; }

    float GetIntensity() const { return light_.intensity; }

    void SetIntensity(float intensity)
    {
        light_.intensity = Math::Max(intensity, 0.0F);
    }

    float GetFalloffRadius() const { return light_.falloffRadius; }

    void SetFalloffRadius(float radius)
    {
        light_.falloffRadius = Math::Max(radius, 0.0F);
    }

    float GetInnerAngle() const { return light_.innerAngle; }

    void SetInnerAngle(float angle)
    {
        light_.innerAngle = Math::Clamp(angle, 0.0F, GetOuterAngle());
    }

    float GetOuterAngle() const { return light_.outerAngle; }

    void SetOuterAngle(float angle)
    {
        light_.outerAngle = Math::Clamp(angle, GetInnerAngle(), float(Math::Pi));
    }

protected:
    Renderer::LoongLight light_;
};

}