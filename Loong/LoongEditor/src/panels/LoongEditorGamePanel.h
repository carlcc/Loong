//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#pragma once

#include "LoongEditorRenderPanel.h"

namespace Loong::Editor {

class LoongEditorGamePanel : public LoongEditorRenderPanel {
public:
    explicit LoongEditorGamePanel(LoongEditor* editor, const std::string& name = "", bool opened = true, const LoongEditorPanelConfig& cfg = {})
        : LoongEditorRenderPanel(editor, name, opened, cfg)
    {
    }

    void Render(const Foundation::LoongClock& clock) override;

private:
};

}