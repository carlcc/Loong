//
// Copyright (c) 2020 Carl Chen All rights reserved.
//

#pragma once

#include "LoongFoundation/LoongMath.h"
#include <cstdint>

namespace Loong::Core {
class LoongActor;
class LoongCCamera;
}

namespace Loong::Editor {

class LoongEditorGizmo {
public:
    enum class ManipulateMode {
        kTranslate = 0,
        kRotate = 1,
        kScale = 2,
        kBound = 3,
    };
    enum class CoordinateMode {
        kLocal = 0,
        kWorld = 1,
    };

    ManipulateMode GetManipulateMode() const { return manipulateMode_; }
    void SetManipulateMode(ManipulateMode manipulateMode) { manipulateMode_ = manipulateMode; }

    CoordinateMode GetCoordinateMode() const { return coordinateMode_; }
    void SetCoordinateMode(CoordinateMode coordinateMode) { coordinateMode_ = coordinateMode; }

    void SetDrawList();

    void SetBoundCamera(Core::LoongCCamera* camera) { boundCamera_ = camera; }

    void SetViewport(const Math::Vector2& viewportMin, const Math::Vector2& viewportSize);

    void Manipulate(Core::LoongActor* targetActor);

    void ViewManipulate(float targetDistance, const Math::Vector2& drawPosition, const Math::Vector2& size, uint32_t backgroundColor);

private:
    ManipulateMode manipulateMode_ { ManipulateMode::kTranslate };
    CoordinateMode coordinateMode_ { CoordinateMode::kLocal };
    Core::LoongCCamera* boundCamera_ { nullptr };
    Math::Vector2 viewportMin_ {};
    Math::Vector2 viewportSize_ {};
};

}
