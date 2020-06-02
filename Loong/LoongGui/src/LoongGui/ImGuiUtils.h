//
// Copyright (c) carlcc. All rights reserved.
//
#pragma once
#include "LoongGui/LoongGuiElement.h"
#include <imgui.h>

namespace Loong::Gui {

ImVec2 ToImVec(const float2& v)
{
    return ImVec2 { v.x, v.y };
}

}