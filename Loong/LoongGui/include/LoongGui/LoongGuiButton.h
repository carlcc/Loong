//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#pragma once

#include "LoongFoundation/LoongSigslotHelper.h"
#include "LoongGui/LoongGuiWidget.h"

namespace Loong::Gui {

class LoongGuiButton : public LoongGuiWidget {
public:
    explicit LoongGuiButton(const std::string& label);

    void Draw() override;

    LOONG_DECLARE_SIGNAL(OnClick, LoongGuiButton*);
};

}