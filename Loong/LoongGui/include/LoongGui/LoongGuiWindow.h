//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#pragma once

#include "LoongGui/LoongGuiContainer.h"

namespace Loong::Gui {

class LoongGuiWindow : public LoongGuiContainer {
public:
    void DrawThis() override;

protected:
    // window flags
    bool isResizable_ { true };
    bool isClosable_ { true };
    bool isMovable { true };
    bool isScrollable { true };
    bool isDockable { true };
    bool hasTitleBar_ { true };
    bool isCollapsable { true };

    // window states
    bool isVisible_ { true };
};

}