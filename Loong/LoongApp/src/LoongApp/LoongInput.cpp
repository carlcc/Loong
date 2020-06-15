//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#include "LoongApp/LoongInput.h"

namespace Loong::App {

void LoongInput::BeginFrame()
{
    mouseDelta_ = Math::Zero;
    isKeyPressEvent_.fill(0);
    isKeyReleaseEvent_.fill(0);
    isKeyRepeatEvent_.fill(0);
    isMouseButtonPressEvent_ = 0;
    isMouseButtonReleaseEvent_ = 0;
}

LoongInput::LoongInput()
{
    isKeyPressed_.fill(0);
    isKeyPressEvent_.fill(0);
    isKeyReleaseEvent_.fill(0);
    isKeyRepeatEvent_.fill(0);
}

void LoongInput::SetMouseDownPosition(float x, float y)
{
    mouseDownPosition_ = { x, y };
}

void LoongInput::SetMousePosition(float x, float y)
{
    mouseDelta_ = Math::Vector2 { x, y } - mousePosition_;
    mousePosition_ = { x, y };
}

void LoongInput::SetMousePositionOnly(float x, float y)
{
    mousePosition_ = { x, y };
}

}