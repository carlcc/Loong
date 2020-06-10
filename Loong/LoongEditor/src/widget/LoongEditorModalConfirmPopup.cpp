//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#include "LoongEditorModalConfirmPopup.h"
#include <imgui.h>

namespace Loong::Editor {

void LoongEditorModalConfirmPopup::Draw()
{
    if (!show_) {
        return;
    }
    ImGui::OpenPopup("Warning");
    if (ImGui::BeginPopupModal("Warning", &show_, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("%s", message_.c_str());
        ImGui::Separator();

        if (ImGui::Button("Yes", ImVec2(120, 0))) {
            // Continue the job
            ImGui::CloseCurrentPopup();
            show_ = false;
            confirmedTask_();
        }
        ImGui::SetItemDefaultFocus();
        ImGui::SameLine();
        if (ImGui::Button("No", ImVec2(120, 0))) {
            ImGui::CloseCurrentPopup();
            show_ = false;
        }
        ImGui::EndPopup();
    }
}

}