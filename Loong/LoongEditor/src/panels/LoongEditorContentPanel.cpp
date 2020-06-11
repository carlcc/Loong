//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#include "LoongEditorContentPanel.h"
#include "../LoongEditorContext.h"
#include "../utils/ImGuiUtils.h"
#include "../utils/LoongFileTreeNode.h"
#include "LoongFileSystem/LoongFileSystem.h"
#include "LoongFoundation/LoongDefer.h"
#include <imgui.h>
#include <vector>

namespace Loong::Editor {

LoongEditorContentPanel::LoongEditorContentPanel(LoongEditor* editor, const std::string& name, bool opened, const LoongEditorPanelConfig& cfg)
    : LoongEditorPanel(editor, name, opened, cfg)
{
    projectRootNode_ = std::make_shared<LoongFileTreeNode>("/");
    projectRootNode_->isDir = true;
    projectRootNode_->isScanned = false;
    SubscribeOnClickNode(GetEditorContext(), &LoongEditorContext::SetCurrentSelectedFileTreeNode);
}

void LoongEditorContentPanel::UpdateImpl(const Foundation::LoongClock& clock)
{
    ImGui::PushID(this);
    OnScopeExit { ImGui::PopID(); };
    if (ImGui::Button("Rescan")) {
        RescanProject();
    }
    DrawNode(projectRootNode_.get(), GetEditorContext().GetCurrentSelectedFileTreeNode());
}

static void MarkTreeToNotScanned(LoongFileTreeNode* node)
{
    node->isScanned = false;
    for (const auto& c : node->children) {
        MarkTreeToNotScanned(c.get());
    }
}

void LoongEditorContentPanel::RescanProject()
{
    // only mark scanned to false
    // TODO: keep the old nodes if they are not changed, so we dont need to change the selected node
    GetEditorContext().SetCurrentSelectedFileTreeNode(nullptr);
    MarkTreeToNotScanned(projectRootNode_.get());
}

void LoongEditorContentPanel::ScanForDir(LoongFileTreeNode* node)
{
    auto fullPath = node->GetFullPath();
    if (FS::LoongFileSystem::IsDir(fullPath)) {
        std::vector<std::string> files;
        FS::LoongFileSystem::EnumerateFiles(fullPath, [&files](const std::string& name) {
            if (name != "." && name != "..") {
                files.push_back(name);
            }
            return false;
        });
        node->children.clear();
        for (auto& f : files) {
            auto newNode = std::make_shared<LoongFileTreeNode>(f, node);
            newNode->isDir = FS::LoongFileSystem::IsDir(newNode->GetFullPath());

            node->children.push_back(newNode);
        }
        std::sort(node->children.begin(), node->children.end(), [](const std::shared_ptr<LoongFileTreeNode>& a, const std::shared_ptr<LoongFileTreeNode>& b) {
            if (a->isDir == b->isDir) {
                return a->fileName < b->fileName;
            }
            return a->isDir > b->isDir;
        });
    }
    node->isScanned = true;
}

static const char* kGlslCode = R"(#shader vertex
#version 330 core
layout (location = 0) in vec3 v_Pos;
layout (location = 1) in vec2 v_Uv;
layout (location = 2) in vec3 v_Normal;
layout (location = 3) in vec3 v_Tan;
layout (location = 4) in vec3 v_BiTan;

layout (std140) uniform EngineUBO
{
    mat4    ub_Model;
    mat4    ub_View;
    mat4    ub_Projection;
    vec3    ub_ViewPos;
    float   ub_Time;
};

out VS_OUT
{
    vec2 Uv;
} vs_out;

void main()
{
    vs_out.Uv = v_Uv;

    gl_Position = ub_Projection * ub_View * ub_Model * vec4(v_Pos, 1.0);
}

#shader fragment
#version 330 core

out vec4 outColor;

in VS_OUT
{
    vec2 Uv;
} fs_in;

void main()
{
    outColor = vec4(1.0, 1.0, 1.0, 1.0);
})";
void LoongEditorContentPanel::DrawNode(LoongFileTreeNode* node, LoongFileTreeNode* currentSelected)
{
    // clang-format off
    ImGuiTreeNodeFlags     flags = ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_OpenOnArrow;
    if (node == currentSelected)     flags |= ImGuiTreeNodeFlags_Selected;
    if (!node->isDir) flags |= ImGuiTreeNodeFlags_Leaf;
    // clang-format on

    ImGui::PushID(node->fileName.c_str());
    OnScopeExit { ImGui::PopID(); };

    bool opened = ImGui::TreeNodeEx((node->fileName).c_str(), flags);

    if (ImGui::IsItemClicked() && (ImGui::GetMousePos().x - ImGui::GetItemRectMin().x) > ImGui::GetTreeNodeToLabelSpacing()) {
        OnClickNodeSignal_.emit(node);
    }
    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {
        ImGui::Text("%s", node->fileName.c_str());
        // TODO: Use different key or diffrend file type?
        ImGuiUtils::SetDragData(ImGuiUtils::kDragTypeFile, node);
        ImGui::EndDragDropSource();
    }

    if (ImGui::BeginPopupContextItem("ContentContextMenu")) {
        if (ImGui::MenuItem("Open", nullptr, nullptr, !node->isDir)) {
            ImGuiUtils::HandleOpenFile(node, editor_);
        }
        // if (ImGui::BeginMenu("Create", node->isDir)) {
        //     if (ImGui::BeginMenu("Material")) {
        //         static char fileNameBuf[512];
        //         if (ImGui::InputText("", fileNameBuf, sizeof(fileNameBuf), ImGuiInputTextFlags_EnterReturnsTrue)) {
        //             std::string file = node->GetFullPath() + '/' + fileNameBuf + ".ssmat";
        //             // TODO: Show a message if file exists
        //             if (!FileSystem::Exists(file.c_str())) {
        //                 Utils::WriteFile(file, "{}");
        //                 node->isScanned = false;
        //             }
        //         }
        //         ImGui::EndMenu();
        //     }
        //     if (ImGui::BeginMenu("Shader")) {
        //         static char fileNameBuf[512];
        //         if (ImGui::InputText("", fileNameBuf, sizeof(fileNameBuf), ImGuiInputTextFlags_EnterReturnsTrue)) {
        //             std::string file = node->GetFullPath() + '/' + fileNameBuf + ".glsl";
        //             // TODO: Show a message if file exists
        //             if (!FileSystem::Exists(file.c_str())) {
        //                 Utils::WriteFile(file, kGlslCode);
        //                 node->isScanned = false;
        //             }
        //         }
        //         ImGui::EndMenu();
        //     }
        //     ImGui::EndMenu();
        // }

        ImGui::EndPopup();
    }

    // NOTE: Must place this before traversing it's children
    // ImGuiHelper::ShowActorContextMenu(node, scene_.get(), editor_);

    if (opened) {
        if (!node->isScanned) {
            ScanForDir(node);
        }
        for (const auto& child : node->children) {
            DrawNode(child.get(), currentSelected);
        }
        ImGui::TreePop();
    }
}

}