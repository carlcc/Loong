//
// Copyright (c) carlcc. All rights reserved.
//

#include "LoongGui/LoongGuiContainer.h"

namespace Loong::Gui {

LoongGuiContainer::~LoongGuiContainer()
{
    ClearChildren();
}

void LoongGuiContainer::ClearChildren()
{
    for (auto* c : children_) {
        delete c;
    }
    children_.clear();
}

bool LoongGuiContainer::RemoveChild(const LoongGuiElement* child)
{
    auto* c = ReleaseChild(child);
    if (c == nullptr) {
        return false;
    }
    delete c;
    return true;
}

LoongGuiElement* LoongGuiContainer::ReleaseChild(const LoongGuiElement* child)
{
    auto it = std::find_if(children_.begin(), children_.end(), [child](LoongGuiElement* c) -> bool {
        return c == child;
    });
    if (it != children_.end()) {
        return nullptr;
    }
    children_.erase(it);
    return *it;
}

void LoongGuiContainer::DrawChildren()
{
    for (auto* child : children_) {
        child->Draw();
    }
}

}