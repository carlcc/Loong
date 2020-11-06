//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#pragma once

#include "LoongFoundation/LoongMacros.h"
#include "LoongGui/LoongGuiWidget.h"
#include <vector>

namespace Loong::Gui {

class LoongGuiContainer : public LoongGuiWidget {
public:
    void RemoveChild(LoongGuiWidget* widget);

    void RemoveAllChildren();

    template <class WidgetType, class... ARGS>
    WidgetType* AddChild(ARGS&&... args)
    {
        auto* w = new WidgetType(std::forward<ARGS>(args)...);
        w->SetParent(this);
        return w;
    }

protected:
    void DrawThis() override;

private:
    void AddChild(LoongGuiWidget* child, bool owns = false);
    friend class LoongGuiWidget;

protected:
    struct ChildWidget {
        LoongGuiWidget* widget;
        bool owns;
    };
    std::vector<ChildWidget> children_ {};
};

}