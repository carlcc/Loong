//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#include "LoongCore/scene/LoongScene.h"
#include "LoongCore/scene/components/LoongCModelRenderer.h"
#include <cassert>

namespace Loong::Core {

void LoongScene::FastAccess::AbsorbAnother(const LoongScene::FastAccess& another)
{
    modelRenderers_.insert(another.modelRenderers_.begin(), another.modelRenderers_.end());
}

void LoongScene::FastAccess::SubtractAnother(const LoongScene::FastAccess& another)
{
    for (auto* modelRenderer : another.modelRenderers_) {
        modelRenderers_.erase(modelRenderer);
    }
}

void LoongScene::FastAccess::Clear()
{
    modelRenderers_.clear();
}

void RecursiveAdd(LoongScene::FastAccess& access, LoongActor* actor)
{
    if (auto* modelRenderer = actor->GetComponent<LoongCModelRenderer>(); modelRenderer != nullptr) {
        access.modelRenderers_.insert(modelRenderer);
    }
    for (auto* child : actor->GetChildren()) {
        RecursiveAdd(access, child);
    }
}

void Core::LoongScene::RecursiveAddToFastAccess(LoongActor* actor)
{
    assert(actor != this);
    assert(actor != nullptr);
    if (auto* subScene = dynamic_cast<LoongScene*>(actor); subScene != nullptr) {
        // If the new sub-tree is a scene, we just use it's FastAccess to update this
        fastAccess_.AbsorbAnother(subScene->fastAccess_);
    } else {
        RecursiveAdd(fastAccess_, actor);
    }
}

inline void ConstructFastAccess(LoongScene::FastAccess& access, LoongActor* actor)
{
    access.Clear();
    RecursiveAdd(access, actor);
}

void Core::LoongScene::RecursiveRemoveFromFastAccess(LoongActor* actor)
{
    assert(actor != this);
    assert(actor != nullptr);
    if (auto* subScene = dynamic_cast<LoongScene*>(actor); subScene != nullptr) {
        // If the leaving sub-tree is a scene, we just use it's FastAccess to update this
        fastAccess_.SubtractAnother(subScene->fastAccess_);
    } else {
        FastAccess tmp;
        ::Loong::Core::ConstructFastAccess(tmp, actor);
        fastAccess_.SubtractAnother(tmp);
    }
}

void LoongScene::ConstructFastAccess()
{
    ::Loong::Core::ConstructFastAccess(fastAccess_, this);
}

}
