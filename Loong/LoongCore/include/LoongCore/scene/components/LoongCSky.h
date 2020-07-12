//
// Copyright (c) 2020 Carl Chen All rights reserved.
//

#pragma once

#include "LoongCore/scene/LoongComponent.h"
#include <memory>
#include <utility>

namespace Loong::Resource {
class LoongMaterial;
}

namespace Loong::Core {

class LoongCSky : public LoongComponent {
public:
    explicit LoongCSky(Core::LoongActor* owner);

    const std::string& GetName() override
    {
        static const std::string kName("Sky");
        return kName;
    }

    void SetSkyMaterial(std::shared_ptr<Resource::LoongMaterial> sky) { skyMaterial_ = std::move(sky); }

    std::shared_ptr<Resource::LoongMaterial> GetSkyMaterial() { return skyMaterial_; }

private:
    std::shared_ptr<Resource::LoongMaterial> skyMaterial_ { nullptr };
};

}
