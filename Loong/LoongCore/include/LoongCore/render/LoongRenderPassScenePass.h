//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#pragma once

#include "LoongCore/render/LoongRenderPass.h"
#include "LoongFoundation/LoongMath.h"

namespace Loong::Core {

class LoongRenderPassScenePass : public LoongRenderPass {
public:
    void Render(Renderer::LoongRenderer& renderer, Resource::LoongUniformBuffer& basicUniforms, LoongScene& scene, LoongCCamera& camera) override;

    void SetDefaultMaterial(const std::shared_ptr<Resource::LoongMaterial>& mat) { defaultMaterial_ = mat; }

protected:
    std::shared_ptr<Resource::LoongMaterial> defaultMaterial_ { nullptr };
};

}