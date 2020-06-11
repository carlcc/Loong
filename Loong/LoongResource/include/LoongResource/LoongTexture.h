//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//
#pragma once

#include <glad/glad.h>

#include <string>
#include <utility>

namespace Loong::Resource {

class LoongTexture {
public:
    enum class FilterMode {
        kNeareast = GL_NEAREST,
        kLinear = GL_LINEAR,
        kNeareastMipmapNearest = GL_NEAREST_MIPMAP_NEAREST,
        kNeareastMipmapLinear = GL_NEAREST_MIPMAP_LINEAR,
        kLinearMipmapNearest = GL_LINEAR_MIPMAP_NEAREST,
        kLinearMipmapLinear = GL_LINEAR_MIPMAP_LINEAR,
    };
    enum class WrapMode {
        kRepeat = GL_REPEAT,
        kClampToBoarder = GL_CLAMP_TO_BORDER,
        kClampToEdge = GL_CLAMP_TO_EDGE,
        kMirroredRepeat = GL_MIRRORED_REPEAT,
        kMirroredClampToEdge = GL_MIRROR_CLAMP_TO_EDGE,
    };
    ~LoongTexture();

    void Bind(uint32_t slot = 0) const;

    void Unbind() const;

    void Resize(uint32_t width, uint32_t height);

    uint32_t GetId() const { return id_; }

    uint32_t GetWidth() const { return width_; }

    uint32_t GetHeight() const { return height_; }

    uint32_t GetBytesPerPixel() const { return bytesPerPixel_; }

    bool IsMipmapped() const { return isMipmapped_; }

    const std::string& GetPath() const { return path_; }

    void SetFilterMode(FilterMode min, FilterMode mag);

    void SetWrapMode(WrapMode s, WrapMode t);

    void SetFilterWrapMode(FilterMode min, FilterMode mag, WrapMode s, WrapMode t);

private:
    void SetPath(const std::string& path) { path_ = path; }

    friend class LoongTextureLoader;
    LoongTexture() = default;
    LoongTexture(GLuint id, uint32_t width, uint32_t height, uint32_t bytesPerPixel, bool generateMipmap);
    LoongTexture(const LoongTexture&) = delete;
    LoongTexture(LoongTexture&& t) noexcept
    {
        *this = std::move(t);
    }

    LoongTexture& operator=(const LoongTexture&) = delete;
    LoongTexture& operator=(LoongTexture&& t)
    {
        std::swap(id_, t.id_);
        std::swap(width_, t.width_);
        std::swap(height_, t.height_);
        std::swap(bytesPerPixel_, t.bytesPerPixel_);
        std::swap(isMipmapped_, t.isMipmapped_);
        return *this;
    }

public:
    GLuint id_ { 0 };
    uint32_t width_ { 0 };
    uint32_t height_ { 0 };
    uint32_t bytesPerPixel_ { 0 };
    bool isMipmapped_ { false };

private:
    std::string path_ {};
};

} // namespace Loong
