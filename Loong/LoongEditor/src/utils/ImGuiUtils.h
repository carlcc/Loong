//
// Copyright (c) carlcc. All rights reserved.
//
#pragma once

#include "LoongFoundation/LoongMath.h"
#include <imgui.h>

namespace Loong::Editor {
class LoongFileTreeNode;
class LoongEditor;
}

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

void HandleOpenFile(const LoongFileTreeNode* fileNode, LoongEditor* editor);

template <class T>
inline void SetDragData(const char* key, const T& data)
{
    ImGui::SetDragDropPayload(key, &data, sizeof(T));
}

template <class T>
inline T GetDropData(const char* key)
{
    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(key)) {
        IM_ASSERT(payload->DataSize == sizeof(T));
        T data = *static_cast<T*>(payload->Data);
        return data;
    } else {
        return nullptr;
    }
}

constexpr const char* kDragTypeMaterialFile = "LG_MAT_FILE";
constexpr const char* kDragTypeModelFile = "LG_MDL_FILE";
constexpr const char* kDragTypeTextureFile = "LG_TEX_FILE";
constexpr const char* kDragTypeShaderFile = "LG_SHADER_FILE";
constexpr const char* kDragTypeActor = "LG_ACTOR";

}