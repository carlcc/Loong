//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#pragma once
#include "LoongEditorWidget.h"
#include <memory>
#include <string>
#include <vector>

namespace Loong::Editor {

// TODO: Inherit from a container widget
class EditorTreeNodeWidget : public LoongEditorWidget {
public:
    explicit EditorTreeNodeWidget(const std::string& name = "")
        : LoongEditorWidget(name)
    {
    }

    LOONG_DECLARE_SIGNAL(OnExpand, EditorTreeNodeWidget*);
    LOONG_DECLARE_SIGNAL(OnCollapse, EditorTreeNodeWidget*);

    void AddChild(const std::shared_ptr<LoongEditorWidget>& child) { children_.push_back(child); }

protected:
    void DrawImpl() override;

protected:
    bool isSelected_ { false };
    bool isLeaf_ { false };

    bool arrowClickToOpen_ { false };
    bool shouldOpen_ { false };
    bool shouldClose_ { false };
    bool isOpened_ { false };

    std::vector<std::shared_ptr<LoongEditorWidget>> children_ {};
};

}