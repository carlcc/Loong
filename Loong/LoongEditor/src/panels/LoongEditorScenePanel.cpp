//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//
#include <glad/glad.h>

#include "../LoongEditorContext.h"
#include "../utils/IconsFontAwesome5.h"
#include "../utils/ImGuiUtils.h"
#include "LoongApp/LoongInput.h"
#include "LoongApp/LoongWindow.h"
#include "LoongCore/render/LoongRenderPassIdPass.h"
#include "LoongCore/render/LoongRenderPassScenePass.h"
#include "LoongCore/scene/LoongActor.h"
#include "LoongCore/scene/components/LoongCCamera.h"
#include "LoongCore/scene/components/LoongCModelRenderer.h"
#include "LoongEditorScenePanel.h"
#include "LoongRenderer/LoongRenderer.h"
#include "LoongResource/LoongFrameBuffer.h"
#include "LoongResource/LoongResourceManager.h"
#include "LoongResource/LoongShader.h"
#include "LoongResource/loader/LoongTextureLoader.h"

#include <imgui.h>
// Put ImGuizmo after imgui
#include <ImGuizmo.h>

namespace Loong::Editor {

LoongEditorScenePanel::LoongEditorScenePanel(LoongEditor* editor, const std::string& name, bool opened, const LoongEditorPanelConfig& cfg)
    : LoongEditorRenderPanel(editor, name, opened, cfg)
{
    idPass_ = std::make_shared<Core::LoongRenderPassIdPass>();

    auto cameraMaterial = std::make_shared<Resource::LoongMaterial>();
    cameraMaterial->SetShaderByFile("/Shaders/unlit.glsl");
    uint8_t color[4] = { 0x40, 0x80, 0xFF, 0xFF };
    cameraMaterial->GetUniformsData()["u_DiffuseMap"] = Resource::LoongTextureLoader::CreateColor(color, true, nullptr);

    auto cameraModel = Resource::LoongResourceManager::GetModel("/Models/camera.lgmdl");

    scenePass_->SetCameraMaterial(cameraMaterial);
    scenePass_->SetCameraModel(cameraModel);
    scenePass_->SetRenderCamera(true);

    idPass_->SetCameraModel(cameraModel);

    wireframeShader_ = Resource::LoongResourceManager::GetShader("/Shaders/wireframe.glsl");
    wireframeShader_->Bind();
    static const Math::Vector4 kWireframeColor { 0.3F, 0.4F, 0.5F, 1.0F };
    wireframeShader_->SetUniformVec4("u_wireColor", kWireframeColor);
    wireframeShader_->Unbind();

    gizmo_.SetBoundCamera(cameraActor_->GetComponent<Core::LoongCCamera>());
}

void LoongEditorScenePanel::UpdateImpl(const Foundation::LoongClock& clock)
{
    LoongEditorRenderPanel::UpdateImpl(clock);
    UpdateButtons(clock);
    UpdateGizmo(clock);
    UpdateShortcuts(clock);
}

void LoongEditorScenePanel::UpdateShortcuts(const Foundation::LoongClock& clock)
{
    auto& inputManager = GetApp().GetInputManager();
    if (ImGui::IsWindowHovered() && !inputManager.IsMouseButtonPressed(App::LoongMouseButton::kButtonRight)) {
        if (inputManager.IsKeyPressEvent(App::LoongKeyCode::kKeyW)) {
            gizmo_.SetManipulateMode(LoongEditorGizmo::ManipulateMode::kTranslate);
        }
        if (inputManager.IsKeyPressEvent(App::LoongKeyCode::kKeyE)) {
            gizmo_.SetManipulateMode(LoongEditorGizmo::ManipulateMode::kRotate);
        }
        if (inputManager.IsKeyPressEvent(App::LoongKeyCode::kKeyR)) {
            gizmo_.SetManipulateMode(LoongEditorGizmo::ManipulateMode::kScale);
        }
    }
}

bool EditorToolbarButton(const char* text, const char* tooltip, bool active)
{
    const auto& style = ImGui::GetStyle();
    const ImU32 kToolButtonActiveColor = IM_COL32(0x98, 0x23, 0x00, 0xFF);
    if (active)
        ImGui::PushStyleColor(ImGuiCol_Button, kToolButtonActiveColor); //style.Colors[ImGuiCol_ButtonActive]);
    else
        ImGui::PushStyleColor(ImGuiCol_Button, style.Colors[ImGuiCol_Button]);
    bool result = ImGui::Button(text);
    ImGui::PopStyleColor();
    ImGui::SameLine(0, 0);
    if (ImGui::IsItemHovered() && tooltip)
        ImGui::SetTooltip("%s", tooltip);
    return result;
}

void LoongEditorScenePanel::UpdateButtons(const Foundation::LoongClock& clock)
{
    auto min = ImGuiUtils::ToVector2(ImGui::GetWindowContentRegionMin());
    isOverToolButton_ = false;
    ImGui::SetCursorPos({ min.x + 10.F, min.y + 10.F });
    if (EditorToolbarButton(ICON_FA_ARROWS_ALT "###Translate", "Translate", gizmo_.GetManipulateMode() == LoongEditorGizmo::ManipulateMode::kTranslate)) {
        gizmo_.SetManipulateMode(LoongEditorGizmo::ManipulateMode::kTranslate);
    }
    isOverToolButton_ |= ImGui::IsItemHovered();
    if (EditorToolbarButton(ICON_FA_SYNC "###Rotate", "Rotate", gizmo_.GetManipulateMode() == LoongEditorGizmo::ManipulateMode::kRotate)) {
        gizmo_.SetManipulateMode(LoongEditorGizmo::ManipulateMode::kRotate);
    }
    isOverToolButton_ |= ImGui::IsItemHovered();
    if (EditorToolbarButton(ICON_FA_EXPAND_ARROWS_ALT "###Scale", "Scale", gizmo_.GetManipulateMode() == LoongEditorGizmo::ManipulateMode::kScale)) {
        gizmo_.SetManipulateMode(LoongEditorGizmo::ManipulateMode::kScale);
        gizmo_.SetCoordinateMode(LoongEditorGizmo::CoordinateMode::kLocal); // Scale only supports local coordinate
    }
    isOverToolButton_ |= ImGui::IsItemHovered();

    ImGui::SameLine(0, 3.f);

    if (EditorToolbarButton(ICON_FA_GLOBE "###WorldSpace", "World Space", gizmo_.GetCoordinateMode() == LoongEditorGizmo::CoordinateMode::kWorld)) {
        if (gizmo_.GetManipulateMode() != LoongEditorGizmo::ManipulateMode::kScale) { // If gizmo mode is scaling, then do not response
            gizmo_.SetCoordinateMode(LoongEditorGizmo::CoordinateMode::kWorld);
        }
    }
    isOverToolButton_ |= ImGui::IsItemHovered();
    if (EditorToolbarButton(ICON_FA_BASEBALL_BALL "###LocalSpace", "Local Space", gizmo_.GetCoordinateMode() == LoongEditorGizmo::CoordinateMode::kLocal)) {
        gizmo_.SetCoordinateMode(LoongEditorGizmo::CoordinateMode::kLocal);
    }
    isOverToolButton_ |= ImGui::IsItemHovered();
}

void LoongEditorScenePanel::UpdateGizmo(const Foundation::LoongClock& clock)
{
    if (viewportHeight_ <= 0 || viewportWidth_ <= 0) {
        return;
    }
    gizmo_.SetDrawList();

    // We must to udpate camera's matrices first
    auto& cameraTransform = cameraActor_->GetTransform();
    auto cameraPos = cameraTransform.GetWorldPosition();
    auto cameraRot = cameraTransform.GetWorldRotation();
    auto& renderCamera = cameraActor_->GetComponent<Core::LoongCCamera>()->GetCamera();
    renderCamera.UpdateMatrices(viewportWidth_, viewportHeight_, cameraPos, cameraRot);

    if (auto* selectedActor = GetEditorContext().GetCurrentSelectedActor(); selectedActor != nullptr) {
        gizmo_.SetViewport(viewportMin_, { viewportWidth_, viewportHeight_ });
        gizmo_.Manipulate(selectedActor);
    }
    gizmo_.ViewManipulate(0.5, { viewportMax_.x - 128, viewportMin_.y }, { 128, 128 }, 0x10101010);
}

void LoongEditorScenePanel::Render(const Foundation::LoongClock& clock)
{
    if (!IsVisible() || !IsContentVisible() || viewportWidth_ <= 0 || viewportHeight_ <= 0) {
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
    if (IsFocused() && !isOverToolButton_
        && inputManager.IsMouseButtonReleaseEvent(Loong::App::LoongMouseButton::kButtonLeft)
        && Math::Distance(inputManager.GetMouseDownPosition(), mousePos) < 4.0F) {
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

        frameBuffer->Unbind();

        auto* clickedActor = scene->GetChildByIdRecursive(actorId);
        auto* oldSelectedActor = GetEditorContext().GetCurrentSelectedActor();

        GetEditorContext().SetCurrentSelectedActor(clickedActor);
    }

    {
        // Render the scene
        GetFrameBuffer()->Bind();

        glViewport(0, 0, viewportWidth_, viewportHeight_);
        renderer.Clear(camera.GetCamera(), true, true, true);
        RenderSceneForCamera(*scene, camera, *scenePass_);

        // Render the selected actor as wireframe
        if (auto* selectedActor = GetEditorContext().GetCurrentSelectedActor(); selectedActor != nullptr) {
            auto* modelRenderer = selectedActor->GetComponent<Core::LoongCModelRenderer>();
            if (modelRenderer != nullptr && modelRenderer->GetModel() != nullptr) {
                wireframeShader_->Bind();
                auto stateBackup = renderer.FetchGLState();
                auto newState = stateBackup;
                newState.SetDepthTestEnabled(false);
                newState.SetFaceCullEnabled(false);
                renderer.ApplyStateMask(newState);

                renderer.SetPolygonMode(Renderer::LoongRenderer::PolygonMode::kLine);

                Core::LoongRenderPass::BasicUBO ubo {};
                ubo.ub_Projection = camera.GetCamera().GetProjectionMatrix();
                ubo.ub_View = camera.GetCamera().GetViewMatrix();
                ubo.ub_ViewPos = camera.GetOwner()->GetTransform().GetWorldPosition();
                ubo.ub_Model = selectedActor->GetTransform().GetWorldTransformMatrix();
                GetEditorContext().GetBasicUniformBuffer()->SetSubData(&ubo, 0);
                for (auto* mesh : modelRenderer->GetModel()->GetMeshes()) {
                    renderer.Draw(*mesh);
                }

                // Restore the GL states and fill mode
                renderer.ApplyStateMask(stateBackup);
                renderer.SetPolygonMode(Renderer::LoongRenderer::PolygonMode::kFill);
                wireframeShader_->Unbind();
            }
        }

        GetFrameBuffer()->Unbind();
    }
}

}
