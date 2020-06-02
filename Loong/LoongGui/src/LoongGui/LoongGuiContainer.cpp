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

void LoongGuiContainer::DrawChildren()
{
    for (auto* child : children_) {
        child->Draw();
    }
}

}