//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#include "LoongGui/LoongGuiButton.h"
#include "ImGuiUtils.h"
#include <imgui.h>

namespace Loong::Gui {

LoongGuiButton::LoongGuiButton(const std::string& label)
    : LoongGuiWidget(label)
{
    SetSize({0.0F, 0.0F});
}

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
