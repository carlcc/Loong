//
// Copyright (c) 2020 Carl Chen All rights reserved.
//

#include "LoongEditorGizmo.h"
#include "LoongCore/scene/LoongActor.h"
#include "LoongCore/scene/components/LoongCCamera.h"
#include "LoongFoundation/LoongTransform.h"
#include "LoongRenderer/LoongCamera.h"
#include <cassert>
#include <imgui.h>

#include <ImGuizmo.h>

namespace Loong::Editor {

void LoongEditorGizmo::SetDrawList()
{
    ImGuizmo::SetDrawlist();
}

void LoongEditorGizmo::SetViewport(const Math::Vector2& viewportMin, const Math::Vector2& viewportSize)
{
    viewportMin_ = viewportMin;
    viewportSize_ = viewportSize;
}

void LoongEditorGizmo::Manipulate(Core::LoongActor* targetActor)
{
    assert(targetActor != nullptr);
    assert(boundCamera_ != nullptr);
    ImGuizmo::Enable(true);

    auto& actorTransform = targetActor->GetTransform();
    auto actorTransformMatrix = actorTransform.GetWorldTransformMatrix();

    auto& renderCamera = boundCamera_->GetCamera();
    auto& cameraViewMatrix = renderCamera.GetViewMatrix();
    auto& cameraProjMatrix = renderCamera.GetProjectionMatrix();

    ImGuizmo::SetRect(viewportMin_.x, viewportMin_.y, viewportSize_.x, viewportSize_.y);
    ImGuizmo::Manipulate(&cameraViewMatrix[0].x, &cameraProjMatrix[0].x,
        ImGuizmo::OPERATION(manipulateMode_), ImGuizmo::MODE(coordinateMode_), &actorTransformMatrix[0].x);

    Math::Vector3 position {}, scale {};
    Math::Quat rotation {};

    Math::Matrix4 actorLocalTransformMatrx = targetActor->HasParent() ? Math::Inverse(targetActor->GetParent()->GetTransform().GetTransformMatrix()) * actorTransformMatrix : actorTransformMatrix;
    Math::Decompose(actorLocalTransformMatrx, scale, rotation, position);
    actorTransform.SetPosition(position);
    actorTransform.SetRotation(rotation);
    actorTransform.SetScale(scale);
}

void LoongEditorGizmo::ViewManipulate(float targetDistance, const Math::Vector2& drawPosition, const Math::Vector2& size, uint32_t backgroundColor)
{
    assert(boundCamera_ != nullptr);

    auto& renderCamera = boundCamera_->GetCamera();
    auto cameraViewMatrix = renderCamera.GetViewMatrix();

    // The navigating cube at the top-right corner
    // TODO: Set proper length argument, note, this argument should not be 0
    ImGuizmo::ViewManipulate(&cameraViewMatrix[0].x, 0.5, ImVec2(drawPosition.x, drawPosition.y), ImVec2(size.x, size.y), backgroundColor);
    auto cameraTransformMatrix = Math::Inverse(cameraViewMatrix);

    Math::Vector3 position {}, scale {};
    Math::Quat rotation {};
    Math::Decompose(cameraTransformMatrix, scale, rotation, position);

    auto& cameraTransform = boundCamera_->GetOwner()->GetTransform();
    cameraTransform.SetWorldPosition(position);
    cameraTransform.SetWorldRotation(rotation);
}

}