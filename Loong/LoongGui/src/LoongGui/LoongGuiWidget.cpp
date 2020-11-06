//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#include "LoongGui/LoongGuiWidget.h"
#include "LoongGui/LoongGuiContainer.h"
#include <imgui.h>

namespace Loong::Gui {

LoongGuiWidget::LoongGuiWidget()
    : id_ { std::to_string(++LoongGuiWidget::sIdCounter) }
{
    SetLabel("");
}

void LoongGuiWidget::SetLabel(const std::string& label)
{
    label_ = label;
    labelAndId_ = label_ + "###" + id_;
}

void LoongGuiWidget::Draw()
{
    if (isEnabled_) {
        DrawThis();

        if (!hasLineBreak_) {
            ImGui::SameLine();
        }
    }
}

void LoongGuiWidget::SetParent(LoongGuiContainer* parent)
{
    if (parent_ != nullptr) {
        parent_->RemoveChild(this);
    }
    parent_ = parent;
    if (parent_ != nullptr) {
        parent_->AddChild(this);
    }
}

std::atomic_int64_t LoongGuiWidget::sIdCounter { 0 };

}