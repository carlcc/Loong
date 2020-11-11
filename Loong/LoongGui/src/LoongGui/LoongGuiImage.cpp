//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//
#include "LoongGui/LoongGuiImage.h"
#include <imgui.h>

namespace Loong::Gui {

LoongGuiImage::LoongGuiImage()
{
    size_ = { 100.F, 100.F };
}

void LoongGuiImage::DrawThis()
{
    // TODO: display a black texture if this->texture_ is null
    RHI::ITextureView* tv = texture_ == nullptr ? nullptr : texture_->GetTexture()->GetDefaultView(RHI::TEXTURE_VIEW_SHADER_RESOURCE);
    if (tv != nullptr) {
        ImGui::Image(tv, ImVec2 { size_.x, size_.y });
    }
}

}
