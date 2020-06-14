//
// Copyright (c) 2020 Carl Chen All rights reserved.
//

#pragma once

#include "LoongCore/render/LoongRenderPass.h"

namespace Loong::Resource {
class LoongGpuModel;
}
namespace Loong::Core {

class LoongRenderPassIdPass : public LoongRenderPass {
public:
    LoongRenderPassIdPass();

    void Render(Renderer::LoongRenderer& renderer, Resource::LoongUniformBuffer& basicUniforms, LoongScene& scene, LoongCCamera& camera) override;

    void SetCameraModel(const std::shared_ptr<Resource::LoongGpuModel>& mdl) { cameraModel_ = mdl; }

    static Math::Vector4 ActorIdToColor(uint32_t actorId)
    {
        return {(float(actorId >> 24u & 0xFFu) / 255.0F),
                (float(actorId >> 16u & 0xFFu) / 255.0F),
                (float(actorId >> 8u & 0xFFu) / 255.0F),
                (float(actorId >> 0u & 0xFFu) / 255.0F),
        };
    }

private:
    Resource::LoongPipelineFixedState state_;
    std::shared_ptr<Resource::LoongShader> sceneIdShader_ { nullptr };
    std::shared_ptr<Resource::LoongGpuModel> cameraModel_ { nullptr };
};

}
