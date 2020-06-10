//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#include "LoongCore/scene/components/LoongCLight.h"
#include "LoongCore/scene/LoongScene.h"

namespace Loong::Core {

LoongCLight::LoongCLight(LoongActor* owner)
    : LoongComponent(owner)
{
    if (auto* scene = dynamic_cast<LoongScene*>(owner->GetRoot()); scene != nullptr) {
        scene->AddLight(this);
    }
}

LoongCLight::~LoongCLight()
{
    if (auto* scene = dynamic_cast<LoongScene*>(GetOwner()->GetRoot()); scene != nullptr) {
        scene->RemoveLight(this);
    }
}

}
