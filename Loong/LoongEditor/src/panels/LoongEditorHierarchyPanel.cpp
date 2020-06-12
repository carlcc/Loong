//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#include "LoongEditorHierarchyPanel.h"
#include "../LoongEditor.h"
#include "../LoongEditorContext.h"
#include "../utils/ImGuiUtils.h"
#include "../utils/LoongEditorTemplates.h"
#include "LoongCore/scene/LoongActor.h"
#include "LoongCore/scene/LoongScene.h"
#include "LoongFoundation/LoongLogger.h"
#include <imgui.h>

namespace Loong::Editor {

LoongEditorHierarchyPanel::LoongEditorHierarchyPanel(LoongEditor* editor, const std::string& name, bool opened, const LoongEditorPanelConfig& cfg)
    : LoongEditorPanel(editor, name, opened, cfg)
{
    SubscribeOnClickNode(GetEditorContext(), &LoongEditorContext::SetCurrentSelectedActor);
}

void LoongEditorHierarchyPanel::UpdateImpl(const Foundation::LoongClock& clock)
{
    auto scene = GetEditorContext().GetCurrentScene().get();
    if (scene != nullptr) {
        DrawNode(scene, editor_->GetContext().GetCurrentSelectedActor());
    }
}

void LoongEditorHierarchyPanel::DrawNode(Core::LoongActor* node, Core::LoongActor* currentSelected)
{
    // clang-format off
    ImGuiTreeNodeFlags     flags = ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_OpenOnArrow;
    if (node == currentSelected)     flags |= ImGuiTreeNodeFlags_Selected;
    if (node->GetChildren().empty()) flags |= ImGuiTreeNodeFlags_Leaf;
    // clang-format on

    ImGuiUtils::ScopedId scopedId(node);

    bool opened = ImGui::TreeNodeEx((node->GetName() + "###" + std::to_string(node->GetID())).c_str(), flags);

    if (node->HasParent()) { // It is not root node
        if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {
            ImGui::Text("%s", node->GetName().c_str());
            // TODO: Use different key or diffrend file type?
            ImGuiUtils::SetDragData(ImGuiUtils::kDragTypeActor, node);
            ImGui::EndDragDropSource();
        }
    }
    if (ImGui::BeginDragDropTarget()) {
        auto* draggedNode = ImGuiUtils::GetDropData<Core::LoongActor*>(ImGuiUtils::kDragTypeActor);
        if (draggedNode != nullptr) {
            draggedNode->SetParent(node);
            LOONG_DEBUG("Reset actor '{}'(ID {})'s parent to '{}'(ID {})", draggedNode->GetName(), draggedNode->GetID(), node->GetName(), node->GetID());
        }
        ImGui::EndDragDropTarget();
    }
    if (ImGui::IsItemClicked() && (ImGui::GetMousePos().x - ImGui::GetItemRectMin().x) > ImGui::GetTreeNodeToLabelSpacing()) {
        OnClickNodeSignal_.emit(node);
    }

    // NOTE: Must place this before traversing it's children
    auto* scene = GetEditorContext().GetCurrentScene().get();
    LoongEditorTemplates::ShowActorContextMenu(node, scene, editor_);

    if (opened) {
        for (auto* actor : node->GetChildren()) {
            DrawNode(actor, currentSelected);
        }
        ImGui::TreePop();
    }
}

}