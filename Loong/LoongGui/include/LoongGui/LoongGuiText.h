//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#pragma once

#include "LoongGui/LoongGuiWidget.h"

namespace Loong::Gui {

class LoongGuiText : public LoongGuiWidget {
    LOONG_GUI_OBJECT(LoongGuiText, "Text", LoongGuiWidget);

public:
    void SetNoWrap(bool b) { isNowrap_ = b; }
    LG_NODISCARD bool IsNoWrap() const { return isNowrap_; }

    void SetUseCustomColor(bool b) { useCustomColor_ = b; }
    LG_NODISCARD bool IsUseCustomColor() const { return useCustomColor_; }

    void SetColor(const Math::Vector4& c) { color_ = c; }
    LG_NODISCARD const Math::Vector4& GetColor() const { return color_; }

protected:
    void DrawThis() override;

protected:
    bool isNowrap_ { true };
    bool useCustomColor_ { false };
    Math::Vector4 color_ {};
};

}