//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#pragma once

#include "LoongGui/LoongGuiContainer.h"

namespace Loong::Gui {

class LoongGuiWindow : public LoongGuiContainer {
    LOONG_GUI_OBJECT(LoongGuiWindow, "Window", LoongGuiContainer);

public:
    void DrawThis() override;

    void SetResizable(bool b) { isResizable_ = b; }
    LG_NODISCARD bool IsResizable() const { return isResizable_; }

    void SetClosable(bool b) { isClosable_ = b; }
    LG_NODISCARD bool IsClosable() const { return isClosable_; }

    void SetMovable(bool b) { isMovable_ = b; }
    LG_NODISCARD bool IsMovable() const { return isMovable_; }

    void SetScrollable(bool b) { isScrollable = b; }
    LG_NODISCARD bool IsScrollable() const { return isScrollable; }

    void SetDockable(bool b) { isDockable_ = b; }
    LG_NODISCARD bool IsDockable() const { return isDockable_; }

    void SetHasTitleBar(bool b) { hasTitleBar_ = b; }
    LG_NODISCARD bool HasTitleBar() const { return hasTitleBar_; }

    void SetHasBackground(bool b) { hasBackground_ = b; }
    LG_NODISCARD bool HasBackground() const { return hasBackground_; }

    void SetAllowInputs(bool b) { allowInputs_ = b; }
    LG_NODISCARD bool AllowInputs() const { return allowInputs_; }

    void SetCollapsable(bool b) { isCollapsable_ = true; }
    LG_NODISCARD bool IsCollapsable() const { return isCollapsable_; }

    LOONG_DECLARE_SIGNAL(OnSizeChange, LoongGuiWindow*, const Math::Vector2&); // new size
    LOONG_DECLARE_SIGNAL(OnPositionChange, LoongGuiWindow*, const Math::Vector2&); // new position

protected:
    // window flags
    bool isResizable_ { true };
    bool isClosable_ { true };
    bool isMovable_ { true };
    bool isScrollable { true };
    bool isDockable_ { true };
    bool hasTitleBar_ { true };
    bool hasBackground_ { true };
    bool allowInputs_ { true };
    bool isCollapsable_ { true };

    // window states
    bool isVisible_ { true };
};

}