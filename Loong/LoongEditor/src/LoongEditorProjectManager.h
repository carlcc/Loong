//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#pragma once

#include "LoongFoundation/LoongSigslotHelper.h"
#include <string>

namespace Loong::App {
class LoongApp;
}
namespace Loong::Editor {

class LoongEditorProjectManager : public Foundation::LoongHasSlots {
public:
    explicit LoongEditorProjectManager(App::LoongApp* app);
    bool Initialize();

    const std::string& GetSelectedPath() const { return selectedPath_; }

protected:
    void OnClose();
    void OnUpdate();

    App::LoongApp* app_ { nullptr };
    std::string selectedPath_ {};
};

}