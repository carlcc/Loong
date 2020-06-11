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
    explicit LoongFrameBuffer(uint32_t width = 0, uint32_t height = 0, uint32_t colorAttachmentsCount = 1);
    LoongFrameBuffer(const LoongFrameBuffer&) = delete;
    LoongFrameBuffer(LoongFrameBuffer&& b) noexcept;
    ~LoongFrameBuffer();

    LoongFrameBuffer& operator=(const LoongFrameBuffer&) = delete;
    LoongFrameBuffer& operator=(LoongFrameBuffer&& b) noexcept;

    void Bind() const;

    void Unbind() const;

    void Resize(uint32_t width, uint32_t height);

    GLuint GetID() { return id_; }

    const std::vector<TextureRef>& GetColorAttachments() const { return colorAttachments_; }

    GLuint GetRenderBufferID() { return depthStencilBuffer_; }

private:
    uint32_t id_ { 0 };
    std::vector<TextureRef> colorAttachments_ {};
    uint32_t depthStencilBuffer_ { 0 };
    uint32_t width_ { 0 };
    uint32_t height_ { 0 };
};

}