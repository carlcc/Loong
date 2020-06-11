//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#include "LoongEditorProjectManager.h"
#include "LoongApp/LoongApp.h"
#include "LoongFoundation/LoongDefer.h"
#include "LoongFoundation/LoongLogger.h"
#include <imgui.h>
#include <nfd.h>

namespace Loong::Editor {

LoongEditorProjectManager::LoongEditorProjectManager(App::LoongApp* app)
{
    app_ = app;
    app->SubscribeUpdate(this, &LoongEditorProjectManager::OnUpdate);
    app->SubscribeWindowClose(this, &LoongEditorProjectManager::OnClose);
}

bool LoongEditorProjectManager::Initialize()
{
    return true;
}

void LoongEditorProjectManager::OnClose()
{
}

void LoongEditorProjectManager::OnUpdate()
{
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize
        | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse
        | ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_NoDocking;

    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->GetWorkPos());
    ImGui::SetNextWindowSize(viewport->GetWorkSize());
    if (ImGui::Begin("LoongEditorProjectManager", nullptr, flags)) {
        if (ImGui::Button("Open")) {
            const nfdchar_t* filterList = nullptr;
            const nfdchar_t* defaultPath = nullptr;
            nfdchar_t* outPath = nullptr;

            // TODO: Determine if the project file is valid
            // Consider use another thread? NFD will block this thread
            auto result = NFD_OpenDialog(filterList, defaultPath, &outPath);
            OnScopeExit { free(outPath); }; // NFDi_Free
            if (result == nfdresult_t::NFD_OKAY) {
                LOONG_INFO("Open project '{}'", outPath);
                selectedPath_ = outPath;
                app_->SetShouldClose(true);
            } else {
                LOONG_ERROR("Selecting project was canceled or an error occured", outPath);
                // TODO: handler error
            }
        }
    }
    ImGui::End();
}

}
