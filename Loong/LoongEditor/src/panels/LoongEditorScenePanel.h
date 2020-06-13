//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#pragma once

#include "LoongEditorRenderPanel.h"

namespace Loong::Core {
class LoongRenderPassIdPass;
}

namespace Loong::Editor {

class LoongEditorScenePanel : public LoongEditorRenderPanel {
public:
    explicit LoongEditorScenePanel(LoongEditor* editor, const std::string& name = "", bool opened = true, const LoongEditorPanelConfig& cfg = {});

    void Render(const Foundation::LoongClock& clock) override;

protected:
    void UpdateImpl(const Foundation::LoongClock& clock) override;

private:
    std::shared_ptr<Core::LoongRenderPassIdPass> idPass_ { nullptr };
};

}