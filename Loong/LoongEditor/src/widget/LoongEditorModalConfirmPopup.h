//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#pragma once

#include <functional>
#include <string>

namespace Loong::Editor {

class LoongEditorModalConfirmPopup {
public:
    LoongEditorModalConfirmPopup() = default;
    virtual ~LoongEditorModalConfirmPopup() = default;
    LoongEditorModalConfirmPopup(const LoongEditorModalConfirmPopup&) = delete;
    LoongEditorModalConfirmPopup(LoongEditorModalConfirmPopup&&) = delete;
    LoongEditorModalConfirmPopup& operator=(const LoongEditorModalConfirmPopup&) = delete;
    LoongEditorModalConfirmPopup& operator=(LoongEditorModalConfirmPopup&&) = delete;

    void Show() { show_ = true; }

    void SetConfirmedTask(std::function<void()>&& task) { confirmedTask_ = std::move(task); }

    void SetMessage(const std::string& message) { message_ = message; }

    void Draw();

public:
    std::function<void()> confirmedTask_ { nullptr };
    std::string message_ {};
    bool show_ { false };
};

}