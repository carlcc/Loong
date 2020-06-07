
//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//
#include "LoongResource/LoongTexture.h"

namespace Loong::Resource {

LoongTexture::~LoongTexture()
{
    if (0 != id_) {
        glDeleteTextures(1, &id_);
        id_ = 0;
    }
}

void LoongTexture::Bind(uint32_t slot) const
{
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, id_);
}

void LoongTexture::Unbind() const
{
    glBindTexture(GL_TEXTURE_2D, 0);
}

LoongTexture::LoongTexture(uint32_t id, uint32_t width, uint32_t height, uint32_t bytesPerPixel, bool generateMipmap)
    : id_(id)
    , width_(width)
    , height_(height)
    , bytesPerPixel_(bytesPerPixel)
    , isMipmapped_(generateMipmap)
{
}

} // namespace Loong