//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#pragma once

#include "LoongCore/render/LoongRenderPass.h"
#include "LoongFoundation/LoongMath.h"

namespace Loong::Core {

class LoongRenderPassScenePass : public LoongRenderPass {
public:
    struct UniformBlock {
        Math::Matrix4 ub_Model;
        Math::Matrix4 ub_View;
        Math::Matrix4 ub_Projection;
        Math::Vector3 ub_ViewPos;
        float ub_Time;
    };

    void Render(Renderer::LoongRenderer& renderer, LoongScene& scene, LoongCCamera& camera) override;

protected:
    std::shared_ptr<Resource::LoongMaterial> defaultMaterial_ { nullptr };
};

}