//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#include "LoongCore/scene/LoongComponent.h"
#include "LoongCore/scene/LoongActor.h"

namespace Loong::Core {

bool LoongComponent::IsActive() const
{
    return IsSelfActive() && GetOwner()->IsActive();
}

}