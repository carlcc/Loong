//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#pragma once

#include "LoongCore/scene/LoongComponent.h"

namespace Loong::Editor {

class LoongEditor;

class LoongEditorSceneCameraController : public Core::LoongComponent {
public:
    LoongEditorSceneCameraController(Core::LoongActor* owner, LoongEditor* editor);
    const std::string& GetName() override
    {
        static const std::string kName("LoongEditorSceneCameraController");
        return kName;
    }
    void OnUpdate(const Foundation::LoongClock& clock) override;

private:
    LoongEditor* editor_ { nullptr };
};

}
