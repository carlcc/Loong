//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#include "LoongEditorContext.h"
#include "LoongCore/render/LoongRenderPass.h"
#include "LoongFoundation/LoongPathUtils.h"
#include "LoongRenderer/LoongRenderer.h"
#include "LoongResource/LoongGpuBuffer.h"
#include "LoongResource/LoongResourceManager.h"

namespace Loong::Editor {

LoongEditorContext::LoongEditorContext(const std::string& projectFile)
{
    projectFile_ = Foundation::LoongPathUtils::Normalize(projectFile);
    projectDir_ = Foundation::LoongPathUtils::GetParent(projectFile_);

    uniformBuffer_ = std::make_shared<Resource::LoongUniformBuffer>();
    Core::LoongRenderPass::UniformBlock ub {};
    uniformBuffer_->BufferData(&ub, 1, Resource::LoongGpuBufferUsage::kStreamDraw);
    uniformBuffer_->SetBindingPoint(0, sizeof(ub));
    defaultMaterial_ = Resource::LoongResourceManager::GetMaterial("Materials/Default.lgmtl");

    renderer_.reset(new Renderer::LoongRenderer);
}

void LoongEditorContext::SetCurrentScene(std::shared_ptr<Core::LoongScene> scene)
{
    if (currentScene_ != scene) {
        currentSelectedActor_ = nullptr;
        currentScene_ = std::move(scene);
    }
}

}