//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//
#include <glad/glad.h>

#include "../LoongEditorContext.h"
#include "../utils/IconsFontAwesome5.h"
#include "../utils/ImGuiUtils.h"
#include "LoongApp/LoongApp.h"
#include "LoongApp/LoongInput.h"
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
}

void LoongEditorScenePanel::UpdateImpl(const Foundation::LoongClock& clock)
{
    LoongEditorRenderPanel::UpdateImpl(clock);
    UpdateButtons(clock);
    UpdateGizmo(clock);
}

void LoongEditorScenePanel::UpdateButtons(const Foundation::LoongClock& clock)
{
    auto min = ImGuiUtils::ToVector2(ImGui::GetWindowContentRegionMin());
    ImGui::SetCursorPos({ min.x + 10.F, min.y + 10.F });
    ImGui::Button(ICON_FA_ARROWS_ALT "###Translate");
    ImGui::Button("你###Translate");ImGui::Text("Kanjis: \xe6\x97\xa5\xe6\x9c\xac\xe8\xaa\x9e (nihongo)");
}

void LoongEditorScenePanel::UpdateGizmo(const Foundation::LoongClock& clock)
{
    ImGuizmo::SetDrawlist();

    auto& cameraTransform = cameraActor_->GetTransform();
    auto cameraPos = cameraTransform.GetWorldPosition();
    auto cameraRot = cameraTransform.GetWorldRotation();
    auto& renderCamera = cameraActor_->GetComponent<Core::LoongCCamera>()->GetCamera();
    renderCamera.UpdateMatrices(viewportWidth_, viewportHeight_, cameraPos, cameraRot);
    auto cameraViewMatrix = renderCamera.GetViewMatrix();

    Math::Vector3 position {}, scale {};
    Math::Quat rotation {};
    if (auto* selectedActor = GetEditorContext().GetCurrentSelectedActor(); selectedActor != nullptr) {
        // gizmo
        auto& actorTransform = selectedActor->GetTransform();
        auto actorTransformMatrix = actorTransform.GetWorldTransformMatrix();
        ImGuizmo::SetRect(viewportMin_.x, viewportMin_.y, float(viewportWidth_), float(viewportHeight_));
        ImGuizmo::Manipulate(&cameraViewMatrix[0].x, &renderCamera.GetProjectionMatrix()[0].x,
            ImGuizmo::OPERATION::ROTATE, ImGuizmo::MODE::LOCAL, &actorTransformMatrix[0].x);

        Math::Decompose(actorTransformMatrix, scale, rotation, position);
        actorTransform.SetPosition(position);
        actorTransform.SetRotation(rotation);
        actorTransform.SetScale(scale);
    }

    {
        // The navigating cube at the top-right corner
        // TODO: Set proper length argument, note, this argument should not be 0
        ImGuizmo::ViewManipulate(&cameraViewMatrix[0].x, 0.5, ImVec2(viewportMax_.x - 128, viewportMin_.y), ImVec2(128, 128), 0x10101010);
        auto cameraTransformMatrix = Math::Inverse(cameraViewMatrix);

        Math::Decompose(cameraTransformMatrix, scale, rotation, position);
        cameraTransform.SetPosition(position);
        cameraTransform.SetRotation(rotation);
    }
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
    if (IsFocused() && inputManager.IsMouseButtonReleaseEvent(Loong::App::LoongMouseButton::kButtonLeft) && Math::Distance(inputManager.GetMouseDownPosition(), mousePos) < 4.0F) {
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

                Core::LoongRenderPass::UniformBlock ubo {};
                ubo.ub_Projection = camera.GetCamera().GetProjectionMatrix();
                ubo.ub_View = camera.GetCamera().GetViewMatrix();
                ubo.ub_ViewPos = camera.GetOwner()->GetTransform().GetWorldPosition();
                ubo.ub_Model = selectedActor->GetTransform().GetWorldTransformMatrix();
                GetEditorContext().GetUniformBuffer()->SetSubData(&ubo, 0);
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
