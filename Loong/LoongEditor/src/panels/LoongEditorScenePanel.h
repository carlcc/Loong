//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#pragma once

#include "LoongEditorRenderPanel.h"

namespace Loong::Core {
class LoongRenderPassIdPass;
}

namespace Loong::Resource {
class LoongShader;
}

namespace Loong::Editor {

class LoongEditorScenePanel : public LoongEditorRenderPanel {
public:
    explicit LoongEditorScenePanel(LoongEditor* editor, const std::string& name = "", bool opened = true, const LoongEditorPanelConfig& cfg = {});

    void Render(const Foundation::LoongClock& clock) override;

protected:
    void UpdateImpl(const Foundation::LoongClock& clock) override;

private:
    void UpdateButtons(const Foundation::LoongClock& clock);
    void UpdateGizmo(const Foundation::LoongClock& clock);

private:
    std::shared_ptr<Core::LoongRenderPassIdPass> idPass_ { nullptr };
    std::shared_ptr<Resource::LoongShader> wireframeShader_ { nullptr };
};

}