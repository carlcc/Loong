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
#include "LoongCore/render/LoongRenderPass.h"
#include "LoongCore/render/LoongRenderPassScenePass.h"
#include "LoongCore/render/LoongRenderPipeline.h"
#include "LoongCore/scene/LoongScene.h"
#include "LoongCore/scene/components/LoongCCamera.h"
#include "LoongResource/LoongFrameBuffer.h"
#include <imgui.h>
#include <iostream>

namespace Loong::Editor {

LoongEditorRenderPanel::LoongEditorRenderPanel(LoongEditor* editor, const std::string& name, bool opened, const LoongEditorPanelConfig& cfg)
    : LoongEditorPanel(editor, name, opened, cfg)
{
    cameraActor_ = Core::LoongScene::CreateActor("SceneViewCamera");
    cameraActor_->AddComponent<Core::LoongCCamera>();
    cameraActor_->AddComponent<LoongEditorSceneCameraController>(editor);

    scenePass_ = std::make_shared<Core::LoongRenderPassScenePass>();
    scenePass_->SetDefaultMaterial(GetEditorContext().GetDefaultMaterial());
}

std::shared_ptr<Resource::LoongFrameBuffer> LoongEditorRenderPanel::GetFrameBuffer() const
{
    return scenePass_->GetFrameBuffer();
}

void LoongEditorRenderPanel::UpdateImpl(const Foundation::LoongClock& clock)
{
    ImGuiUtils::ScopedId scopedId(this);

    ImGui::GetItemRectSize();
    auto min = ImGuiUtils::ToVector2(ImGui::GetWindowContentRegionMin());
    auto max = ImGuiUtils::ToVector2(ImGui::GetWindowContentRegionMax());
    auto viewportSize = max - min;
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
    assert(scenePass_ != nullptr && scenePass_->GetFrameBuffer() != nullptr);

    auto frameBuffer = GetFrameBuffer();
    frameBuffer->Resize(viewportWidth_, viewportHeight_);
    ImGui::Image((void*)(intptr_t)frameBuffer->GetColorAttachments()[0]->GetId(), ImGuiUtils::ToImVec(viewportSize), ImVec2 { 0, 1 }, ImVec2 { 1, 0 });

    // Get the viewport's coordinate
    viewportMin_ = ImGuiUtils::ToVector2(ImGui::GetItemRectMin());
    viewportMax_ = ImGuiUtils::ToVector2(ImGui::GetItemRectMax());

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

void LoongEditorRenderPanel::RenderSceneForCamera(Core::LoongScene& scene, Core::LoongCCamera& camera, Core::LoongRenderPass& renderPass)
{
    auto cameraPos = camera.GetOwner()->GetTransform().GetWorldPosition();
    auto cameraRot = camera.GetOwner()->GetTransform().GetWorldRotation();
    camera.GetCamera().UpdateMatrices(viewportWidth_, viewportHeight_, cameraPos, cameraRot);

    auto& context = GetEditorContext();
    renderPass.Render(context.GetRenderer(), *context.GetUniformBuffer(), scene, camera);
}

}