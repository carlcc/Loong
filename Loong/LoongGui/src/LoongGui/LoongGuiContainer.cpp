//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//
#include "LoongGui/LoongGuiContainer.h"
#include "LoongFoundation/LoongAssert.h"
#include <algorithm>

namespace Loong::Gui {

void LoongGuiContainer::AddChild(LoongGuiWidget* child, bool owns)
{
    LOONG_ASSERT(child->GetParent() == this, "child's parent is not me");
    children_.push_back({ child, owns });
}

void LoongGuiContainer::RemoveChild(LoongGuiWidget* widget)
{
    LOONG_ASSERT(widget->GetParent() == this, "This widget is not my child");
    auto it = std::find_if(children_.begin(), children_.end(), [widget](const ChildWidget& c) { return c.widget == widget; });
    if (it != children_.end()) {
        if (it->owns) {
            delete it->widget;
        }
        children_.erase(it);
    }
}

void LoongGuiContainer::RemoveAllChildren()
{
    for (auto& child : children_) {
        if (child.owns) {
            delete child.widget;
        }
    }
    children_.clear();
}

void LoongGuiContainer::DrawThis()
{
    for (auto& child : children_) {
        child.widget->Draw();
    }
}

}