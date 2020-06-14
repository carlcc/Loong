//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#include "LoongEditorGamePanel.h"
#include "../LoongEditor.h"
#include "../LoongEditorContext.h"
#include "LoongCore/render/LoongRenderPassScenePass.h"
#include "LoongCore/scene/LoongScene.h"
#include "LoongCore/scene/components/LoongCCamera.h"
#include "LoongRenderer/LoongRenderer.h"
#include "LoongResource/LoongFrameBuffer.h"

namespace Loong::Editor {

void LoongEditorGamePanel::Render(const Foundation::LoongClock& clock)
{
    if (!IsVisible() || !IsContentVisible() || viewportWidth_ <= 0 || viewportHeight_ <= 0) {
        return;
    }
    GetFrameBuffer()->Bind();
    if (auto scene = GetEditorContext().GetCurrentScene(); scene != nullptr) {
        auto& renderer = GetEditorContext().GetRenderer();

        if (auto camera = scene->GetFirstActiveCamera(); camera != nullptr) {
            glViewport(0, 0, viewportWidth_, viewportHeight_);
            renderer.Clear(camera->GetCamera());
            RenderSceneForCamera(*scene, *camera, *scenePass_);
        }
    }
    GetFrameBuffer()->Unbind();
}

}