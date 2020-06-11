//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#include "LoongEditorRenderPanel.h"
#include "../LoongEditorContext.h"
#include "../utils/ImGuiUtils.h"
#include "../utils/LoongEditorSceneCameraController.h"
#include "LoongApp/LoongApp.h"
#include "LoongApp/LoongInput.h"
#include "LoongApp/LoongInputAction.h"
#include "LoongCore/scene/LoongScene.h"
#include "LoongCore/scene/components/LoongCCamera.h"
#include "LoongResource/LoongFrameBuffer.h"
#include <imgui.h>
#include <iostream>

namespace Loong::Editor {

LoongEditorRenderPanel::LoongEditorRenderPanel(LoongEditor* editor, const std::string& name, bool opened, const LoongEditorPanelConfig& cfg)
    : LoongEditorPanel(editor, name, opened, cfg)
{
    frameBuffer_ = std::make_shared<Resource::LoongFrameBuffer>();
    cameraActor_ = Core::LoongScene::CreateActor("SceneViewCamera");
    cameraActor_->AddComponent<Core::LoongCCamera>();
    cameraActor_->AddComponent<LoongEditorSceneCameraController>(editor);
}

void LoongEditorRenderPanel::UpdateImpl(const Foundation::LoongClock& clock)
{
    ImGuiUtils::ScopedId scopedId(this);

    ImGui::GetItemRectSize();
    ImVec2 vMin = ImGui::GetWindowContentRegionMin();
    ImVec2 vMax = ImGui::GetWindowContentRegionMax();
    ImVec2 viewportSize = { vMax.x - vMin.x, vMax.y - vMin.y };
    viewportWidth_ = int(viewportSize.x);
    viewportHeight_ = int(viewportSize.y);
    //    auto pos = ImGui::GetWindowPos();
    //    vMin.x += pos.x;
    //    vMax.x += pos.x;
    //    vMin.y += pos.y;
    //    vMax.y += pos.y;
    //    IntRect rect;
    //    rect.left_ = int(vMin.x);
    //    rect.top_ = int(vMin.y);
    //    rect.right_ = int(vMax.x);
    //    rect.bottom_ = int(vMax.y);

    // auto windowSize = ImGui::GetItemRectSize();
    assert(frameBuffer_ != nullptr);
    frameBuffer_->Resize(viewportWidth_, viewportHeight_);
    ImGui::Image((void*)(intptr_t)frameBuffer_->GetColorAttachments()[0]->GetId(), viewportSize, ImVec2 { 0, 1 }, ImVec2 { 1, 0 });

    // Update camera actor
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_None)) {

        const auto& editorInput = GetApp().GetInputManager();

        if (editorInput.IsMouseButtonPressed(App::LoongMouseButton::kButtonRight)) {
            GetApp().SetMouseMode(App::LoongApp::MouseMode::kDisabled);
            ImGui::GetIO().DisableMouseUpdate = true;
        } else {
            GetApp().SetMouseMode(App::LoongApp::MouseMode::kNormal);
            ImGui::GetIO().DisableMouseUpdate = false;
        }
        if (ImGui::GetIO().DisableMouseUpdate) {
            cameraActor_->OnUpdate(clock);
        }
    }
}

void LoongEditorRenderPanel::RenderSceneForCamera(Core::LoongScene& scene, Core::LoongCCamera& camera)
{
    auto cameraPos = camera.GetOwner()->GetTransform().GetWorldPosition();
    auto cameraRot = camera.GetOwner()->GetTransform().GetWorldRotation();
    camera.GetCamera().UpdateMatrices(viewportWidth_, viewportHeight_, cameraPos, cameraRot);

    LoongEditorContext::UniformBlock ub {};
    ub.ub_ViewPos = camera.GetOwner()->GetTransform().GetWorldPosition();
    ub.ub_Projection = camera.GetCamera().GetProjectionMatrix();
    ub.ub_View = camera.GetCamera().GetViewMatrix();

    auto& context = GetEditorContext();
    scene.Render(context.GetRenderer(), camera, context.GetDefaultMaterial().get(), [uniformBuffer = context.GetUniformBuffer().get(), &ub](const Math::Matrix4& modelMatrix) {
        ub.ub_Model = modelMatrix;
        uniformBuffer->SetSubData(&ub, 0);
    });
}

}