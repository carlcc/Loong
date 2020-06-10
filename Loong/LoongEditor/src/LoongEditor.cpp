//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#include "LoongEditor.h"
#include "LoongApp/LoongApp.h"

namespace Loong::Editor {

LoongEditor::LoongEditor(Loong::App::LoongApp* app)
{
    app_ = app;
    app->SubscribeRender(this, &LoongEditor::OnRender);
    app->SubscribeUpdate(this, &LoongEditor::OnUpdate);
    app->SubscribeLateUpdate(this, &LoongEditor::OnLateUpdate);
    app->SubscribeFrameBufferResize(this, &LoongEditor::OnFrameBufferResize);
}

void LoongEditor::OnUpdate()
{
}

void LoongEditor::OnRender()
{
}

void LoongEditor::OnLateUpdate()
{
}

void LoongEditor::OnFrameBufferResize(int width, int height)
{
}

}
