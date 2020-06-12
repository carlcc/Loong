//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#include "LoongCore/render/LoongRenderPass.h"
#include "LoongResource/LoongFrameBuffer.h"

namespace Loong::Core {

LoongRenderPass::LoongRenderPass()
{
    frameBuffer_ = std::make_shared<Resource::LoongFrameBuffer>();
}

}
