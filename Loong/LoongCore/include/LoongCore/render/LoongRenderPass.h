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
    virtual void Render(Renderer::LoongRenderer& renderer, LoongScene& scene, LoongCCamera& camera) = 0;

protected:
    std::shared_ptr<Resource::LoongFrameBuffer> frameBuffer_ {};
    std::shared_ptr<Resource::LoongUniformBuffer> uniformBuffer_ {};
    Resource::LoongPipelineFixedState renderState_ {};
};

}