//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//
#include "LoongGui/LoongGuiWindow.h"
#include <imgui.h>

namespace Loong::Gui {

void LoongGuiWindow::DrawThis()
{
    bool* visible = isClosable_ ? (bool*)&isVisible_ : nullptr;
    if (ImGui::Begin(labelAndId_.c_str(), visible, 0)) {
        LoongGuiContainer::DrawThis();
    }
    ImGui::End();
}

}