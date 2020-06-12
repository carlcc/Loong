//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#include "LoongEditorMaterialEditorPanel.h"
#include "../LoongEditor.h"
#include "../LoongEditorConstants.h"
#include "../LoongEditorContext.h"
#include "../utils/ImGuiUtils.h"
#include "../utils/LoongFileTreeNode.h"
#include "LoongCore/scene/LoongScene.h"
#include "LoongCore/scene/components/LoongCCamera.h"
#include "LoongCore/scene/components/LoongCModelRenderer.h"
#include "LoongFoundation/LoongDefer.h"
#include "LoongRenderer/LoongRenderer.h"
#include "LoongResource/LoongFrameBuffer.h"
#include "LoongResource/LoongMaterial.h"
#include "LoongResource/LoongResourceManager.h"
#include "LoongResource/LoongShader.h"
#include "LoongResource/loader/LoongMaterialLoader.h"
#include "inspector/LoongEditorInspector.h"
#include <imgui.h>
#include <imgui_stdlib.h>
#include <string>

namespace Loong::Editor {

LoongEditorMaterialEditorPanel::LoongEditorMaterialEditorPanel(LoongEditor* editor, const std::string& name, bool opened, const LoongEditorPanelConfig& cfg)
    : LoongEditorRenderPanel(editor, name, opened, cfg)
{
    materialBackUp_ = std::make_shared<Resource::LoongMaterial>();
    previewScene_ = Core::LoongScene::CreateScene("PreviewSceneRoot");

    {
        // prepare the scene
        previewActor_ = Core::LoongScene::CreateActor("Actor").release();
        previewModel_ = previewActor_->AddComponent<Core::LoongCModelRenderer>();
        previewModel_->SubscribeModelChanged(this, &LoongEditorMaterialEditorPanel::OnPreviewActorModelChanged);
        previewActor_->SetParent(previewScene_.get());
    }
}

void LoongEditorMaterialEditorPanel::Render(const Foundation::LoongClock& clock)
{
    if (!IsVisible() || viewportWidth_ <= 0 || viewportHeight_ <= 0) {
        return;
    }
    GetFrameBuffer()->Bind();
    auto* camera = cameraActor_->GetComponent<Core::LoongCCamera>();
    glViewport(0, 0, viewportWidth_, viewportHeight_);
    GetEditorContext().GetRenderer().Clear(camera->GetCamera(), true, true, true);
    RenderSceneForCamera(*previewScene_, *camera);

    GetFrameBuffer()->Unbind();
}

void LoongEditorMaterialEditorPanel::AdjustPreviewModelAndCamera()
{
    auto model = previewModel_->GetModel();
    if (model == nullptr) {
        return;
    }
    auto& aabb = model->GetAABB();
    auto size = aabb.max - aabb.min;
    auto maxDimension = std::max({ size.x, size.y, size.z });
    previewActor_->GetTransform().SetScale(Math::Vector3 { 1.0F / maxDimension });
    previewActor_->GetTransform().SetPosition(Math::Zero);
    cameraActor_->GetTransform().SetPosition(Math::Vector3 { 2.0F });
    cameraActor_->GetTransform().LookAt(Math::Zero, Math::kUp);
}

void LoongEditorMaterialEditorPanel::OpenMaterial(const LoongFileTreeNode* fileNode)
{
    currentMaterial_ = Resource::LoongResourceManager::GetMaterial(fileNode->GetFullPath());
    if (currentMaterial_ != nullptr) {
        *materialBackUp_ = *currentMaterial_;
        materialFileFullPath_ = fileNode->GetFullPath();
        auto model = Resource::LoongResourceManager::GetModel(Constants::kSphereModelPath);
        previewModel_->SetModel(model);
        previewModel_->SetMaterial(0, currentMaterial_);

        AdjustPreviewModelAndCamera();
    }
}

void LoongEditorMaterialEditorPanel::OnPreviewActorModelChanged(Resource::LoongGpuModel* newModel, Resource::LoongGpuModel* oldModel)
{
    AdjustPreviewModelAndCamera();
}

void LoongEditorMaterialEditorPanel::UpdateImpl(const Foundation::LoongClock& clock)
{
    ImGui::Columns(2, nullptr);
    {
        ImGui::BeginChild("Properties");

        UpdateProperies(clock);

        ImGui::EndChild();
    }

    ImGui::NextColumn();

    {
        ImGui::BeginChild("Preview");
        LoongEditorRenderPanel::UpdateImpl(clock);
        ImGui::EndChild();
    }
    ImGui::NextColumn();
    ImGui::Columns(1, nullptr);
}

void LoongEditorMaterialEditorPanel::UpdateProperies(const Foundation::LoongClock& clock)
{
    if (currentMaterial_ == nullptr) {
        return;
    }

    ImGui::Columns(2, nullptr);
    ImGui::Text("File");
    ImGui::NextColumn();
    ImGui::InputText("", &materialFileFullPath_, ImGuiInputTextFlags_ReadOnly);
    ImGui::NextColumn();
    ImGui::Columns(1, nullptr);
    {
        if (ImGui::Button("Restore")) {
            *currentMaterial_ = *materialBackUp_;
        }
        ImGui::SameLine();
        if (ImGui::Button("Save")) {
            // TODO: Show a confirm?
            Resource::LoongMaterialLoader::Write(materialFileFullPath_, currentMaterial_.get());
            *materialBackUp_ = *currentMaterial_;
        }
    }
    ImGui::Separator();

    {
        if (ImGui::CollapsingHeader("The Preview Model", ImGuiTreeNodeFlags_None)) {
            ImGuiUtils::ScopedId scopedId(1);
            LoongEditorInspector::Inspect(previewActor_->GetTransform());
            LoongEditorInspector::Inspect(previewModel_);
        }
        if (ImGui::CollapsingHeader("The Preview Camera", ImGuiTreeNodeFlags_None)) {
            ImGuiUtils::ScopedId scopedId(2);
            LoongEditorInspector::Inspect(cameraActor_->GetTransform());
            LoongEditorInspector::Inspect(cameraActor_->GetComponent<Core::LoongCCamera>());
        }
    }

    ImGui::Separator();

    {
        ImGui::BeginChild("Properties");
        OnScopeExit { ImGui::EndChild(); };

        auto currentShader = currentMaterial_->GetShader();
        ImGui::Columns(2, nullptr);
        {
            ImGui::Text("Shader");
            ImGui::NextColumn();
            std::string currentShaderPath = currentShader == nullptr ? "" : currentShader->GetPath();
            ImGui::InputText("", &currentShaderPath, ImGuiInputTextFlags_ReadOnly);

            if (ImGui::BeginDragDropTarget()) {
                auto* node = ImGuiUtils::GetDropData<LoongFileTreeNode*>(ImGuiUtils::kDragTypeShaderFile);
                if (node != nullptr) {
                    auto fullPath = node->GetFullPath();
                    auto newShader = Resource::LoongResourceManager::GetShader(fullPath);
                    if (newShader != nullptr) {
                        currentMaterial_->SetShader(newShader);
                        currentShader = newShader;
                    } else {
                        LOONG_ERROR("Cannot set shader to '{}', which is not a valid shader file", fullPath);
                    }
                }
                ImGui::EndDragDropTarget();
            }

            ImGui::SameLine();
            if (ImGui::Button("X")) {
                currentMaterial_->SetShader(nullptr);
                currentShader = nullptr;
            }
            ImGui::NextColumn();
        }

        if (currentShader != nullptr) {
            ImGui::Columns(1, nullptr);
            bool expand = ImGui::CollapsingHeader("Shader Parameters", ImGuiTreeNodeFlags_DefaultOpen);
            ImGui::Columns(2, nullptr);
            if (expand) {
                const auto& uniformInfos = currentShader->GetUniformInfo();
                auto& uniformData = currentMaterial_->GetUniformsData();

                for (auto& info : uniformInfos) {
                    ImGui::Text("%s", info.name.c_str());
                    ImGui::NextColumn();
                    auto& value = uniformData[info.name];

                    ImGui::PushID(info.name.c_str());
                    switch (info.type) {
                    case Resource::LoongShader::UniformType::kUniformBool: {
                        bool v = value.type() == typeid(bool) ? std::any_cast<bool>(value) : false;
                        if (ImGui::Checkbox("", &v)) {
                            value = v;
                        }
                        break;
                    }
                    case Resource::LoongShader::UniformType::kUniformInt: {
                        int v = value.type() == typeid(int) ? std::any_cast<int>(value) : 0;
                        if (ImGui::DragInt("", &v)) {
                            value = v;
                        }
                        break;
                    }
                    case Resource::LoongShader::UniformType::kUniformFloat: {
                        float v = value.type() == typeid(float) ? std::any_cast<float>(value) : 0.0F;
                        if (ImGui::DragFloat("", &v, 0.1F)) {
                            value = v;
                        }
                        break;
                    }
                    case Resource::LoongShader::UniformType::kUniformFloatVec2: {
                        Math::Vector2 v = value.type() == typeid(Math::Vector2) ? std::any_cast<Math::Vector2>(value) : Math::Zero;
                        if (ImGui::DragFloat2("", &v.x, 0.1F)) {
                            value = v;
                        }
                        break;
                    }
                    case Resource::LoongShader::UniformType::kUniformFloatVec3: {
                        Math::Vector3 v = value.type() == typeid(Math::Vector3) ? std::any_cast<Math::Vector3>(value) : Math::Zero;
                        if (ImGui::DragFloat3("", &v.x, 0.1F)) {
                            value = v;
                        }
                        break;
                    }
                    case Resource::LoongShader::UniformType::kUniformFloatVec4: {
                        Math::Vector4 v = value.type() == typeid(Math::Vector4) ? std::any_cast<Math::Vector4>(value) : Math::Zero;
                        if (ImGui::DragFloat4("", &v.x, 0.1F)) {
                            value = v;
                        }
                        break;
                    }
                    case Resource::LoongShader::UniformType::kUniformFloatMat4: {
                        Math::Matrix4 v = value.type() == typeid(Math::Matrix4) ? std::any_cast<Math::Matrix4>(value) : Math::Zero;
                        if (ImGui::DragFloat4("", &v[0].x)) {
                            value = v;
                        }
                        if (ImGui::DragFloat4("", &v[1].x)) {
                            value = v;
                        }
                        if (ImGui::DragFloat4("", &v[2].x)) {
                            value = v;
                        }
                        if (ImGui::DragFloat4("", &v[3].x)) {
                            value = v;
                        }
                        break;
                    }
                    case Resource::LoongShader::UniformType::kUniformSampler2D: {
                        using TextureRef = std::shared_ptr<Resource::LoongTexture>;
                        TextureRef tex2 = value.type() == typeid(TextureRef) ? std::any_cast<TextureRef>(value) : nullptr;
                        const ImVec2 kPreviewSize { 80.F, 80.F };
                        if (tex2 == nullptr) {
                            ImGui::Button("Drag A\nTexture\nHere", kPreviewSize);
                        } else {
                            ImGui::Image((void*)(intptr_t)tex2->GetId(), kPreviewSize, ImVec2 { 0, 1 }, ImVec2 { 1, 0 });
                        }
                        if (ImGui::BeginDragDropTarget()) {
                            auto* node = ImGuiUtils::GetDropData<LoongFileTreeNode*>(ImGuiUtils::kDragTypeTextureFile);
                            if (node != nullptr) {
                                auto fullPath = node->GetFullPath();
                                auto newTexture = Resource::LoongResourceManager::GetTexture(fullPath);
                                if (newTexture != nullptr) {
                                    value = newTexture;
                                } else {
                                    LOONG_ERROR("Cannot set texture to '{}', which is not a valid texture file", fullPath);
                                }
                            }
                            ImGui::EndDragDropTarget();
                        }
                        ImGui::SameLine();
                        if (ImGui::Button("X")) {
                            value = TextureRef(nullptr);
                        }

                        break;
                    }
                    case Resource::LoongShader::UniformType::kUniformSamplerCube:
                    default:
                        assert(false); // impossible, or there is a bug in Shader::GetUniformInfo();
                    }
                    ImGui::PopID();

                    ImGui::NextColumn();
                }
            }
        }

        {
            ImGui::Columns(1, nullptr);
            bool expand = ImGui::CollapsingHeader("Render States", ImGuiTreeNodeFlags_DefaultOpen);
            ImGui::Columns(2, nullptr);
            if (expand) {
                bool blendable = currentMaterial_->IsBlendable();
                ImGui::Text("Blend");
                ImGui::NextColumn();
                if (ImGui::Checkbox("###Blend", &blendable)) {
                    currentMaterial_->SetBlendable(blendable);
                }
                ImGui::NextColumn();

                bool backFaceCulling = currentMaterial_->HasBackFaceCulling();
                ImGui::Text("BackCull");
                ImGui::NextColumn();
                if (ImGui::Checkbox("###BackCull", &backFaceCulling)) {
                    currentMaterial_->SetBackFaceCulling(backFaceCulling);
                }
                ImGui::NextColumn();

                bool frontFaceCulling = currentMaterial_->HasFrontFaceCulling();
                ImGui::Text("FrontCull");
                ImGui::NextColumn();
                if (ImGui::Checkbox("###FrontCull", &frontFaceCulling)) {
                    currentMaterial_->SetFrontFaceCulling(frontFaceCulling);
                }
                ImGui::NextColumn();

                bool depthTest = currentMaterial_->HasDepthTest();
                ImGui::Text("DepthTest");
                ImGui::NextColumn();
                if (ImGui::Checkbox("###DepthTest", &depthTest)) {
                    currentMaterial_->SetDepthTest(depthTest);
                }
                ImGui::NextColumn();

                bool depthWriting = currentMaterial_->HasDepthWriting();
                ImGui::Text("Depth Write");
                ImGui::NextColumn();
                if (ImGui::Checkbox("###DepthW", &depthWriting)) {
                    currentMaterial_->SetDepthWriting(depthWriting);
                }
                ImGui::NextColumn();

                bool colorWriting = currentMaterial_->HasColorWriting();
                ImGui::Text("Color Write");
                ImGui::NextColumn();
                if (ImGui::Checkbox("###ColorW", &colorWriting)) {
                    currentMaterial_->SetColorWriting(colorWriting);
                }
                ImGui::NextColumn();
            }
        }
    }
}

}