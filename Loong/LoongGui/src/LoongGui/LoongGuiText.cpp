//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#include "LoongGui/LoongGuiText.h"
#include <imgui.h>

namespace Loong::Gui {

void LoongGuiText::Draw()
{
    if (!IsVisible()) {
        return;
    }

    ImGui::Text("%s", label_.c_str());
}
}