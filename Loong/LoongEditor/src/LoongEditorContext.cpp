//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#include "LoongEditorContext.h"

#include "LoongCore/render/LoongRenderPass.h"
#include "LoongFoundation/LoongPathUtils.h"
#include "LoongRenderer/LoongRenderer.h"
#include "LoongResource/LoongGpuBuffer.h"
#include "LoongResource/LoongResourceManager.h"
#include <memory>

namespace Loong::Editor {

LoongEditorContext::LoongEditorContext(const std::string& projectFile)
{
    projectFile_ = Foundation::LoongPathUtils::Normalize(projectFile);
    projectDir_ = Foundation::LoongPathUtils::GetParent(projectFile_);
    {
        basicUniformBuffer_ = std::make_shared<Resource::LoongUniformBuffer>();
        Core::LoongRenderPass::BasicUBO ub {};
        basicUniformBuffer_->BufferData(&ub, 1, Resource::LoongGpuBufferUsage::kStreamDraw);
        basicUniformBuffer_->SetBindingPoint(0, sizeof(ub));
    }
    {
        lightUniformBuffer_ = std::make_shared<Resource::LoongUniformBuffer>();
        Core::LoongRenderPass::LightUBO lub {};
        lightUniformBuffer_->BufferData(&lub, 1, Resource::LoongGpuBufferUsage::kStreamDraw);
        lightUniformBuffer_->SetBindingPoint(1, sizeof(lub));
    }

    defaultMaterial_ = Resource::LoongResourceManager::GetMaterial("/Materials/Default.lgmtl");
    renderer_ = std::make_unique<Renderer::LoongRenderer>();
}

void LoongEditorContext::SetCurrentScene(std::shared_ptr<Core::LoongScene> scene)
{
    if (currentScene_ != scene) {
        currentSelectedActor_ = nullptr;
        currentScene_ = std::move(scene);
    }
}

}