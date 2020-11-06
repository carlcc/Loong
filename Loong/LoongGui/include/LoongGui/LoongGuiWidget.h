//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#pragma once

#include "LoongFoundation/LoongMacros.h"
#include "LoongFoundation/LoongMath.h"
#include "LoongFoundation/LoongSigslotHelper.h"
#include <atomic>
#include <string>

namespace Loong::Gui {

class LoongGuiContainer;

class LoongGuiWidget {
public:
    explicit LoongGuiWidget();

    void SetLabel(const std::string& label);

    const std::string& GetLabel() { return label_; }

    void SetParent(LoongGuiContainer* parent);

    LG_NODISCARD LoongGuiContainer* GetParent() const { return parent_; }

    LG_NODISCARD bool HasParent() const { return GetParent() != nullptr; }

    virtual void Draw();

    void SetName(const std::string& name) { name_ = name; }

    LG_NODISCARD const std::string& GetName() const { return name_; }

protected:
    virtual void DrawThis() = 0;
    virtual ~LoongGuiWidget() = default;

    friend class LoongGuiContainer;

protected:
    static std::atomic_int64_t sIdCounter;

    const std::string id_ {};
    std::string label_ {};
    std::string labelAndId_ {};
    LoongGuiContainer* parent_ { nullptr };

    std::string name_ {};
    bool isEnabled_ { true };
    bool hasLineBreak_ { true };

    Math::Vector2 position_ { 0.F, 0.F };
    Math::Vector2 minSize { 0.F, 0.F };
    Math::Vector2 maxSize_ { 0.F, 0.F };
};

}