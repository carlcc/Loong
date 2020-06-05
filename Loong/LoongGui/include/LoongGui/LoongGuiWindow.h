//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#pragma once

#include "LoongGui/LoongGuiContainer.h"
#include <vector>

namespace Loong::Gui {

class LoongGuiWindow : public LoongGuiContainer {
    enum WindowFlag : uint32_t {
        kMovable = 0x01u,
        kTitleless = 0x02u,
        kResizable = 0x04u,
        kHasMenuBar = 0x08u,
        kBringToFrontOnFocus = 0x10u,
        kDockable = 0x20u,
    };

public:
    explicit LoongGuiWindow(const std::string& label = "");
    ~LoongGuiWindow() override = default;

    void Draw() override;

    const float2& GetPosition() const { return position_; }

    void SetPosition(const float2& p) { position_ = p; }

    bool IsFocused();

    void SetFocused();

    LOONG_GUI_DEFINE_FLAG_GETTER_SETTER(Movable, windowFlags_, kMovable)
    LOONG_GUI_DEFINE_FLAG_GETTER_SETTER(Titleless, windowFlags_, kTitleless)
    LOONG_GUI_DEFINE_FLAG_GETTER_SETTER(Resizable, windowFlags_, kResizable)
    LOONG_GUI_DEFINE_FLAG_GETTER_SETTER_HAS(MenuBar, windowFlags_, kHasMenuBar)
    LOONG_GUI_DEFINE_FLAG_GETTER_SETTER(BringToFrontOnFocus, windowFlags_, kBringToFrontOnFocus)
    LOONG_GUI_DEFINE_FLAG_GETTER_SETTER(Dockable, windowFlags_, kDockable)

protected:
    float2 position_ { 0.0F, 0.0F };
    uint32_t windowFlags_ { 0 };
};

}