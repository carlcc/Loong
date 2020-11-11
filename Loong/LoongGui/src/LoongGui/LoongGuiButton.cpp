//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#include "LoongGui/LoongGuiButton.h"
#include "imgui_utils.h"
#include <imgui.h>

namespace Loong::Gui {

void LoongGuiButton::DrawThis()
{
    if (useCustomColor_) {
        ImGui::PushStyleColor(ImGuiCol_Text, ToImVec4(color_));
    }
    if (ImGui::Button(labelAndId_.c_str(), ToImVec2(size_))) {
        OnClickedSignal_.emit(this);
    }
    if (useCustomColor_) {
        ImGui::PopStyleColor();
    }
}

}