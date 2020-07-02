//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#include "LoongCore/render/LoongRenderPassScenePass.h"
#include "LoongCore/scene/components/LoongCCamera.h"
#include "LoongCore/scene/components/LoongCLight.h"
#include "LoongCore/scene/components/LoongCModelRenderer.h"
#include "LoongRenderer/LoongRenderer.h"
#include "LoongResource/LoongGpuMesh.h"
#include "LoongResource/LoongMaterial.h"

namespace Loong::Core {

void LoongRenderPassScenePass::Render(const Context& context)
{
    struct ScenePassDrawable {
        const Math::Matrix4* transform;
        const Resource::LoongGpuMesh* mesh;
        const Resource::LoongMaterial* material;
        float distance;
    };

    auto& camera = *context.camera;
    auto& scene = *context.scene;
    auto& renderer = *context.renderer;
    auto& basicUniforms = *context.basicUniforms;

    BasicUBO ub {};
    ub.ub_ViewPos = camera.GetOwner()->GetTransform().GetWorldPosition();
    ub.ub_Projection = camera.GetCamera().GetProjectionMatrix();
    ub.ub_View = camera.GetCamera().GetViewMatrix();

    std::vector<ScenePassDrawable> opaqueDrawables;
    std::vector<ScenePassDrawable> transparentDrawables;

    auto& cameraActor = *camera.GetOwner();
    auto& cameraActorTransform = cameraActor.GetTransform();

    // Prepare drawables
    for (auto* modelRenderer : scene.GetFastAccess().modelRenderers_) {
        // TODO: cull the objects cannot be seen

        ScenePassDrawable drawable {};
        auto* actor = modelRenderer->GetOwner();
        auto& actorTransform = actor->GetTransform();
        drawable.transform = &actorTransform.GetWorldTransformMatrix();
        drawable.distance = Math::Distance(actorTransform.GetWorldPosition(), cameraActorTransform.GetWorldPosition());

        auto gpuModel = modelRenderer->GetModel();
        if (gpuModel == nullptr) {
            continue;
        }
        auto& materials = modelRenderer->GetMaterials();
        auto& meshes = gpuModel->GetMeshes();
        size_t meshCount = meshes.size();
        for (size_t i = 0; i < meshCount; ++i) {
            drawable.mesh = meshes[i];
            drawable.material = materials[drawable.mesh->GetMaterialIndex()].get();
            if (drawable.material == nullptr || !drawable.material->HasShader()) {
                drawable.material = defaultMaterial_.get();
                if (drawable.material == nullptr || !drawable.material->HasShader()) {
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

    if (shouldRenderCamera_ && cameraModel_ != nullptr && cameraMaterial_ != nullptr && cameraMaterial_->HasShader()) {
        // TODO: cull the objects cannot be seen
        for (auto* cam : scene.GetFastAccess().cameras_) {
            ScenePassDrawable drawable {};
            auto* actor = cam->GetOwner();
            auto& actorTransform = actor->GetTransform();
            drawable.transform = &actorTransform.GetWorldTransformMatrix();
            drawable.distance = Math::Distance(actorTransform.GetWorldPosition(), cameraActorTransform.GetWorldPosition());
            drawable.material = cameraMaterial_.get();

            auto& meshes = cameraModel_->GetMeshes();
            for (auto* mesh : meshes) {
                drawable.mesh = mesh;
                transparentDrawables.push_back(drawable);
            }
        }
    }

    // sort
    std::sort(opaqueDrawables.begin(), opaqueDrawables.end(), [](const ScenePassDrawable& a, const ScenePassDrawable& b) -> bool {
        return a.distance < b.distance;
    });
    std::sort(transparentDrawables.begin(), transparentDrawables.end(), [](const ScenePassDrawable& a, const ScenePassDrawable& b) -> bool {
        return a.distance > b.distance;
    });

    if (auto* lightUniforms = context.lightUniforms; lightUniforms != nullptr) {
        LightUBO lightUbo {};
        int lightsCount = 0;
        for (auto& light : scene.GetFastAccess().lights_) {
            if (lightsCount >= kMaxLightCount) {
                break;
            }
            auto& lightInfo = lightUbo.ub_lights[lightsCount++];
            lightInfo.lightType = float(light->GetType());
            lightInfo.color = light->GetColor();
            lightInfo.dir = light->GetOwner()->GetTransform().GetWorldForward();
            lightInfo.pos = light->GetOwner()->GetTransform().GetWorldPosition();
            lightInfo.intencity = light->GetIntensity();
            lightInfo.falloffRadius = light->GetFalloffRadius();
            lightInfo.innerAngle = light->GetInnerAngle();
            lightInfo.outerAngle = light->GetOuterAngle();
        }
        lightUbo.ub_lightsCount = float(lightsCount);
        lightUbo.padding1_[0] = float(lightsCount);
        lightUbo.padding1_[2] = float(lightsCount);
        lightUbo.padding1_[1] = float(lightsCount);

        lightUniforms->SetSubData(&lightUbo, 0);
    }

    // render
    for (auto& drawable : opaqueDrawables) {
        ub.ub_Model = *drawable.transform;
        basicUniforms.SetSubData(&ub, 0);
        drawable.material->Bind(nullptr);
        renderer.ApplyStateMask(drawable.material->GenerateStateMask());

        renderer.Draw(*drawable.mesh);
    }
    for (auto& drawable : transparentDrawables) {
        ub.ub_Model = *drawable.transform;
        basicUniforms.SetSubData(&ub, 0);
        drawable.material->Bind(nullptr);
        renderer.ApplyStateMask(drawable.material->GenerateStateMask());

        renderer.Draw(*drawable.mesh);
    }
}

}
