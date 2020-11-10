//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#pragma once

#include "LoongGui/LoongGuiWidget.h"

namespace Loong::Gui {

class LoongGuiButton : public LoongGuiWidget {
    LOONG_GUI_OBJECT(LoongGuiButton, "Button", LoongGuiWidget);

public:
    LOONG_DECLARE_SIGNAL(OnClicked, LoongGuiWidget*); // The sender

protected:
    void DrawThis() override;
};

}