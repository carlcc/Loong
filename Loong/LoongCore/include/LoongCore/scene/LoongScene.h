//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#pragma once

#include "LoongCore/scene/LoongActor.h"
#include <functional>
#include <unordered_set>

namespace Loong::Resource {
class LoongMaterial;
}
namespace Loong::Renderer {
class LoongRenderer;
}

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

protected:
    LoongScene(uint32_t actorID, std::string name, std::string tag)
        : LoongActor(actorID, std::move(name), std::move(tag))
    {
    }

public:

    void AddModelRenderer(LoongCModelRenderer* modelRenderer) { fastAccess_.modelRenderers_.insert(modelRenderer); }

    void RemoveModelRenderer(LoongCModelRenderer* modelRenderer) { fastAccess_.modelRenderers_.erase(modelRenderer); }

    void AddCamera(LoongCCamera* camera) { fastAccess_.cameras_.insert(camera); }

    void RemoveCamera(LoongCCamera* camera) { fastAccess_.cameras_.erase(camera); }

    void RecursiveAddToFastAccess(LoongActor* actor);

    void RecursiveRemoveFromFastAccess(LoongActor* actor);

    LoongCCamera* GetFirstActiveCamera();

    using SetModelMatrixCallback = std::function<void(const Math::Matrix4& modelMatrix)>;
    void Render(Renderer::LoongRenderer& renderer, LoongCCamera& camera, const Resource::LoongMaterial* defaultMaterial, const SetModelMatrixCallback& onSetModelMatrix);

    static std::unique_ptr<LoongActor> CreateActor(const std::string& name, const std::string& tag = "");

    static std::unique_ptr<LoongScene> CreateScene(const std::string& name, const std::string& tag = "");

private:
    void ConstructFastAccess();

private:
    FastAccess fastAccess_ {};

    friend class LoongActor;
};

}