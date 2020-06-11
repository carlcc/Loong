//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#include "LoongCore/scene/LoongScene.h"
#include "LoongCore/scene/components/LoongCCamera.h"
#include "LoongCore/scene/components/LoongCLight.h"
#include "LoongCore/scene/components/LoongCModelRenderer.h"
#include "LoongFoundation/LoongMath.h"
#include "LoongRenderer/LoongRenderer.h"
#include "LoongResource/LoongGpuMesh.h"
#include "LoongResource/LoongGpuModel.h"
#include "LoongResource/LoongMaterial.h"
#include <algorithm>
#include <cassert>

namespace Loong::Core {

void LoongScene::FastAccess::AbsorbAnother(const LoongScene::FastAccess& another)
{
    modelRenderers_.insert(another.modelRenderers_.begin(), another.modelRenderers_.end());
    cameras_.insert(another.cameras_.begin(), another.cameras_.end());
    lights_.insert(another.lights_.begin(), another.lights_.end());
}

void LoongScene::FastAccess::SubtractAnother(const LoongScene::FastAccess& another)
{
    for (auto* modelRenderer : another.modelRenderers_) {
        modelRenderers_.erase(modelRenderer);
    }
    for (auto* camera : another.cameras_) {
        cameras_.erase(camera);
    }
    for (auto* light : another.lights_) {
        lights_.erase(light);
    }
}

void LoongScene::FastAccess::Clear()
{
    modelRenderers_.clear();
    cameras_.clear();
    lights_.clear();
}

void RecursiveAdd(LoongScene::FastAccess& access, LoongActor* actor)
{
    if (auto* modelRenderer = actor->GetComponent<LoongCModelRenderer>(); modelRenderer != nullptr) {
        access.modelRenderers_.insert(modelRenderer);
    }
    if (auto* camera = actor->GetComponent<LoongCCamera>(); camera != nullptr) {
        access.cameras_.insert(camera);
    }
    if (auto* light = actor->GetComponent<LoongCLight>(); light != nullptr) {
        access.lights_.insert(light);
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

LoongCCamera* LoongScene::GetFirstActiveCamera()
{
    for (auto* camera : fastAccess_.cameras_) {
        if (camera->IsActive()) {
            return camera;
        }
    }
    return nullptr;
}

struct Drawable {
    const Math::Matrix4* transform;
    const Resource::LoongGpuMesh* mesh;
    const Resource::LoongMaterial* material;
    float distance;
};

void LoongScene::Render(Renderer::LoongRenderer& renderer, LoongCCamera& camera, const Resource::LoongMaterial* defaultMaterial, const SetModelMatrixCallback& onSetModelMatrix)
{
    std::vector<Drawable> opaqueDrawables;
    std::vector<Drawable> transparentDrawables;

    auto& cameraActor = *camera.GetOwner();
    auto& cameraActorTransform = cameraActor.GetTransform();

    // Prepare drawables
    for (auto* modelRenderer : fastAccess_.modelRenderers_) {
        // TODO: cull the objects cannot be seen

        Drawable drawable {};
        auto* actor = modelRenderer->GetOwner();
        auto& actorTransform = actor->GetTransform();
        drawable.transform = &actorTransform.GetWorldTransformMatrix();
        drawable.distance = Math::Distance(actorTransform.GetWorldPosition(), cameraActorTransform.GetWorldPosition());

        auto gpuModel = modelRenderer->GetModel();
        if (gpuModel == nullptr) {
            continue;
        }
        auto& meshes = gpuModel->GetMeshes();
        size_t meshCount = meshes.size();
        for (size_t i = 0; i < meshCount; ++i) {
            drawable.mesh = meshes[i];
            auto materialIndex = drawable.mesh->GetMaterialIndex();
            drawable.material = modelRenderer->GetMaterials()[materialIndex].get();
            if (drawable.material == nullptr) {
                drawable.material = defaultMaterial;
                if (drawable.material == nullptr) {
                    continue;
                }
            }
            if (drawable.material->IsBlendable()) {
                transparentDrawables.push_back(drawable);
            } else {
                opaqueDrawables.push_back(drawable);
            }
        }
    }

    // sort
    std::sort(opaqueDrawables.begin(), opaqueDrawables.end(), [](const Drawable& a, const Drawable& b) -> bool {
        return a.distance < b.distance;
    });
    std::sort(transparentDrawables.begin(), transparentDrawables.end(), [](const Drawable& a, const Drawable& b) -> bool {
        return a.distance > b.distance;
    });

    // render
    for (auto& drawable : opaqueDrawables) {
        onSetModelMatrix(*drawable.transform);
        drawable.material->Bind(nullptr);
        renderer.ApplyStateMask(drawable.material->GenerateStateMask());

        renderer.Draw(*drawable.mesh);
    }
}

// TODO: Use an object pool?
static uint32_t gActorIdCounter = 0;
std::unique_ptr<LoongActor> LoongScene::CreateActor(const std::string& name, const std::string& tag)
{
    auto* actor = new LoongActor(++gActorIdCounter, name, tag);
    return std::unique_ptr<LoongActor>(actor);
}

std::unique_ptr<LoongScene> LoongScene::CreateScene(const std::string& name, const std::string& tag)
{
    auto* scene = new LoongScene(++gActorIdCounter, name, tag);
    return std::unique_ptr<LoongScene>(scene);
}

}
