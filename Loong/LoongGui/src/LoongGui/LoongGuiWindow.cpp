//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//
#include "LoongGui/LoongGuiWindow.h"
#include "imgui_utils.h"
#include <imgui.h>

namespace Loong::Gui {

void LoongGuiWindow::DrawThis()
{
    bool* visible = isClosable_ ? (bool*)&isVisible_ : nullptr;
    if (sizeChangedByApi_) {
        ImGui::SetNextWindowSize(ToImVec2(size_), ImGuiCond_Always);
        sizeChangedByApi_ = false;
    }
    if (posChangedByApi_) {
        ImGui::SetNextWindowPos(ToImVec2(position_), ImGuiCond_Always);
        posChangedByApi_ = false;
    }
    int windowFlags = ImGuiWindowFlags_None;

    // clang-format off
    if (!isResizable_)   { windowFlags |= ImGuiWindowFlags_NoResize;                                         }
    if (!isMovable_)     { windowFlags |= ImGuiWindowFlags_NoMove;                                           }
    if (!isDockable_)    { windowFlags |= ImGuiWindowFlags_NoDocking;                                        }
    if (!hasBackground_) { windowFlags |= ImGuiWindowFlags_NoBackground;                                     }
    if (!isCollapsable_) { windowFlags |= ImGuiWindowFlags_NoCollapse;                                       }
    if (!allowInputs_)   { windowFlags |= ImGuiWindowFlags_NoInputs;                                         }
    if (!isScrollable)   { windowFlags |= ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar; }
    if (!hasTitleBar_)   { windowFlags |= ImGuiWindowFlags_NoTitleBar;                                       }
    // clang-format on

    if (ImGui::Begin(labelAndId_.c_str(), visible, windowFlags)) {
        LoongGuiContainer::DrawThis();

        auto newPos = ImGui::GetWindowPos();
        if (position_ != newPos) {
            auto p = ToVector2(newPos);
            position_ = p;
            OnPositionChangeSignal_.emit(this, p);
        }

        auto newSize = ImGui::GetWindowSize();
        if (size_ != newSize) {
            auto s = ToVector2(newSize);
            size_ = s;
            OnSizeChangeSignal_.emit(this, s);
        }
    }
    ImGui::End();
}

}