//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#pragma once

#include "LoongCore/scene/LoongScene.h"
#include "LoongResource/LoongGpuBuffer.h"
#include "LoongResource/LoongPipelineFixedState.h"
#include <memory>

namespace Loong::Resource {
class LoongShader;
class LoongFrameBuffer;
}

namespace Loong::Renderer {
class LoongRenderer;
}

namespace Loong::Core {

class LoongScene;
class LoongCCamera;

class LoongRenderPass {
public:
    struct BasicUBO {
        Math::Matrix4 ub_Model;
        Math::Matrix4 ub_View;
        Math::Matrix4 ub_Projection;
        Math::Vector3 ub_ViewPos;
        float ub_Time;
    };
    struct Light {
        Math::Vector3 pos;
        float lightType;
        Math::Vector3 dir;
        float falloffRadius;
        Math::Vector3 color;
        float intencity;
        float innerAngle;
        float outerAngle;
        float padding1_[2];
    };

    static const int kMaxLightCount = 32;
    struct LightUBO {
        float ub_lightsCount;
        float padding1_[3]; // align to vec4
        Light ub_lights[kMaxLightCount];
    };

    struct Context {
        Renderer::LoongRenderer* renderer { nullptr };
        Resource::LoongUniformBuffer* basicUniforms { nullptr }; // Corresponding to UniformUBO struct
        Resource::LoongUniformBuffer* lightUniforms { nullptr }; // Corresponding to LightUBO struct
        LoongScene* scene { nullptr };
        LoongCCamera* camera { nullptr };
    };

    LoongRenderPass();
    virtual ~LoongRenderPass() = default;

    std::shared_ptr<Resource::LoongFrameBuffer> GetFrameBuffer() const { return frameBuffer_; }
    virtual void Render(const Context& context) = 0;

protected:
    std::shared_ptr<Resource::LoongFrameBuffer> frameBuffer_ {};
    Resource::LoongPipelineFixedState renderState_ {};
};

}