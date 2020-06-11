//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#pragma once

#include "LoongCore/render/LoongRenderPass.h"
#include <memory>
#include <vector>

namespace Loong::Core {

class LoongRenderPipeline {
public:

private:
    std::vector<std::shared_ptr<LoongRenderPass>> renderPasses_ {};
};

}