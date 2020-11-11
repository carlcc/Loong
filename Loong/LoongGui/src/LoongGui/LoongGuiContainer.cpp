//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//
#include "LoongGui/LoongGuiContainer.h"
#include "LoongFoundation/LoongAssert.h"
#include <algorithm>

namespace Loong::Gui {

void LoongGuiContainer::AddChild(LoongGuiWidget* child)
{
    LOONG_ASSERT(child->GetParent() == this, "child's parent is not me");
    children_.push_back(child->SharedFromThis());
}

void LoongGuiContainer::RemoveChild(const LoongGuiWidget* widget)
{
    LOONG_ASSERT(widget->GetParent() == this, "This widget is not my child");
    auto it = std::find_if(children_.begin(), children_.end(), [widget](const WidgetRef& c) { return c.get() == widget; });
    if (it != children_.end()) {
        children_.erase(it);
    }
}

void LoongGuiContainer::RemoveAllChildren()
{
    children_.clear();
}

void LoongGuiContainer::DrawThis()
{
    for (auto& child : children_) {
        child->Draw();
    }
}

LoongGuiContainer::WidgetRef LoongGuiContainer::GetChildByName(const std::string& name, bool recursive)
{
    if (recursive) {
        for (auto& child : children_) {
            if (child->GetName() == name) {
                return child;
            }
            if (child->IsInstanceOf<LoongGuiContainer>()) {
                auto w = static_cast<LoongGuiContainer*>(child.get())->GetChildByName(name, true); // NOLINT(cppcoreguidelines-pro-type-static-cast-downcast)
                if (w != nullptr) {
                    return w;
                }
            }
        }
    } else {
        for (auto& child : children_) {
            if (child->GetName() == name) {
                return child;
            }
        }
    }
    return nullptr;
}

}