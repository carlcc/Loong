//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#include "LoongEditorInspector.h"
#include "../../utils/ImGuiUtils.h"
#include "../../utils/LoongFileTreeNode.h"
#include "LoongCore/scene/components/LoongCCamera.h"
#include "LoongCore/scene/components/LoongCLight.h"
#include "LoongCore/scene/components/LoongCModelRenderer.h"
#include "LoongFoundation/LoongTransform.h"
#include "LoongRenderer/LoongLight.h"
#include "LoongResource/LoongMaterial.h"
#include "LoongResource/LoongResourceManager.h"
#include <any>
#include <imgui.h>
#include <imgui_stdlib.h>

namespace Loong::Editor {

void LoongEditorInspector::Inspect(Core::LoongCCamera* camera)
{
    ImGui::Columns(2, nullptr);
    ImGui::PushID((void*)camera);

    {
        ImGui::Text("Field Of View");
        ImGui::NextColumn();
        float value = Math::RadToDegree(camera->GetFov());
        if (ImGui::DragFloat("###Fov", &value, 0.10F, 0.1F, 180.0F, "%.1f")) {
            camera->SetFov(Math::DegreeToRad(value));
        }
        ImGui::NextColumn();
    }

    {
        ImGui::Text("Near Plane");
        ImGui::NextColumn();
        float value = camera->GetNear();
        if (ImGui::DragFloat("###Near", &value, 0.10F, 0.01F, camera->GetFar(), "%.1f")) {
            camera->SetNear(value);
        }
        ImGui::NextColumn();
    }

    {
        ImGui::Text("Far Plane");
        ImGui::NextColumn();
        float value = camera->GetFar();
        if (ImGui::DragFloat("###Far", &value, 0.01F, camera->GetNear(), std::numeric_limits<float>::infinity(), "%.1f", 4.0F)) {
            camera->SetFar(value);
        }
        ImGui::NextColumn();
    }

    {
        ImGui::Text("Clear Color");
        ImGui::NextColumn();
        Math::Vector3 value = camera->GetClearColor();
        if (ImGui::ColorEdit3("###ClearColor", &value.x, ImGuiColorEditFlags_None)) {
            camera->SetClearColor(value);
        }
        ImGui::NextColumn();
    }
    ImGui::PopID();
    ImGui::Columns(1, nullptr);
}

void LoongEditorInspector::Inspect(Core::LoongCLight* light)
{
    ImGui::Columns(2, nullptr);
    ImGui::PushID((void*)light);

    {
        ImGui::Text("Type");
        ImGui::NextColumn();
        int type = int(light->GetType());
        auto& lightNames = Renderer::LoongLight::kTypeNames;
        const char* currentName = lightNames[type].c_str();

        if (ImGui::BeginCombo("###Type", currentName, ImGuiComboFlags_None)) {
            int i = 0;
            for (const auto& option : lightNames) {
                const bool isSelected = option.c_str() == currentName;
                if (ImGui::Selectable(option.c_str(), isSelected)) {
                    // is changed
                    auto newValue = std::make_any<std::string>(option.c_str());
                    light->SetType(Core::LoongCLight::Type(i));
                }
                // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
                if (isSelected) {
                    ImGui::SetItemDefaultFocus();
                }
                ++i;
            }
            ImGui::EndCombo();
        }
        ImGui::NextColumn();
    }

    {
        ImGui::Text("Color");
        ImGui::NextColumn();
        Math::Vector3 value = light->GetColor();
        if (ImGui::ColorEdit3("###ClearColor", &value.x, ImGuiColorEditFlags_None)) {
            light->SetColor(value);
        }
        ImGui::NextColumn();
    }

    {
        ImGui::Text("Intensity");
        ImGui::NextColumn();
        float value = light->GetIntensity();
        if (ImGui::DragFloat("###Intensity", &value, 0.10F, 0.0F, std::numeric_limits<float>::infinity(), "%.1f")) {
            light->SetIntensity(value);
        }
        ImGui::NextColumn();
    }

    {
        ImGui::Text("Falloff radius");
        ImGui::NextColumn();
        float value = light->GetFalloffRadius();
        if (ImGui::DragFloat("###Falloff", &value, 0.10F, 0.0F, std::numeric_limits<float>::infinity(), "%.1f")) {
            light->SetFalloffRadius(value);
        }
        ImGui::NextColumn();
    }

    {
        float innerAngle = Math::RadToDegree(light->GetInnerAngle());
        float outerAngle = Math::RadToDegree(light->GetOuterAngle());

        ImGui::Text("Fading Inner Angle");
        ImGui::NextColumn();
        if (ImGui::DragFloat("###InnerAngle", &innerAngle, 0.10F, 0.0F, outerAngle, "%.1f")) {
            light->SetInnerAngle(Math::DegreeToRad(innerAngle));
        }
        ImGui::NextColumn();

        ImGui::Text("Fading Outer Angle");
        ImGui::NextColumn();
        if (ImGui::DragFloat("###OuterAngle", &outerAngle, 0.10F, innerAngle, 180.0F, "%.1f")) {
            light->SetOuterAngle(Math::DegreeToRad(outerAngle));
        }
        ImGui::NextColumn();
    }

    ImGui::PopID();
    ImGui::Columns(1, nullptr);
}

void LoongEditorInspector::Inspect(Core::LoongCModelRenderer* model)
{
    ImGui::Columns(2, nullptr);
    ImGui::PushID((void*)model);

    auto lowModel = model->GetModel();
    {
        ImGui::Text("Model");
        ImGui::NextColumn();
        std::string modelPath;
        if (lowModel != nullptr) {
            modelPath = lowModel->GetPath();
        }
        if (ImGui::InputText("###Model", &modelPath, ImGuiInputTextFlags_ReadOnly)) {
            // model->SetColor(value);
        }

        if (ImGui::BeginDragDropTarget()) {
            auto* node = ImGuiUtils::GetDropData<LoongFileTreeNode*>(ImGuiUtils::kDragTypeFile);
            if (node != nullptr) {
                auto fullPath = node->GetFullPath();
                auto newModel = Resource::LoongResourceManager::GetModel(fullPath);
                if (newModel != nullptr) {
                    model->SetModel(newModel);
                    lowModel = newModel;
                } else {
                    LOONG_ERROR("Cannot set model to '{}', which is not a valid model file", fullPath);
                }
            }
            ImGui::EndDragDropTarget();
        }

        ImGui::SameLine();
        if (ImGui::Button("X")) {
            model->SetModel(nullptr);
            lowModel = nullptr;
        }

        ImGui::NextColumn();
    }

    {
        if (model->GetMaterials().size() > 0) {
            assert(lowModel != nullptr);
            // ImGui::Text("Materials");
            // ImGui::NextColumn();
            // ImGui::NextColumn();

            std::string modelPath;
            assert(model->GetMaterials().size() == lowModel->GetMaterialNames().size());
            for (size_t i = 0; i < model->GetMaterials().size(); ++i) {
                std::string materialName = lowModel->GetMaterialNames()[i];

                ImGuiUtils::ScopedId scopedId(materialName.c_str());

                auto material = model->GetMaterials()[i];
                if (material != nullptr) {
                    modelPath = material->GetPath();
                } else {
                    modelPath.clear();
                }
                ImGui::Text("%s", materialName.c_str());
                ImGui::NextColumn();
                if (ImGui::InputText("", &modelPath, ImGuiInputTextFlags_ReadOnly)) {
                }

                if (ImGui::BeginDragDropTarget()) {
                    auto* node = ImGuiUtils::GetDropData<LoongFileTreeNode*>(ImGuiUtils::kDragTypeFile);
                    if (node != nullptr) {
                        auto fullPath = node->GetFullPath();
                        auto newMaterial = Resource::LoongResourceManager::GetMaterial(fullPath);
                        if (newMaterial != nullptr) {
                            model->SetMaterial(int(i), newMaterial);
                        } else {
                            LOONG_ERROR("Cannot set material to '{}', which is not a valid material file", fullPath);
                        }
                    }
                    ImGui::EndDragDropTarget();
                }
                ImGui::SameLine();
                if (ImGui::Button("X")) {
                    model->SetMaterial(int(i), nullptr);
                }
                ImGui::NextColumn();
            }
            ImGui::NextColumn();
        }
    }

    ImGui::PopID();
    ImGui::Columns(1, nullptr);
}

void LoongEditorInspector::Inspect(Foundation::Transform& transform)
{
    ImGui::Columns(2, nullptr, true);

    Math::Vector3 position = transform.GetPosition();
    Math::Vector3 rotation = Math::RadToDegree(Math::QuatToEuler(transform.GetRotation()));
    Math::Vector3 scale = transform.GetScale();
    ImGui::Text("Position");
    ImGui::NextColumn();
    if (ImGui::DragFloat3("###Position", &position.x, 0.1F, 0.0F, 0.0F, "%0.1f")) {
        transform.SetPosition(position);
    }
    ImGui::NextColumn();

    ImGui::Text("Rotation");
    ImGui::NextColumn();
    if (ImGui::DragFloat3("###Rotation", &rotation.x, 1.0F, 0.0F, 0.0F, "%0.1f")) {
        transform.SetRotation(Math::EulerToQuat(Math::DegreeToRad(rotation)));
    }
    ImGui::NextColumn();

    ImGui::Text("Scale");
    ImGui::NextColumn();
    if (ImGui::DragFloat3("###Scale", &scale.x, 0.05F, 0.0F, 0.0F, "%0.2f")) {
        transform.SetScale(scale);
    }
    ImGui::NextColumn();

    ImGui::Columns(1, nullptr);
}

}