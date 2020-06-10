//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#include "LoongResource/LoongFrameBuffer.h"
#include <utility>

namespace Loong::Resource {

LoongFrameBuffer::LoongFrameBuffer(uint32_t width, uint32_t height)
{
    glGenFramebuffers(1, &id_);
    glGenTextures(1, &renderTexture_);
    glGenRenderbuffers(1, &depthStencilBuffer_);

    glBindTexture(GL_TEXTURE_2D, renderTexture_);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);

    Bind();
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, renderTexture_, 0);
    Unbind();

    width_ = width;
    height_ = height;
    Resize(width, height);
}

LoongFrameBuffer::LoongFrameBuffer(LoongFrameBuffer&& b) noexcept
{
    std::swap(b.id_, id_);
    std::swap(b.renderTexture_, renderTexture_);
    std::swap(b.depthStencilBuffer_, depthStencilBuffer_);
}

LoongFrameBuffer::~LoongFrameBuffer()
{
    if (id_ != 0) {
        glDeleteBuffers(1, &id_);
        id_ = 0;
    }
    if (renderTexture_ != 0) {
        glDeleteTextures(1, &renderTexture_);
        renderTexture_ = 0;
    }
    if (depthStencilBuffer_ != 0) {
        glGenRenderbuffers(1, &depthStencilBuffer_);
        depthStencilBuffer_ = 0;
    }
}

LoongFrameBuffer& LoongFrameBuffer::operator=(LoongFrameBuffer&& b) noexcept
{
    std::swap(b.id_, id_);
    std::swap(b.renderTexture_, renderTexture_);
    std::swap(b.depthStencilBuffer_, depthStencilBuffer_);
    return *this;
}

void LoongFrameBuffer::Bind() const
{
    glBindFramebuffer(GL_FRAMEBUFFER, id_);
}

void LoongFrameBuffer::Unbind() const
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void LoongFrameBuffer::Resize(uint32_t width, uint32_t height)
{
    if (width == width_ || height == height_) {
        return;
    }

    glBindTexture(GL_TEXTURE_2D, renderTexture_);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
    glBindTexture(GL_TEXTURE_2D, 0);

    glBindRenderbuffer(GL_RENDERBUFFER, depthStencilBuffer_);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_STENCIL, width, height);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    Bind();
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthStencilBuffer_);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depthStencilBuffer_);
    Unbind();
}

}
