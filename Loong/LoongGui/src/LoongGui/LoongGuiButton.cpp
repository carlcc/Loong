//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#include "LoongGui/LoongGuiButton.h"
#include <imgui.h>

namespace Loong::Gui {

void LoongGuiButton::DrawThis()
{
    if (ImGui::Button(labelAndId_.c_str())) {
        OnClickedSignal_.emit(this);
    }
}

}