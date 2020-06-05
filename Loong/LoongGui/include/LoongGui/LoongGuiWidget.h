//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#pragma once

#include "LoongGui/LoongGuiElement.h"

namespace Loong::Gui {

class LoongGuiWidget : public LoongGuiElement {
public:
    explicit LoongGuiWidget(const std::string& label = "")
        : LoongGuiElement(label)
    {
    }
};

}