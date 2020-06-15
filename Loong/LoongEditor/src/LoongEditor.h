//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#pragma once

#include "LoongEditorContext.h"
#include "LoongFoundation/LoongSigslotHelper.h"
#include "widget/LoongEditorModalConfirmPopup.h"
#include <functional>
#include <memory>
#include <queue>
#include <unordered_map>

namespace Loong::App {
class LoongApp;
}

namespace Loong::Editor {

class LoongEditorPanel;

class LoongEditor : public Foundation::LoongHasSlots {
public:
    using EndFrameTask = std::function<void()>; // Tasks execute on end of logic frame (before rendering)
    using PanelMap = std::unordered_map<std::string, std::shared_ptr<LoongEditorPanel>>;

    explicit LoongEditor(Loong::App::LoongApp* app, const std::shared_ptr<LoongEditorContext>& context);

    bool Initialize();

    void OnBeginFrame();

    void OnUpdate();

    void OnRender();

    void OnLateUpdate();

    void OnFrameBufferResize(int width, int height);

    template <class T>
    T* GetPanel()
    {
        static_assert(std::is_base_of_v<LoongEditorPanel, std::remove_cv_t<T>>);
        for (auto& [name, panel] : panels_) {
            T* p = dynamic_cast<T*>(panel.get());
            if (p != nullptr) {
                return p;
            }
        }
        assert(false);
        return nullptr;
    }

    App::LoongApp& GetApp() const { return *app_; }

    LoongEditorContext& GetContext() const { return *context_; }

    void AddEndFrameTask(EndFrameTask&& task) { endFrameTaskQueue_.push(std::move(task)); }

private:
    void SetupDockSpace();

    void OnUpdateMainMenuBar();

    void DoEndFrameTasks();

private:
    App::LoongApp* app_ { nullptr };
    std::shared_ptr<LoongEditorContext> context_ { nullptr };
    std::queue<EndFrameTask> endFrameTaskQueue_ {};
    LoongEditorModalConfirmPopup confirmPopup_ {};
    PanelMap panels_ {};
};

}