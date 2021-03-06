//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#include "LoongEditorTemplates.h"
#include "../LoongEditor.h"
#include "../LoongEditorConstants.h"
#include "../LoongEditorContext.h"
#include "ImGuiUtils.h"
#include "LoongCore/scene/LoongActor.h"
#include "LoongCore/scene/LoongScene.h"
#include "LoongCore/scene/components/LoongCCamera.h"
#include "LoongCore/scene/components/LoongCLight.h"
#include "LoongCore/scene/components/LoongCModelRenderer.h"
#include "LoongCore/scene/components/LoongCSky.h"
#include "LoongResource/LoongResourceManager.h"
#include <imgui.h>

namespace Loong::Editor::LoongEditorTemplates {

void ShowActorContextMenu(Core::LoongActor* actor, Core::LoongScene* scene, LoongEditor* editor)
{
    ImGuiUtils::ScopedId scopedId(scene);

    if (ImGui::BeginPopupContextItem("Context Menu")) {
        FillActorMenu(actor, scene, editor);
        ImGui::EndPopup();
    }
}

void FillActorMenu(Core::LoongActor* actor, Core::LoongScene* scene, LoongEditor* editor)
{
    bool isActorSelected = actor != nullptr;
    bool isRootActor = actor != nullptr && !actor->HasParent();

    if (ImGui::MenuItem("Add Empty Child###0", nullptr, false, isActorSelected)) {
        editor->AddEndFrameTask([actor, editor]() {
            auto* newActor = Core::LoongScene::CreateActor("New Actor").release();
            newActor->SetParent(actor);
            editor->GetContext().SetCurrentSelectedActor(newActor);
            assert(actor != nullptr);
            LOONG_DEBUG("Create child(ID {}) for actor {}(ID {})", newActor->GetID(), actor->GetName(), actor->GetID());
        });
    }
    if (ImGui::BeginMenu("Add Component###1", isActorSelected)) {
        using CComponent = Core::LoongComponent;
        using Actor = Core::LoongActor;
        static std::vector<std::pair<const char*, std::function<CComponent*(Actor*)>>> kComponentCreatorMap = {
            { "Model", [](Actor* a) -> CComponent* { return a->AddComponent<Core::LoongCModelRenderer>(); } },
            { "Light", [](Actor* a) -> CComponent* { return a->AddComponent<Core::LoongCLight>(); } },
            { "Camera", [](Actor* a) -> CComponent* { return a->AddComponent<Core::LoongCCamera>(); } },
        };
        for (auto& [name, creator] : kComponentCreatorMap) {
            ImGui::PushID((void*)name);
            if (ImGui::MenuItem(name, nullptr, false, !isRootActor)) {
                creator(actor);
            }
            ImGui::PopID();
        }
        ImGui::Separator();
        {
            ImGui::PushID((void*)"Sky");
            if (ImGui::MenuItem("Sky", nullptr, false, isRootActor)) {
                actor->AddComponent<Core::LoongCSky>();
            }
            ImGui::PopID();
        }
        ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Add Builtin###2", isActorSelected)) {
        struct BuiltinModelInfo {
            const char* menuName;
            std::string actorName;
            std::string modelPath;
        };
        static BuiltinModelInfo kBuiltinModelInfos[] = {
            { "Cube", "Cube ", Constants::kCubeModelPath },
            { "Cone", "Cone ", Constants::kConeModelPath },
            { "Cylinder", "Cylinder ", Constants::kCylinderModelPath },
            { "Pipe", "Pipe ", Constants::kPipeModelPath },
            { "Plane", "Plane ", Constants::kPlaneModelPath },
            { "Pyramid", "Pyramid ", Constants::kPyramidModelPath },
            { "Sphere", "Sphere ", Constants::kSphereModelPath },
        };
        for (const auto& info : kBuiltinModelInfos) {
            if (ImGui::MenuItem(info.menuName)) {
                editor->AddEndFrameTask([actor, editor, &actorName = info.actorName, &path = info.modelPath]() {
                    auto* newActor = Core::LoongScene::CreateActor("").release();
                    newActor->SetName(actorName + std::to_string(newActor->GetID()));
                    newActor->SetParent(actor);
                    auto* modelRenderer = newActor->AddComponent<Core::LoongCModelRenderer>();
                    modelRenderer->SetModel(Resource::LoongResourceManager::GetModel(path));
                    editor->GetContext().SetCurrentSelectedActor(newActor);
                    LOONG_DEBUG("Create child(ID {}) for actor {}(ID {})", newActor->GetID(), actor->GetName(), actor->GetID());
                });
            }
        }
        ImGui::Separator();
        struct BuiltinLightInfo {
            const char* menuName;
            std::string actorName;
            Core::LoongCLight::Type type;
        };
        static BuiltinLightInfo kBuiltinLightInfos[] = {
            { "Directional Light", "DirectionalLight ", Core::LoongCLight::Type::kTypeDirectional },
            { "Point Light", "PointLight ", Core::LoongCLight::Type::kTypePoint },
            { "Spot Light", "SpotLight ", Core::LoongCLight::Type::kTypeSpot },
        };
        for (const auto& info : kBuiltinLightInfos) {
            if (ImGui::MenuItem(info.menuName)) {
                editor->AddEndFrameTask([actor, editor, &actorName = info.actorName, type = info.type]() {
                    auto* newActor = Core::LoongScene::CreateActor("").release();
                    newActor->SetName(actorName + std::to_string(newActor->GetID()));
                    newActor->SetParent(actor);
                    auto* light = newActor->AddComponent<Core::LoongCLight>();
                    light->SetType(type);
                    editor->GetContext().SetCurrentSelectedActor(newActor);
                    LOONG_DEBUG("Create child(ID {}) for actor {}(ID {})", newActor->GetID(), actor->GetName(), actor->GetID());
                });
            }
        }
        ImGui::Separator();
        if (ImGui::MenuItem("Camera")) {
            editor->AddEndFrameTask([actor, editor]() {
                auto* newActor = Core::LoongScene::CreateActor("").release();
                newActor->SetName("Camera " + std::to_string(newActor->GetID()));
                newActor->SetParent(actor);
                newActor->AddComponent<Core::LoongCCamera>();
                editor->GetContext().SetCurrentSelectedActor(newActor);
                LOONG_DEBUG("Create child(ID {}) for actor {}(ID {})", newActor->GetID(), actor->GetName(), actor->GetID());
            });
        }
        ImGui::EndMenu();
    }
    ImGui::Separator();

    if (ImGui::MenuItem("Delete###3", nullptr, false, isActorSelected && !isRootActor)) {
        actor->MarkAsDestroy();
        if (actor == editor->GetContext().GetCurrentSelectedActor()) {
            editor->GetContext().SetCurrentSelectedActor(nullptr);
        }
        editor->AddEndFrameTask([actor]() {
            delete actor;
        });
    }
}

}
