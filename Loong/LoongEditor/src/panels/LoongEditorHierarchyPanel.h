//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#pragma once

#include "LoongEditorPanel.h"
#include <memory>

namespace Loong::Core {
class LoongActor;
}

namespace Loong::Editor {

class LoongEditorHierarchyPanel : public LoongEditorPanel {
public:
    explicit LoongEditorHierarchyPanel(LoongEditor* editor, const std::string& name = "", bool opened = true, const LoongEditorPanelConfig& cfg = {});

    LOONG_DECLARE_SIGNAL(OnClickNode, Core::LoongActor*);

protected:
    void UpdateImpl(const Foundation::LoongClock& clock) override;

private:

    void DrawNode(Core::LoongActor* node, Core::LoongActor* currentSelected);

};

}