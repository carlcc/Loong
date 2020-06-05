//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#pragma once

#include "LoongFoundation/LoongMath.h"
#include "LoongFoundation/LoongSigslotHelper.h"
#include <string>

namespace Loong::Gui {

using float2 = Loong::Math::Vector2;

#define LOONG_GUI_DEFINE_FLAG_GETTER_SETTER(funcName, flagVar, flagName) \
    bool Is##funcName() const { return flagVar & flagName; }             \
    void Set##funcName(bool b)                                           \
    {                                                                    \
        if (b) {                                                         \
            flagVar |= flagName;                                         \
        } else {                                                         \
            flagVar &= ~flagName;                                        \
        }                                                                \
    }

#define LOONG_GUI_DEFINE_FLAG_GETTER_SETTER_HAS(funcName, flagVar, flagName) \
    bool Has##funcName() const { return flagVar & flagName; }                \
    void SetHas##funcName(bool b)                                            \
    {                                                                        \
        if (b) {                                                             \
            flagVar |= flagName;                                             \
        } else {                                                             \
            flagVar &= ~flagName;                                            \
        }                                                                    \
    }

class LoongGuiElement {
public:
    explicit LoongGuiElement(const std::string& label = "")
    {
        SetVisible(true);
        SetLabel(label);
    }
    virtual ~LoongGuiElement() = default;

    virtual void Draw() = 0;

    void SetLabel(const std::string& label) { label_ = label; }

    const std::string& GetLabel() const { return label_; }

    bool IsVisible() const { return isVisible_; }
    void SetVisible(bool b)
    {
        if (isVisible_ != b) {
            isVisible_ = b;
            OnVisibilityChangeSignal_.emit(b);
        }
    }

    const float2& GetSize() const { return size_; }

    void SetSize(const float2& size) { size_ = size; }

    LOONG_DECLARE_SIGNAL(OnVisibilityChange, bool); // The parameter is new state

protected:
    std::string label_ {};
    float2 size_ { 100.0F, 100.0F };
    bool isVisible_ { true };
};

}