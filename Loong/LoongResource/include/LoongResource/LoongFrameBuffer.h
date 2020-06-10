//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#pragma once

#include <glad/glad.h>

#include <cstdint>

namespace Loong::Resource {

class LoongFrameBuffer {
public:
    explicit LoongFrameBuffer(uint32_t width = 0, uint32_t height = 0);
    LoongFrameBuffer(const LoongFrameBuffer&) = delete;
    LoongFrameBuffer(LoongFrameBuffer&& b) noexcept;
    ~LoongFrameBuffer();

    LoongFrameBuffer& operator=(const LoongFrameBuffer&) = delete;
    LoongFrameBuffer& operator=(LoongFrameBuffer&& b) noexcept;

    void Bind() const;

    void Unbind() const;

    void Resize(uint32_t width, uint32_t height);

    GLuint GetID() { return id_; }

    GLuint GetTextureID() { return renderTexture_; }

    GLuint GetRenderBufferID() { return depthStencilBuffer_; }

private:
    uint32_t id_ { 0 };
    uint32_t renderTexture_ { 0 };
    uint32_t depthStencilBuffer_ { 0 };
    uint32_t width_ { 0 };
    uint32_t height_ { 0 };
};

}