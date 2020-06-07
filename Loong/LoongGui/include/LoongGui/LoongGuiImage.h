//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#pragma once

#include <utility>

#include "LoongGui/LoongGuiWidget.h"

namespace Loong::Resource {
class LoongTexture;
}

namespace Loong::Gui {

class LoongGuiImage : public LoongGuiWidget {
public:
    explicit LoongGuiImage(const std::string& label = "")
        : LoongGuiWidget("")
    {
    }

    void Draw() override;

    void SetTexture(std::shared_ptr<Resource::LoongTexture> texture) { texture_ = std::move(texture); }

protected:
    std::shared_ptr<Resource::LoongTexture> texture_ { nullptr };
};

}