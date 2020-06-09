//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#pragma once

#include "LoongCore/scene/LoongActor.h"
#include <unordered_set>

namespace Loong::Core {

class LoongCModelRenderer;

class LoongScene : public LoongActor {
public:
    struct FastAccess {
        std::unordered_set<LoongCModelRenderer*> modelRenderers_;

        void AbsorbAnother(const FastAccess& another);
        void SubtractAnother(const FastAccess& another);
        void Clear();
    };

    LoongScene(uint32_t actorID, std::string name, std::string tag)
        : LoongActor(actorID, std::move(name), std::move(tag))
    {
    }

    void RecursiveAddToFastAccess(LoongActor* actor);

    void RecursiveRemoveFromFastAccess(LoongActor* actor);

private:
    void ConstructFastAccess();

private:
    FastAccess fastAccess_ {};

    friend class LoongActor;
};

}