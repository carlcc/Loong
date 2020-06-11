//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#include "LoongEditor.h"
#include "LoongApp/LoongApp.h"
#include "LoongCore/scene/LoongScene.h"
#include "LoongFoundation/LoongClock.h"
#include "panels/LoongEditorContentPanel.h"
#include "panels/LoongEditorGamePanel.h"
#include "panels/LoongEditorHierarchyPanel.h"
#include "panels/LoongEditorInspectorPanel.h"
#include "panels/LoongEditorMaterialEditorPanel.h"
#include "panels/LoongEditorPanel.h"
#include "panels/LoongEditorScenePanel.h"
#include "utils/LoongEditorTemplates.h"
#include <imgui.h>

namespace Loong::Editor {

LoongEditor::LoongEditor(Loong::App::LoongApp* app, const std::shared_ptr<LoongEditorContext>& context)
{
    app_ = app;
    app->SubscribeRender(this, &LoongEditor::OnRender);
    app->SubscribeUpdate(this, &LoongEditor::OnUpdate);
    app->SubscribeLateUpdate(this, &LoongEditor::OnLateUpdate);
    app->SubscribeFrameBufferResize(this, &LoongEditor::OnFrameBufferResize);
    context_ = context;
}

struct PanelMaker {
    PanelMaker(LoongEditor* editor, LoongEditor::PanelMap& panels)
        : editor_(editor)
        , panels_(panels)
    {
    }
    template <class T>
    void MakePanel(const std::string& name, bool opened = true, const LoongEditorPanelConfig& cfg = {})
    {
        panels_.insert({ name, std::make_shared<T>(editor_, name, opened, cfg) });
    }

    LoongEditor* editor_ { nullptr };
    LoongEditor::PanelMap& panels_;
};

bool LoongEditor::Initialize()
{
    PanelMaker panelMaker(this, panels_);
    panelMaker.MakePanel<LoongEditorHierarchyPanel>("Hierarchy");
    panelMaker.MakePanel<LoongEditorScenePanel>("Scene");
    panelMaker.MakePanel<LoongEditorInspectorPanel>("Inspector");
    panelMaker.MakePanel<LoongEditorContentPanel>("Content");
    panelMaker.MakePanel<LoongEditorGamePanel>("Game");
    panelMaker.MakePanel<LoongEditorMaterialEditorPanel>("Material");

    return true;
}

bool showImGuiDemoWindow_ = true;
void LoongEditor::OnUpdate()
{
    auto& editorClock = GetContext().GetEditorClock();
    editorClock.Update();
    
    SetupDockSpace();

    if (showImGuiDemoWindow_) {
        ImGui::ShowDemoWindow(&showImGuiDemoWindow_);
    }

    for (auto& [name, panel] : panels_) {
        if (panel->IsVisible()) {
            panel->Update(editorClock);
        }
    }
    // NOTE: Since the frame buffer's size may be changed during update,
    // which will make our rendered image invalid if we first Render then Update.
    // So... Let's put Render after Update
    for (auto& [name, panel] : panels_) {
        if (panel->IsVisible()) {
            panel->Render(editorClock);
        }
    }

    confirmPopup_.Draw();
    DoEndFrameTasks();
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

void LoongEditor::SetupDockSpace()
{
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
    window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
    window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->GetWorkPos());
    ImGui::SetNextWindowSize(viewport->GetWorkSize());
    ImGui::SetNextWindowViewport(viewport->ID);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

    // When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background
    // and handle the pass-thru hole, so we ask Begin() to not render a background.
    //    if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
    // window_flags |= ImGuiWindowFlags_NoBackground;

    // Important: note that we proceed even if Begin() returns false (aka window is collapsed).
    // This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
    // all active windows docked into it will lose their parent and become undocked.
    // We cannot preserve the docking relationship between an active window and an inactive docking, otherwise
    // any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

    bool open = true;
    ImGui::Begin("SPARKLE_STUDIO_ROOT_DOCKSPACE", &open, window_flags);
    ImGui::PopStyleVar();

    ImGui::PopStyleVar(2);

    ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
    ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);

    OnUpdateMainMenuBar();
    ImGui::End();
}

void LoongEditor::OnUpdateMainMenuBar()
{
    ImGui::PushID(this);
    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("New Scene")) {
                auto createNewScene = [this]() {
                    std::shared_ptr<Core::LoongScene> newScene;
                    newScene.reset(Core::LoongScene::CreateScene("Root").release());
                    context_->SetCurrentScene(newScene);
                };
                if (GetContext().GetCurrentScene() != nullptr) {
                    confirmPopup_.Show();
                    confirmPopup_.SetMessage("There is a scene opened, you may lost your modification if you continue,\n are you sure to continue?");
                    confirmPopup_.SetConfirmedTask(createNewScene);
                } else {
                    createNewScene();
                }
            }

            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Create")) {
            LoongEditorTemplates::FillActorMenu(GetContext().GetCurrentSelectedActor(), GetContext().GetCurrentScene().get(), this);
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Window")) {
            for (auto& [name, panel] : panels_) {
                bool isVisible = panel->IsVisible();
                if (ImGui::Checkbox(name.c_str(), &isVisible)) {
                    panel->SetVisible(isVisible);
                }
            }
            ImGui::EndMenu();
        }

        ImGui::EndMenuBar();
    }

    ImGui::PopID();
}

void LoongEditor::DoEndFrameTasks()
{
    while (!endFrameTaskQueue_.empty()) {
        auto& task = endFrameTaskQueue_.front();
        task();
        endFrameTaskQueue_.pop();
    }
}

}
