//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#pragma once

#include "LoongEditorPanel.h"
#include <memory>
#include <vector>

namespace Loong::Editor {

class LoongFileTreeNode;

class LoongEditorContentPanel : public LoongEditorPanel {
public:
    LoongEditorContentPanel(LoongEditor* editor, const std::string& name, bool opened, const LoongEditorPanelConfig& cfg);

    LOONG_DECLARE_SIGNAL(OnClickNode, LoongFileTreeNode*);

protected:
    void UpdateImpl(const Foundation::LoongClock& clock) override;

    void DrawNode(LoongFileTreeNode* node, LoongFileTreeNode* currentSelected);

private:
    static void ScanForDir(LoongFileTreeNode* node);
    void RescanProject();

private:
    std::shared_ptr<LoongFileTreeNode> projectRootNode_ { nullptr };
};

}
