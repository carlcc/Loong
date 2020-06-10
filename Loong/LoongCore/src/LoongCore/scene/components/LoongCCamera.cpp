//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#include "LoongCore/scene/components/LoongCCamera.h"
#include "LoongCore/scene/LoongScene.h"

namespace Loong::Core {

LoongCCamera::LoongCCamera(LoongActor* owner)
    : LoongComponent(owner)
{
    if (auto* scene = dynamic_cast<LoongScene*>(owner->GetRoot()); scene != nullptr) {
        scene->AddCamera(this);
    }
}

LoongCCamera::~LoongCCamera()
{
    if (auto* scene = dynamic_cast<LoongScene*>(GetOwner()->GetRoot()); scene != nullptr) {
        scene->RemoveCamera(this);
    }
}

}
