//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#include "LoongEditorPanel.h"
#include "../LoongEditor.h"
#include <imgui.h>

namespace Loong::Editor {

static size_t gPanelIdCounter = 0;

LoongEditorPanel::LoongEditorPanel(LoongEditor* editor, const std::string& name, bool opened, const LoongEditorPanelConfig& cfg)
{
    editor_ = editor;
    name_ = name;
    isVisible_ = opened;
    config_ = cfg;
    panelId_ = "###p_" + std::to_string(gPanelIdCounter++);
}

void LoongEditorPanel::Show()
{
    SetVisible(true);
}

void LoongEditorPanel::Close()
{
    SetVisible(false);
}

void LoongEditorPanel::Focus()
{
    ImGui::SetWindowFocus((name_ + panelId_).c_str());
}

void LoongEditorPanel::SetVisible(bool value)
{
    if (isVisible_ != value) {
        isVisible_ = value;

        isVisible_ ? OnOpenSignal_.emit() : OnCloseSignal_.emit();
    }
}

void LoongEditorPanel::Update(const Foundation::LoongClock& clock)
{
    uint32_t windowFlags = ImGuiWindowFlags_None;

    // clang-format off
    if (!config_.isResizable)               windowFlags |= ImGuiWindowFlags_NoResize;
    if (!config_.isMovable)                 windowFlags |= ImGuiWindowFlags_NoMove;
    if (config_.isHideBackground)           windowFlags |= ImGuiWindowFlags_NoBackground;
    if (config_.isForceHorizontalScrollbar) windowFlags |= ImGuiWindowFlags_AlwaysHorizontalScrollbar;
    if (config_.isForceVerticalScrollbar)   windowFlags |= ImGuiWindowFlags_AlwaysVerticalScrollbar;
    if (config_.allowHorizontalScrollbar)   windowFlags |= ImGuiWindowFlags_HorizontalScrollbar;
    if (!config_.isBringToFrontOnFocus)     windowFlags |= ImGuiWindowFlags_NoBringToFrontOnFocus;
    if (!config_.isCollapsable)             windowFlags |= ImGuiWindowFlags_NoCollapse;
    if (!config_.allowInputs)               windowFlags |= ImGuiWindowFlags_NoInputs;
    // clang-format on

    // ImVec2 minSizeConstraint = ImGuiHelper::ToImVec(Math::Max(minSize_, { 0.0F, 0.0F }));
    // ImVec2 maxSizeConstraint = ImGuiHelper::ToImVec(Math::Max(maxSize_, { 10000.0F, 10000.0F }));
    // ImGui::SetNextWindowSizeConstraints(minSizeConstraint, maxSizeConstraint);
    isContentVisible_ = ImGui::Begin((name_ + panelId_).c_str(), config_.isClosable ? &isVisible_ : nullptr, windowFlags);
    if (isContentVisible_) {
        isHovered_ = ImGui::IsWindowHovered();
        isFocused_ = ImGui::IsWindowFocused();

        if (!isVisible_) {
            OnCloseSignal_.emit();
        }

        UpdateImpl(clock);
    }

    ImGui::End();
}

bool LoongEditorPanel::IsVisible() const
{
    return isVisible_;
}

bool LoongEditorPanel::IsContentVisible() const
{
    return isContentVisible_;
}

bool LoongEditorPanel::IsHovered() const
{
    return isHovered_;
}

bool LoongEditorPanel::IsFocused() const
{
    return isFocused_;
}

LoongEditorContext& LoongEditorPanel::GetEditorContext()
{
    return editor_->GetContext();
}

App::LoongWindow& LoongEditorPanel::GetApp()
{
    return editor_->GetWindow();
}

}
