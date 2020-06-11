//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#include "LoongEditorTreeNodeWidget.h"
#include <imgui.h>

namespace Loong::Editor {

void EditorTreeNodeWidget::DrawImpl()
{
    bool prevOpened = isOpened_;

    if (shouldOpen_) {
        ImGui::SetNextTreeNodeOpen(true);
        shouldOpen_ = false;
    } else if (shouldClose_) {
        ImGui::SetNextTreeNodeOpen(false);
        shouldClose_ = false;
    }

    // clang-format off
    ImGuiTreeNodeFlags     flags = ImGuiTreeNodeFlags_None;
    if (arrowClickToOpen_) flags |= ImGuiTreeNodeFlags_OpenOnArrow;
    if (isSelected_)       flags |= ImGuiTreeNodeFlags_Selected;
    if (isLeaf_)           flags |= ImGuiTreeNodeFlags_Leaf;
    // clang-format on

    bool opened = ImGui::TreeNodeEx((name_ + widgetId_).c_str(), flags);

    if (ImGui::IsItemClicked() && (ImGui::GetMousePos().x - ImGui::GetItemRectMin().x) > ImGui::GetTreeNodeToLabelSpacing()) {
        OnClickSignal_.emit(this);
    }

    if (opened) {
        if (!prevOpened) {
            OnExpandSignal_.emit(this);
        }

        isOpened_ = true;

        for (auto& widget : children_) {
            widget->Draw();
        }

        ImGui::TreePop();
    } else {
        if (prevOpened) {
            OnCollapseSignal_.emit(this);
        }

        isOpened_ = false;
    }
}

}