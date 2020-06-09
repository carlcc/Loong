//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#include "LoongCore/scene/components/LoongCModelRenderer.h"
#include "LoongCore/scene/LoongActor.h"
#include "LoongCore/scene/LoongScene.h"

namespace Loong::Core {

LoongCModelRenderer::LoongCModelRenderer(LoongActor* owner)
    : LoongComponent(owner)
{
    if (auto* scene = dynamic_cast<LoongScene*>(owner->GetRoot()); scene != nullptr) {
        scene->AddModelRenderer(this);
    }
}

LoongCModelRenderer::~LoongCModelRenderer()
{
    if (auto* scene = dynamic_cast<LoongScene*>(GetOwner()->GetRoot()); scene != nullptr) {
        scene->RemoveModelRenderer(this);
    }
}

}
