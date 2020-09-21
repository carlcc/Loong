//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#pragma once

#include "LoongFoundation/LoongSigslotHelper.h"
#include <string>

namespace Loong::App {
class LoongWindow;
}
namespace Loong::Editor {

class LoongEditorProjectManager : public Foundation::LoongHasSlots {
public:
    explicit LoongEditorProjectManager(App::LoongWindow* window);
    bool Initialize();

    const std::string& GetSelectedPath() const { return selectedPath_; }

protected:
    void OnClose();
    void OnUpdate();

    App::LoongWindow* window_ { nullptr };
    std::string selectedPath_ {};
};

}