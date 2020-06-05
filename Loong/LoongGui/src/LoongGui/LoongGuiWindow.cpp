//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#include "LoongGui/LoongGuiWindow.h"
#include "ImGuiUtils.h"
#include <ImGuiUtils.hpp>
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
    if (!IsResizable())                 flags |= ImGuiColumnsFlags_NoResize;
    if (IsTitleless())                  flags |= ImGuiWindowFlags_NoTitleBar;
    if (HasMenuBar())                   flags |= ImGuiWindowFlags_MenuBar;
    if (!IsBringToFrontOnFocus())       flags |= ImGuiWindowFlags_NoBringToFrontOnFocus;
    // clang-format on

    ImGui::ScopedID scopedId(this);
    bool* isOpen = IsClosable() ? &isVisible_ : nullptr;
    ImGui::SetNextWindowPos(ToImVec(GetPosition()), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ToImVec(GetSize()));
    if (ImGui::Begin(label_.c_str(), isOpen, flags)) {

        DrawChildren();

        ImGui::End();
    }
}

}