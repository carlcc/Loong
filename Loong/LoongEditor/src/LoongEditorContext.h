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
namespace Loong::Renderer {
class LoongRenderer;
}
namespace Loong::Resource {
class LoongMaterial;
}

namespace Loong::Editor {

class LoongFileTreeNode;

class LoongEditorContext final : public Foundation::LoongHasSlots {
public:
    explicit LoongEditorContext(const std::string& projectFile);
    LoongEditorContext(const LoongEditorContext&) = delete;
    LoongEditorContext(LoongEditorContext&&) = delete;
    ~LoongEditorContext() override = default;
    LoongEditorContext& operator=(const LoongEditorContext&) = delete;
    LoongEditorContext& operator=(LoongEditorContext&&) = delete;

    const std::string& GetProjectDirPath() const { return projectDir_; }

    std::shared_ptr<Resource::LoongUniformBuffer> GetBasicUniformBuffer() const { return basicUniformBuffer_; }

    std::shared_ptr<Resource::LoongUniformBuffer> GetLightUniformBuffer() const { return lightUniformBuffer_; }

    Foundation::LoongClock& GetGameClock() { return gameClock_; }

    Foundation::LoongClock& GetEditorClock() { return editorClock_; }

    std::shared_ptr<Core::LoongScene> GetCurrentScene() const { return currentScene_; }

    void SetCurrentScene(std::shared_ptr<Core::LoongScene> scene);

    void SetCurrentSelectedActor(Core::LoongActor* actor) { currentSelectedActor_ = actor; }

    Core::LoongActor* GetCurrentSelectedActor() const { return currentSelectedActor_; }

    Renderer::LoongRenderer& GetRenderer() const { return *renderer_; }

    std::shared_ptr<Resource::LoongMaterial> GetDefaultMaterial() const { return defaultMaterial_; }

    LoongFileTreeNode* GetCurrentSelectedFileTreeNode() const { return currentSelectedFileTreeNode_; }

    void SetCurrentSelectedFileTreeNode(LoongFileTreeNode* node) { currentSelectedFileTreeNode_ = node; }

private:
    std::string projectFile_ {};
    std::string projectDir_ {};
    std::shared_ptr<Resource::LoongUniformBuffer> basicUniformBuffer_ {};
    std::shared_ptr<Resource::LoongUniformBuffer> lightUniformBuffer_ {};
    std::shared_ptr<Resource::LoongMaterial> defaultMaterial_ { nullptr };
    Foundation::LoongClock gameClock_ {};
    Foundation::LoongClock editorClock_ {};
    std::shared_ptr<Core::LoongScene> currentScene_ { nullptr };
    Core::LoongActor* currentSelectedActor_ { nullptr };
    std::unique_ptr<Renderer::LoongRenderer> renderer_ { nullptr };
    LoongFileTreeNode* currentSelectedFileTreeNode_ { nullptr };
};

}