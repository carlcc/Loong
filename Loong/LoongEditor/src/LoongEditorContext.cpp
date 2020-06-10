//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#include "LoongEditorContext.h"
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
    UniformBlock ub {};
    uniformBuffer_->BufferData(&ub, 1, Resource::LoongGpuBufferUsage::kStreamDraw);
    uniformBuffer_->SetBindingPoint(0, sizeof(UniformBlock));
    defaultMaterial_ = Resource::LoongResourceManager::GetMaterial("Materials/Default.lgmtl");

    renderer_.reset(new Renderer::LoongRenderer);
}

}