//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#pragma once

#include "LoongFoundation/LoongClock.h"
#include "LoongFoundation/LoongMath.h"
#include "LoongFoundation/LoongSigslotHelper.h"
#include "LoongResource/LoongGpuBuffer.h"
#include "LoongResource/LoongMaterial.h"
#include <memory>
#include <string>
#include <utility>

namespace Loong::Core {
class LoongScene;
class LoongActor;
}

namespace Loong::Editor {

class LoongEditorContext final : public Foundation::LoongHasSlots {
public:
    struct UniformBlock {
        Math::Matrix4 ub_Model;
        Math::Matrix4 ub_View;
        Math::Matrix4 ub_Projection;
        Math::Vector3 ub_ViewPos;
        float ub_Time;
    };

    explicit LoongEditorContext(const std::string& projectFile);
    LoongEditorContext(const LoongEditorContext&) = delete;
    LoongEditorContext(LoongEditorContext&&) = delete;
    ~LoongEditorContext() override = default;
    LoongEditorContext& operator=(const LoongEditorContext&) = delete;
    LoongEditorContext& operator=(LoongEditorContext&&) = delete;

    const std::string& GetProjectDirPath() const { return projectDir_; }

    std::shared_ptr<Resource::LoongUniformBuffer> GetUniformBuffer() const { return uniformBuffer_; }

    Foundation::LoongClock& GetGameClock() { return gameClock_; }

    Foundation::LoongClock& GetEditorClock() { return editorClock_; }

    std::shared_ptr<Core::LoongScene> GetCurrentScene() const { return currentScene_; }

    void SetCurrentScene(std::shared_ptr<Core::LoongScene> scene) { currentScene_ = std::move(scene); }

    void SetCurrentSelectedActor(Core::LoongActor* actor) { currentSelectedActor_ = actor; }

    Core::LoongActor* GetCurrentSelectedActor() const { return currentSelectedActor_; }

private:
    std::string projectFile_ {};
    std::string projectDir_ {};
    std::shared_ptr<Resource::LoongUniformBuffer> uniformBuffer_ {};
    std::shared_ptr<Resource::LoongMaterial> defaultMaterial_ { nullptr };
    Foundation::LoongClock gameClock_ {};
    Foundation::LoongClock editorClock_ {};
    std::shared_ptr<Core::LoongScene> currentScene_ { nullptr };
    Core::LoongActor* currentSelectedActor_ { nullptr };
};

}