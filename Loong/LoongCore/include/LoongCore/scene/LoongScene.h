//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#pragma once

#include "LoongCore/scene/LoongActor.h"
#include <unordered_set>

namespace Loong::Core {

class LoongCModelRenderer;
class LoongCCamera;

class LoongScene : public LoongActor {
public:
    struct FastAccess {
        std::unordered_set<LoongCModelRenderer*> modelRenderers_;
        std::unordered_set<LoongCCamera*> cameras_;

        void AbsorbAnother(const FastAccess& another);
        void SubtractAnother(const FastAccess& another);
        void Clear();
    };

    LoongScene(uint32_t actorID, std::string name, std::string tag)
        : LoongActor(actorID, std::move(name), std::move(tag))
    {
    }

    void AddModelRenderer(LoongCModelRenderer* modelRenderer) { fastAccess_.modelRenderers_.insert(modelRenderer); }

    void RemoveModelRenderer(LoongCModelRenderer* modelRenderer) { fastAccess_.modelRenderers_.erase(modelRenderer); }

    void AddCamera(LoongCCamera* camera) { fastAccess_.cameras_.insert(camera); }

    void RemoveCamera(LoongCCamera* camera) { fastAccess_.cameras_.erase(camera); }

    void RecursiveAddToFastAccess(LoongActor* actor);

    void RecursiveRemoveFromFastAccess(LoongActor* actor);

    LoongCCamera* GetFirstActiveCamera();

private:
    void ConstructFastAccess();

private:
    FastAccess fastAccess_ {};

    friend class LoongActor;
};

}