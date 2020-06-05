//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#include "LoongGui/LoongGuiWindow.h"
#include "ImGuiUtils.h"
#include <imgui.h>

namespace Loong::Gui {

LoongGuiWindow::LoongGuiWindow(const std::string& label)
    : LoongGuiContainer(label)
{
    SetMovable(true);
    SetResizable(true);
    SetBringToFrontOnFocus(true);
    SetTitleless(false);
    SetHasMenuBar(false);
}

void LoongGuiWindow::Draw()
{
    if (!IsVisible()) {
        return;
    }

    uint32_t flags = ImGuiWindowFlags_None;
    // clang-format off
    if (!IsMovable())                   flags |= ImGuiWindowFlags_NoMove;
    if (!IsResizable())                 flags |= ImGuiWindowFlags_NoResize;
    if (IsTitleless())                  flags |= ImGuiWindowFlags_NoTitleBar;
    if (HasMenuBar())                   flags |= ImGuiWindowFlags_MenuBar;
    if (!IsBringToFrontOnFocus())       flags |= ImGuiWindowFlags_NoBringToFrontOnFocus;
    if (!IsDockable())                  flags |= ImGuiWindowFlags_NoDocking;
    // clang-format on

    bool* isOpen = IsClosable() ? &isVisible_ : nullptr;
    ImGui::SetNextWindowPos(ToImVec(GetPosition()), ImGuiCond_Once);
    ImGui::SetNextWindowSize(ToImVec(GetSize()));
    ScopedId scopedId(this);
    if (ImGui::Begin(label_.c_str(), isOpen, flags)) {

        DrawChildren();

        if (IsResizable()) {
            size_ = ToFloat2(ImGui::GetWindowSize());
        }
        if (IsMovable()) {
            position_ = ToFloat2(ImGui::GetWindowPos());
        }
    }
    ImGui::End();
}

}