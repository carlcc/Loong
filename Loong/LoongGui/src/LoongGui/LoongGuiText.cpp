//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#include "LoongGui/LoongGuiText.h"
#include "imgui_utils.h"
#include <imgui.h>

namespace Loong::Gui {

void LoongGuiText::DrawThis()
{
    if (useCustomColor_) {
        ImGui::PushStyleColor(ImGuiCol_Text, ToImVec4(color_));
    }
    if (isNowrap_) {
        ImGui::Text("%s", label_.c_str());
    } else {
        ImGui::TextWrapped("%s", label_.c_str());
    }
    if (useCustomColor_) {
        ImGui::PopStyleColor();
    }
}

}