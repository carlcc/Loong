//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#pragma once

#include <glad/glad.h>

#include <cstdint>

namespace Loong::Resource {

class LoongFramebuffer {
public:
    explicit LoongFramebuffer(uint32_t width = 0, uint32_t height = 0);
    LoongFramebuffer(const LoongFramebuffer&) = delete;
    LoongFramebuffer(LoongFramebuffer&& b) noexcept;
    ~LoongFramebuffer();

    LoongFramebuffer& operator=(const LoongFramebuffer&) = delete;
    LoongFramebuffer& operator=(LoongFramebuffer&& b) noexcept;

    void Bind();

    void Unbind();

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