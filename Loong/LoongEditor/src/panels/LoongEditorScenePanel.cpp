//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//
#include <glad/glad.h>

#include "../LoongEditorContext.h"
#include "LoongApp/LoongApp.h"
#include "LoongApp/LoongInput.h"
#include "LoongCore/render/LoongRenderPassIdPass.h"
#include "LoongCore/render/LoongRenderPassScenePass.h"
#include "LoongCore/scene/LoongActor.h"
#include "LoongCore/scene/components/LoongCCamera.h"
#include "LoongEditorScenePanel.h"
#include "LoongRenderer/LoongRenderer.h"
#include "LoongResource/LoongFrameBuffer.h"

namespace Loong::Editor {

LoongEditorScenePanel::LoongEditorScenePanel(LoongEditor* editor, const std::string& name, bool opened, const LoongEditorPanelConfig& cfg)
    : LoongEditorRenderPanel(editor, name, opened, cfg)
{
    idPass_ = std::make_shared<Core::LoongRenderPassIdPass>();
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

    auto scene = GetEditorContext().GetCurrentScene();
    if (scene == nullptr) {
        return;
    }
    auto& camera = *cameraActor_->GetComponent<Core::LoongCCamera>();

    auto& renderer = GetEditorContext().GetRenderer();

    auto& inputManager = GetApp().GetInputManager();
    auto& mousePos = inputManager.GetMousePosition();
    if (inputManager.IsMouseButtonPressEvent(Loong::App::LoongMouseButton::kButtonLeft) && Math::IsBetween(mousePos, viewportMin_, viewportMax_)) {
        // Render selecting
        auto* frameBuffer = idPass_->GetFrameBuffer().get();
        frameBuffer->Resize(viewportWidth_, viewportHeight_);

        frameBuffer->Bind();

        glViewport(0, 0, viewportWidth_, viewportHeight_);
        glClearColor(0.F, 0.F, 0.F, 0.F);
        renderer.Clear(true, true, true);
        RenderSceneForCamera(*scene, camera, *idPass_);

        auto mouseX = mousePos.x - viewportMin_.x;
        auto mouseY = viewportHeight_ - (mousePos.y - viewportMin_.y); // Flip upside down, since the image is also upside down.

        uint8_t pixel[4] { 0 };
        glReadPixels(static_cast<int>(mouseX), static_cast<int>(mouseY), 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixel);
        uint32_t actorId = (pixel[0] << 24u) | (pixel[1] << 16u) | (pixel[2] << 8u) | (pixel[3] << 0u);

        GetEditorContext().SetCurrentSelectedActor(scene->GetChildById(actorId));
        frameBuffer->Unbind();
    }

    {
        // Render the scene
        GetFrameBuffer()->Bind();

        glViewport(0, 0, viewportWidth_, viewportHeight_);
        renderer.Clear(camera.GetCamera(), true, true, true);
        RenderSceneForCamera(*scene, camera, *scenePass_);

        GetFrameBuffer()->Unbind();
    }
}

}
