//
// Copyright (c) 2020 Carl Chen All rights reserved.
//

#pragma once

#include "LoongCore/render/LoongRenderPass.h"

namespace Loong::Core {

class LoongRenderPassIdPass : public LoongRenderPass {
public:
    LoongRenderPassIdPass();
    void Render(Renderer::LoongRenderer& renderer, Resource::LoongUniformBuffer& basicUniforms, LoongScene& scene, LoongCCamera& camera) override;

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
};

}
