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
        kBringToFrontOnFocus = 0x10u
    };

public:
    explicit LoongGuiWindow(const std::string& label = "");
    ~LoongGuiWindow() override = default;

    void Draw() override;

    const float2& GetPosition() const { return position_; }

    void SetPosition(const float2& p) { position_ = p; }

    const float2& GetSize() const { return size_; }

    void SetSize(const float2& size) { size_ = size; }

    bool IsFocused();

    void SetFocused();

    LOONG_GUI_DEFINE_FLAG_GETTER_SETTER(Movable, windowFlags_, kMovable)
    LOONG_GUI_DEFINE_FLAG_GETTER_SETTER(Titleless, windowFlags_, kTitleless)
    LOONG_GUI_DEFINE_FLAG_GETTER_SETTER(Resizable, windowFlags_, kResizable)
    LOONG_GUI_DEFINE_FLAG_GETTER_SETTER_HAS(MenuBar, windowFlags_, kHasMenuBar)
    LOONG_GUI_DEFINE_FLAG_GETTER_SETTER(BringToFrontOnFocus, windowFlags_, kBringToFrontOnFocus)

protected:
    float2 position_ { 0.0F, 0.0F };
    float2 size_ { 100.0F, 100.0F };
    uint32_t windowFlags_ { 0 };
};

}