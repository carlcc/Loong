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
float2 ToFloat2(const ImVec2& v)
{
    return float2 { v.x, v.y };
}

template <class T>
struct ScopedId {
public:
    explicit ScopedId(T id)
    {
        ImGui::PushID(id);
    }
    ~ScopedId()
    {
        ImGui::PopID();
    }
};

}