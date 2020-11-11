//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#pragma once

#include "LoongGui/LoongGuiText.h"

namespace Loong::Gui {

class LoongGuiButton : public LoongGuiText {
    LOONG_GUI_OBJECT(LoongGuiButton, "Button", LoongGuiText);

public:
    LOONG_DECLARE_SIGNAL(OnClicked, LoongGuiWidget*); // The sender

protected:
    void DrawThis() override;
};

}