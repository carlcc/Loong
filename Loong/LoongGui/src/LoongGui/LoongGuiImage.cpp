//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#include "LoongGui/LoongGuiImage.h"
#include "ImGuiUtils.h"
#include "LoongResource/LoongTexture.h"
#include <imgui.h>

namespace Loong::Gui {

void LoongGuiImage::Draw()
{
    if (!IsVisible()) {
        return;
    }

    ScopedId scopedId(this);
    ImGui::Image(ImTextureID(intptr_t(texture_->GetId())), ToImVec(size_));
}

}