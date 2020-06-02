//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#pragma once

#include "LoongFoundation/LoongSigslotHelper.h"
#include <BasicMath.hpp>
#include <string>

namespace Loong::Gui {

using float2 = Diligent::float2;

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

    LOONG_DECLARE_SIGNAL(OnVisibilityChange, bool); // The parameter is new state

protected:
    std::string label_ {};
    bool isVisible_ { true };
};

}