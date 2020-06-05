//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#pragma once

#include "LoongGui/LoongGuiWidget.h"

namespace Loong::Gui {

class LoongGuiText : public LoongGuiWidget {
public:
    explicit LoongGuiText(const std::string& label)
        : LoongGuiWidget(label)
    {
    }

    void Draw() override;
};

}