//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#include "LoongEditorSceneCameraController.h"
#include "../LoongEditor.h"
#include "LoongApp/LoongApp.h"
#include "LoongApp/LoongInput.h"
#include "LoongCore/scene/LoongActor.h"
#include "LoongFoundation/LoongMath.h"

namespace Loong::Editor {

LoongEditorSceneCameraController::LoongEditorSceneCameraController(Core::LoongActor* owner, LoongEditor* editor)
    : Core::LoongComponent(owner)
    , editor_(editor)
{
}

void LoongEditorSceneCameraController::OnUpdate(const Foundation::LoongClock& clock)
{
    auto* actor = GetOwner();
    auto euler = Math::QuatToEuler(actor->GetTransform().GetRotation());
    const auto& input = editor_->GetApp().GetInputManager();

    {
        euler.x -= input.GetMouseDelta().y / 200.0F; // pitch
        euler.y -= input.GetMouseDelta().x / 200.0F; // yaw
        euler.z = 0.0F;
        euler.x = Math::Clamp(euler.x, -float(Math::HalfPi) + 0.1F, float(Math::HalfPi) - 0.1F);

        actor->GetTransform().SetRotation(Math::EulerToQuat(euler));
    }

    {
        Math::Vector3 dir { 0.0F };
        if (input.IsKeyPressed(App::LoongKeyCode::kKeyW)) {
            dir += actor->GetTransform().GetForward();
        }
        if (input.IsKeyPressed(App::LoongKeyCode::kKeyS)) {
            dir -= actor->GetTransform().GetForward();
        }
        if (input.IsKeyPressed(App::LoongKeyCode::kKeyA)) {
            dir -= actor->GetTransform().GetRight();
        }
        if (input.IsKeyPressed(App::LoongKeyCode::kKeyD)) {
            dir += actor->GetTransform().GetRight();
        }
        if (input.IsKeyPressed(App::LoongKeyCode::kKeyE)) {
            dir += Math::kUp;
        }
        if (input.IsKeyPressed(App::LoongKeyCode::kKeyQ)) {
            dir -= Math::kUp;
        }
        if (input.IsKeyPressed(App::LoongKeyCode::kKeyLeftShift) || input.IsKeyPressed(App::LoongKeyCode::kKeyRightShift)) {
            dir *= 5.0F;
        }
        actor->GetTransform().Translate(dir * clock.DeltaTime() * 2.0F);
    }
}

}