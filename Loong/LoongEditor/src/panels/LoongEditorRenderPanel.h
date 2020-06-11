//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#pragma once

#include "LoongEditorPanel.h"
#include <memory>

namespace Loong::Resource {
class LoongFrameBuffer;
}
namespace Loong::Core {
class LoongActor;
class LoongScene;
class LoongCCamera;
}

namespace Loong::Editor {

class LoongEditorRenderPanel : public LoongEditorPanel {
public:
    LoongEditorRenderPanel(LoongEditor* editor, const std::string& name, bool opened, const LoongEditorPanelConfig& cfg);

    std::shared_ptr<Resource::LoongFrameBuffer> GetFrameBuffer() const { return frameBuffer_; }

    uint32_t GetViewportWidth() const { return viewportWidth_; }

    uint32_t GetViewportHeight() const { return viewportHeight_; }

    std::shared_ptr<Core::LoongActor> GetCamera() const { return cameraActor_; }

protected:
    void UpdateImpl(const Foundation::LoongClock& clock) override;

    void RenderSceneForCamera(Core::LoongScene& scene, Core::LoongCCamera& camera);

protected:
    std::shared_ptr<Resource::LoongFrameBuffer> frameBuffer_ { nullptr };
    uint32_t viewportWidth_ { 0 };
    uint32_t viewportHeight_ { 0 };
    // This actor is owned by this panel, instead of the scene tree
    std::shared_ptr<Core::LoongActor> cameraActor_ { nullptr };
};

}
