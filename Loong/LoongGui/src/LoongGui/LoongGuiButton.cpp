//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#include "LoongGui/LoongGuiButton.h"
#include "ImGuiUtils.h"
#include <imgui.h>

namespace Loong::Gui {

void LoongGuiButton::Draw()
{
    if (!IsVisible()) {
        return;
    }

    ScopedId scopedId(this);
    if (ImGui::Button(label_.c_str(), ToImVec(size_))) {
        OnClickSignal_.emit(this);
    }
}

}
