//
// Copyright (c) carlcc. All rights reserved.
//
#pragma once

#include "LoongFoundation/LoongMath.h"
#include <imgui.h>

namespace Loong::Editor::ImGuiUtils {

inline ImVec2 ToImVec(const Math::Vector2& v)
{
    return ImVec2 { v.x, v.y };
}
inline Math::Vector2 ToVector2(const ImVec2& v)
{
    return Math::Vector2 { v.x, v.y };
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