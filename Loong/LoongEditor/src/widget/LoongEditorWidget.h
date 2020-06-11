//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#pragma once

#include "LoongFoundation/LoongSigslotHelper.h"
#include <string>

namespace Loong::Editor {

class LoongEditorWidget : public Foundation::LoongHasSlots {
public:
    explicit LoongEditorWidget(const std::string& name = "");
    LoongEditorWidget(const LoongEditorWidget&) = delete;
    LoongEditorWidget(LoongEditorWidget&&) = delete;
    ~LoongEditorWidget() override = default;
    LoongEditorWidget& operator=(const LoongEditorWidget&) = delete;
    LoongEditorWidget& operator=(LoongEditorWidget&&) = delete;

    void Draw();

    LOONG_DECLARE_SIGNAL(OnClick, LoongEditorWidget*);

protected:
    virtual void DrawImpl() = 0;

protected:
    std::string widgetId_ {};
    std::string name_ {};
};

}