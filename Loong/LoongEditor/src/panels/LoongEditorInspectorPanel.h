//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#pragma once

#include "LoongEditorPanel.h"

namespace Loong::Editor {

class LoongEditorInspectorPanel : public LoongEditorPanel {
public:
    explicit LoongEditorInspectorPanel(LoongEditor* editor, const std::string& name = "", bool opened = true, const LoongEditorPanelConfig& cfg = {})
        : LoongEditorPanel(editor, name, opened, cfg)
    {
    }

protected:
    void UpdateImpl(const Foundation::LoongClock& clock) override;
};

}