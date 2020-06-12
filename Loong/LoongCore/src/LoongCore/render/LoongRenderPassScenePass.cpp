//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#include "LoongCore/render/LoongRenderPassScenePass.h"
#include "LoongCore/scene/components/LoongCCamera.h"
#include "LoongCore/scene/components/LoongCModelRenderer.h"
#include "LoongRenderer/LoongRenderer.h"
#include "LoongResource/LoongMaterial.h"

namespace Loong::Core {

struct Drawable {
    const Math::Matrix4* transform;
    const Resource::LoongGpuMesh* mesh;
    const Resource::LoongMaterial* material;
    float distance;
};

void LoongRenderPassScenePass::Render(Renderer::LoongRenderer& renderer, Resource::LoongUniformBuffer& basicUniforms, LoongScene& scene, LoongCCamera& camera)
{
    UniformBlock ub {};
    ub.ub_ViewPos = camera.GetOwner()->GetTransform().GetWorldPosition();
    ub.ub_Projection = camera.GetCamera().GetProjectionMatrix();
    ub.ub_View = camera.GetCamera().GetViewMatrix();

    std::vector<Drawable> opaqueDrawables;
    std::vector<Drawable> transparentDrawables;

    auto& cameraActor = *camera.GetOwner();
    auto& cameraActorTransform = cameraActor.GetTransform();

    // Prepare drawables
    for (auto* modelRenderer : scene.GetFastAccess().modelRenderers_) {
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
            drawable.material = modelRenderer->GetMaterials()[i].get();
            if (drawable.material == nullptr) {
                drawable.material = defaultMaterial_.get();
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
        ub.ub_Model = *drawable.transform;
        basicUniforms.SetSubData(&ub, 0);
        drawable.material->Bind(nullptr);
        renderer.ApplyStateMask(drawable.material->GenerateStateMask());

        renderer.Draw(*drawable.mesh);
    }
}

}
