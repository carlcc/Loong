//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#pragma once

#include "LoongFoundation/LoongSigslotHelper.h"
#include <string>

namespace Loong::Window {
class LoongWindow;
}
namespace Loong::Editor {

class LoongEditorProjectManager : public Foundation::LoongHasSlots {
public:
    explicit LoongEditorProjectManager(Window::LoongWindow* window);
    bool Initialize();

    const std::string& GetSelectedPath() const { return selectedPath_; }

protected:
    void OnClose();
    void OnUpdate();

    Window::LoongWindow* window_ { nullptr };
    std::string selectedPath_ {};
};

}