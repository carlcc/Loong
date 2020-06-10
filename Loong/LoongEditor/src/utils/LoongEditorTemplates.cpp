//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#include "LoongEditorTemplates.h"
#include "../LoongEditor.h"
#include "../LoongEditorContext.h"
#include "ImGuiUtils.h"
#include "LoongCore/scene/LoongActor.h"
#include "LoongCore/scene/LoongScene.h"
#include "LoongCore/scene/components/LoongCCamera.h"
#include "LoongCore/scene/components/LoongCModelRenderer.h"
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
    if (ImGui::MenuItem("Add Child###0", nullptr, false, actor != nullptr)) {
        editor->AddEndFrameTask([actor, scene]() {
            auto* newActor = Core::LoongScene::CreateActor("New Actor").release();
            newActor->SetParent(actor);
            assert(actor != nullptr);
            LOONG_DEBUG("Create child(ID {}) for actor {}(ID {})", newActor->GetID(), actor->GetName(), actor->GetID());
        });
    }
    if (ImGui::BeginMenu("Add Component###1", actor != nullptr)) {
        using CComponent = Core::LoongComponent;
        using Actor = Core::LoongActor;
        static std::vector<std::pair<const char*, std::function<CComponent*(Actor*)>>> kComponentCreatorMap = {
            { "Model", [](Actor* a) -> CComponent* { return a->AddComponent<Core::LoongCModelRenderer>(); } },
            // { "Light", [](Actor* a) -> CComponent* { return a->AddComponent<CLight>(); } },
            { "Camera", [](Actor* a) -> CComponent* { return a->AddComponent<Core::LoongCCamera>(); } },
        };
        for (auto& [name, creator] : kComponentCreatorMap) {
            ImGui::PushID((void*)name);
            if (ImGui::MenuItem(name)) {
                creator(actor);
            }
            ImGui::PopID();
        }
        ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Add Builtin###2")) {
        if (ImGui::MenuItem("Cube")) {
            // TODO
        }
        ImGui::EndMenu();
    }
    ImGui::Separator();

    if (ImGui::MenuItem("Delete###3", nullptr, false, actor != nullptr && actor != actor->GetRoot())) {
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
