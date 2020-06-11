//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#include "LoongEditorWidget.h"

namespace Loong::Editor {

static int64_t gWidgetIdCounter = 0;

LoongEditorWidget::LoongEditorWidget(const std::string& name)
{
    widgetId_ = "###w_" + std::to_string(gWidgetIdCounter++);
    name_ = name;
}

void LoongEditorWidget::Draw()
{
    DrawImpl();
}

}