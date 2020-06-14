//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#include "LoongResource/LoongFrameBuffer.h"
#include "LoongResource/LoongTexture.h"
#include "LoongResource/loader/LoongTextureLoader.h"
#include <utility>

namespace Loong::Resource {

LoongFrameBuffer::LoongFrameBuffer(uint32_t width, uint32_t height, uint32_t colorAttachmentsCount)
{
    glGenFramebuffers(1, &id_);
    glGenRenderbuffers(1, &depthStencilBuffer_);

    colorAttachments_.resize(colorAttachmentsCount);
    for (auto& attachment : colorAttachments_) {
        attachment = LoongTextureLoader::CreateFromMemory(nullptr, width, height, false, nullptr);
        attachment->SetFilterMode(LoongTexture::FilterMode::kLinear, LoongTexture::FilterMode::kLinear);
    }

    Bind();
    for (uint32_t i = 0; i < colorAttachmentsCount; ++i) {
        glFramebufferTexture(GL_FRAMEBUFFER, GLenum(GL_COLOR_ATTACHMENT0 + i), colorAttachments_[i]->GetId(), 0);
    }
    Unbind();

    width_ = width;
    height_ = height;
    Resize(width, height);
}

LoongFrameBuffer::LoongFrameBuffer(LoongFrameBuffer&& b) noexcept
{
    std::swap(b.id_, id_);
    std::swap(b.colorAttachments_, colorAttachments_);
    std::swap(b.depthStencilBuffer_, depthStencilBuffer_);
}

LoongFrameBuffer::~LoongFrameBuffer()
{
    if (id_ != 0) {
        glDeleteBuffers(1, &id_);
        id_ = 0;
    }
    if (depthStencilBuffer_ != 0) {
        glGenRenderbuffers(1, &depthStencilBuffer_);
        depthStencilBuffer_ = 0;
    }
}

LoongFrameBuffer& LoongFrameBuffer::operator=(LoongFrameBuffer&& b) noexcept
{
    std::swap(b.id_, id_);
    std::swap(b.colorAttachments_, colorAttachments_);
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

    for (auto& attachment : colorAttachments_) {
        attachment->Resize(width, height);
    }

    glBindRenderbuffer(GL_RENDERBUFFER, depthStencilBuffer_);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_STENCIL, width, height);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    Bind();
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthStencilBuffer_);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depthStencilBuffer_);
    Unbind();
}

}
