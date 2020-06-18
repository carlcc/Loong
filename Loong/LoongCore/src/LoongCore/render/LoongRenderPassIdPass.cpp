//
// Copyright (c) 2020 Carl Chen All rights reserved.
//

#include "LoongCore/render/LoongRenderPassIdPass.h"
#include "LoongCore/scene/components/LoongCCamera.h"
#include "LoongCore/scene/components/LoongCModelRenderer.h"
#include "LoongRenderer/LoongRenderer.h"
#include "LoongResource/LoongGpuModel.h"
#include "LoongResource/LoongMaterial.h"
#include "LoongResource/LoongResourceManager.h"
#include "LoongResource/LoongShader.h"

namespace Loong::Core {

LoongRenderPassIdPass::LoongRenderPassIdPass()
{
    state_.SetBackCullEnabled(false);
    state_.SetFrontCullEnabled(false);
    state_.SetFrontAndBackCullEnabled(false);
    state_.SetFaceCullEnabled(false);
    state_.SetColorWriteEnabled(true);
    state_.SetDepthWriteEnabled(true);
    state_.SetDepthTestEnabled(true);
    state_.SetBlendEnabled(false);
    sceneIdShader_ = Resource::LoongResourceManager::GetShader("/Shaders/id.glsl");
}

void LoongRenderPassIdPass::Render(const Context& context)
{
    struct IdPassDrawable {
        const Math::Matrix4* transform;
        const Resource::LoongGpuMesh* mesh;
        uint32_t actorId;
        float distance;
    };

    auto& camera = *context.camera;
    auto& scene = *context.scene;
    auto& renderer = *context.renderer;
    auto& basicUniforms = *context.basicUniforms;

    UniformBlock ub {};
    ub.ub_ViewPos = camera.GetOwner()->GetTransform().GetWorldPosition();
    ub.ub_Projection = camera.GetCamera().GetProjectionMatrix();
    ub.ub_View = camera.GetCamera().GetViewMatrix();

    std::vector<IdPassDrawable> allDrawables;

    auto& cameraActor = *camera.GetOwner();
    auto& cameraActorTransform = cameraActor.GetTransform();

    // Prepare drawables
    for (auto* modelRenderer : scene.GetFastAccess().modelRenderers_) {
        // TODO: cull the objects cannot be seen

        IdPassDrawable drawable {};
        auto* actor = modelRenderer->GetOwner();
        auto& actorTransform = actor->GetTransform();
        drawable.transform = &actorTransform.GetWorldTransformMatrix();
        drawable.distance = Math::Distance(actorTransform.GetWorldPosition(), cameraActorTransform.GetWorldPosition());
        drawable.actorId = actor->GetID();

        auto gpuModel = modelRenderer->GetModel();
        if (gpuModel == nullptr) {
            continue;
        }
        for (auto* mesh : gpuModel->GetMeshes()) {
            drawable.mesh = mesh;
            allDrawables.push_back(drawable);
        }
    }

    if (cameraModel_ != nullptr) {
        for (auto* cam : scene.GetFastAccess().cameras_) {
            // TODO: cull the objects cannot be seen
            IdPassDrawable drawable {};
            auto* actor = cam->GetOwner();
            auto& actorTransform = actor->GetTransform();
            drawable.transform = &actorTransform.GetWorldTransformMatrix();
            drawable.distance = Math::Distance(actorTransform.GetWorldPosition(), cameraActorTransform.GetWorldPosition());
            drawable.actorId = actor->GetID();

            for (auto* mesh : cameraModel_->GetMeshes()) {
                drawable.mesh = mesh;
                allDrawables.push_back(drawable);
            }
        }
    }

    // sort
    std::sort(allDrawables.begin(), allDrawables.end(), [](const IdPassDrawable& a, const IdPassDrawable& b) -> bool {
        return a.distance < b.distance;
    });

    renderer.ApplyStateMask(state_);
    sceneIdShader_->Bind();
    // render
    for (auto& drawable : allDrawables) {
        ub.ub_Model = *drawable.transform;
        basicUniforms.SetSubData(&ub, 0);
        sceneIdShader_->SetUniformVec4("u_id", ActorIdToColor(drawable.actorId));

        renderer.Draw(*drawable.mesh);
    }
    sceneIdShader_->Unbind();
}

}