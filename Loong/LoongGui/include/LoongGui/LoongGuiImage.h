//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#pragma once
#include "LoongFoundation/LoongMacros.h"
#include "LoongGui/LoongGuiWidget.h"
#include "LoongResource/LoongTexture.h"

namespace Loong::Gui {

class LoongGuiImage : public LoongGuiWidget {
    LOONG_GUI_OBJECT(LoongGuiImage, "Image", LoongGuiWidget);

public:
    LG_NODISCARD Resource::LoongTextureRef GetTexture() const { return texture_; }

    void SetTexture(Resource::LoongTextureRef texture) { texture_ = std::move(texture); }

protected:
    void DrawThis() override;

protected:
    Resource::LoongTextureRef texture_ { nullptr };
};

}