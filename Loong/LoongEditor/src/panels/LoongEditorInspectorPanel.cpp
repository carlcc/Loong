//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#include "LoongEditorInspectorPanel.h"
#include "../LoongEditor.h"
#include "../LoongEditorContext.h"
#include "../utils/ImGuiUtils.h"
#include "LoongCore/scene/LoongActor.h"
#include "LoongCore/scene/LoongComponent.h"
#include "LoongCore/scene/components/LoongCCamera.h"
#include "LoongCore/scene/components/LoongCLight.h"
#include "LoongCore/scene/components/LoongCModelRenderer.h"
#include "inspector/LoongEditorInspector.h"
#include <imgui.h>
#include <imgui_stdlib.h>

namespace Loong::Editor {

void LoongEditorInspectorPanel::UpdateImpl(const Foundation::LoongClock& clock)
{
    auto* selectedActor = GetEditorContext().GetCurrentSelectedActor();
    if (selectedActor == nullptr) {
        return;
    }

    ImGuiUtils::ScopedId scopedId(this);

    {
        ImGui::Columns(2, nullptr, true);
        ImGui::Text("Name");
        ImGui::NextColumn();
        std::string name = selectedActor->GetName();
        if (ImGui::InputText("", &name)) {
            selectedActor->SetName(name);
        }
        ImGui::NextColumn();
    }
    {
        ImGui::Columns(2, nullptr, true);
        ImGui::Text("Tag");
        ImGui::NextColumn();
        std::string tag = selectedActor->GetTag();
        if (ImGui::InputText("###Tag", &tag)) {
            selectedActor->SetTag(tag);
        }
        ImGui::NextColumn();
    }

    // Transform
    LoongEditorInspector::Inspect(selectedActor->GetTransform());
    //

    Core::LoongComponent* componentToRemove = nullptr;
    for (const std::shared_ptr<Core::LoongComponent>& component : selectedActor->GetComponents()) {
        bool open = true;
        ImGui::Columns(1, nullptr);

        ImGui::PushID(component.get());
        auto showProps = ImGui::CollapsingHeader(component->GetName().c_str(), &open, ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_OpenOnDoubleClick);
        ImGui::PopID();

        if (!open) {
            componentToRemove = component.get();
            continue; // Don't need to draw it
        }

        if (showProps) {
            ImGui::Columns(2, nullptr);
            // TODO Display properties

            ImGuiUtils::ScopedId componentScopeId(component.get());

            if (auto* camera = dynamic_cast<Core::LoongCCamera*>(component.get()); camera != nullptr) {
                LoongEditorInspector::Inspect(camera);
            }
            if (auto* light = dynamic_cast<Core::LoongCLight*>(component.get()); light != nullptr) {
                LoongEditorInspector::Inspect(light);
            }
            if (auto* model = dynamic_cast<Core::LoongCModelRenderer*>(component.get()); model != nullptr) {
                LoongEditorInspector::Inspect(model);
            }
        }
    }
    selectedActor->RemoveComponent(componentToRemove);
}

}
