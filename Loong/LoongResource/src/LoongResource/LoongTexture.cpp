
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

void LoongTexture::Resize(uint32_t width, uint32_t height)
{
    Bind();
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    width_ = width;
    height_ = height;
    Unbind();
}

LoongTexture::LoongTexture(uint32_t id, uint32_t width, uint32_t height, uint32_t bytesPerPixel, bool generateMipmap)
    : id_(id)
    , width_(width)
    , height_(height)
    , bytesPerPixel_(bytesPerPixel)
    , isMipmapped_(generateMipmap)
{
}

void LoongTexture::SetFilterMode(LoongTexture::FilterMode min, LoongTexture::FilterMode mag)
{
    Bind();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, static_cast<GLint>(mag));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, static_cast<GLint>(min));
    Unbind();
}

void LoongTexture::SetWrapMode(LoongTexture::WrapMode s, LoongTexture::WrapMode t)
{
    Bind();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, static_cast<GLint>(s));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, static_cast<GLint>(t));
    Unbind();
}

void LoongTexture::SetFilterWrapMode(LoongTexture::FilterMode min, LoongTexture::FilterMode mag, LoongTexture::WrapMode s, LoongTexture::WrapMode t)
{
    Bind();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, static_cast<GLint>(mag));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, static_cast<GLint>(min));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, static_cast<GLint>(s));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, static_cast<GLint>(t));
    Unbind();
}

} // namespace Loong