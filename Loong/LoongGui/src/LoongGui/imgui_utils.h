//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#pragma once

#include "LoongFoundation/LoongMath.h"
#include <imgui.h>

namespace Loong::Gui {

inline bool operator==(const Math::Vector2& v2, const ImVec2& imvec)
{
    return imvec.x == v2.x && imvec.y == v2.y;
}

inline bool operator==(const ImVec2& imvec, const Math::Vector2& v2)
{
    return v2 == imvec;
}

inline bool operator!=(const Math::Vector2& v2, const ImVec2& imvec)
{
    return !(imvec == v2);
}

inline bool operator!=(const ImVec2& imvec, const Math::Vector2& v2)
{
    return !(imvec == v2);
}

inline ImVec2 ToImVec2(const Math::Vector2& v2)
{
    return { v2.x, v2.y };
}

inline Math::Vector2 ToVector2(ImVec2& imvec)
{
    return { imvec.x, imvec.y };
}

inline ImVec4 ToImVec4(const Math::Vector4& v4)
{
    return { v4.x, v4.y, v4.z, v4.w };
}

inline Math::Vector4 ToVector4(ImVec4& imvec)
{
    return { imvec.x, imvec.y, imvec.z, imvec.w };
}

}