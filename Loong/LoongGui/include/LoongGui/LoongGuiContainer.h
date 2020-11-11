//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#pragma once

#include "LoongFoundation/LoongMacros.h"
#include "LoongGui/LoongGuiWidget.h"
#include <memory>
#include <vector>

namespace Loong::Gui {

class LoongGuiContainer : public LoongGuiWidget {
    LOONG_GUI_OBJECT(LoongGuiContainer, "Container", LoongGuiWidget);

public:
    using WidgetRef = std::shared_ptr<LoongGuiWidget>;
    void RemoveChild(const LoongGuiWidget* widget);

    void RemoveAllChildren();

    WidgetRef GetChildByName(const std::string& name, bool recursive = false);

    template <class WidgetType, class... ARGS>
    std::shared_ptr<WidgetType> AddChild(ARGS&&... args)
    {
        auto w = MakeGuiWidget<WidgetType>(std::forward<ARGS>(args)...);
        w->SetParent(this);
        return w;
    }

protected:
    void DrawThis() override;

private:
    void AddChild(LoongGuiWidget* child);
    friend class LoongGuiWidget;

protected:
    std::vector<WidgetRef> children_ {};
};

}