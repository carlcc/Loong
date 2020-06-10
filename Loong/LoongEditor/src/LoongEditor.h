//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#pragma once

#include "LoongFoundation/LoongSigslotHelper.h"
#include "LoongGui/LoongGuiWindow.h"
#include <memory>
#include <vector>

namespace Loong::App {
class LoongApp;
}

namespace Loong::Editor {

class LoongEditor : public Foundation::LoongHasSlots {
public:
    explicit LoongEditor(Loong::App::LoongApp* app);

    void OnUpdate();

    void OnRender();

    void OnLateUpdate();

    void OnFrameBufferResize(int width, int height);

private:
    float clearColor_[4] { 0.3F, 0.4F, 0.5F, 1.0F };
    App::LoongApp* app_ { nullptr };
    std::vector<std::shared_ptr<Gui::LoongGuiWindow>> panels_ {};
};

}