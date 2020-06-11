//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//
#include <glad/glad.h>

#include "../LoongEditorContext.h"
#include "LoongCore/scene/LoongActor.h"
#include "LoongCore/scene/components/LoongCCamera.h"
#include "LoongEditorScenePanel.h"
#include "LoongRenderer/LoongRenderer.h"
#include "LoongResource/LoongFrameBuffer.h"

namespace Loong::Editor {

LoongEditorScenePanel::LoongEditorScenePanel(LoongEditor* editor, const std::string& name, bool opened, const LoongEditorPanelConfig& cfg)
    : LoongEditorRenderPanel(editor, name, opened, cfg)
{
}

void LoongEditorScenePanel::UpdateImpl(const Foundation::LoongClock& clock)
{
    LoongEditorRenderPanel::UpdateImpl(clock);
}

void LoongEditorScenePanel::Render(const Foundation::LoongClock& clock)
{
    if (!IsVisible() || viewportWidth_ <= 0 || viewportHeight_ <= 0) {
        return;
    }

    frameBuffer_->Bind();
    glViewport(0, 0, viewportWidth_, viewportHeight_);
    auto& camera = *cameraActor_->GetComponent<Core::LoongCCamera>();
    GetEditorContext().GetRenderer().Clear(camera.GetCamera(), true, true, true);
    if (auto scene = GetEditorContext().GetCurrentScene(); scene != nullptr) {
        RenderSceneForCamera(*scene, camera);
    }
    frameBuffer_->Unbind();
}

}
