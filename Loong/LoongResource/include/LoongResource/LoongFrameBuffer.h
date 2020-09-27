//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#pragma once

#include <glad/glad.h>

#include <cstdint>
#include <memory>
#include <vector>

namespace Loong::Resource {

class LoongTexture;

class LoongFrameBuffer {
public:
    using TextureRef = std::shared_ptr<LoongTexture>;

private:
};

}