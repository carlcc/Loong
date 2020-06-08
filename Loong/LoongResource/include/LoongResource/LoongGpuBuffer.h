//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#pragma once

#include <glad/glad.h>

#include <utility>

namespace Loong::Resource {

enum class LoongGpuBufferUsage {
    kStaticDraw = GL_STATIC_DRAW,
    kStaticCopy = GL_STATIC_COPY,
    kStaticRead = GL_STATIC_READ,
    kDynamicDraw = GL_DYNAMIC_DRAW,
    kDynamicCopy = GL_DYNAMIC_COPY,
    kDynamicRead = GL_DYNAMIC_READ,
    kStreamDraw = GL_STREAM_DRAW,
    kStreamCopy = GL_STREAM_COPY,
    kStreamRead = GL_STREAM_READ,
};

enum class LoongGpuBufferType {
    kVertexBuffer,
    kIndexBuffer,
    kUniformBuffer,
    // NOTE: Shader storage buffer requires OpenGL 4.3 which is not supported on macos by now(2020),
    // so... We don't use it now
    // kShaderStorageBuffer,
};

// clang-format off
template <LoongGpuBufferType BufferType> struct LoongGpuBufferTypeTrait;
template <> struct LoongGpuBufferTypeTrait<LoongGpuBufferType::kVertexBuffer> {
    static constexpr GLenum kTargetType = GL_ARRAY_BUFFER;
};
template <> struct LoongGpuBufferTypeTrait<LoongGpuBufferType::kIndexBuffer> {
    static constexpr GLenum kTargetType = GL_ELEMENT_ARRAY_BUFFER;
};
template <> struct LoongGpuBufferTypeTrait<LoongGpuBufferType::kUniformBuffer> {
    static constexpr GLenum kTargetType = GL_UNIFORM_BUFFER;
};
// clang-format on

template <LoongGpuBufferType BufferType>
class LoongGpuBuffer {
public:
    LoongGpuBuffer()
    {
        glGenBuffers(1, &id_);
    }

    LoongGpuBuffer(const LoongGpuBuffer&) = delete;
    LoongGpuBuffer(LoongGpuBuffer&& b) noexcept
        : id_(b.id_)
    {
        b.id_ = 0;
    }

    ~LoongGpuBuffer()
    {
        if (id_ != 0) {
            glDeleteBuffers(1, &id_);
            id_ = 0;
        }
    }

    LoongGpuBuffer& operator=(const LoongGpuBuffer&) = delete;

    LoongGpuBuffer& operator=(LoongGpuBuffer&& b) noexcept
    {
        std::swap(id_, b.id_);
    }

    bool operator!() const
    {
        return id_ == 0;
    }

    explicit operator bool() const
    {
        return id_ != 0;
    }

    template <class T>
    void BufferData(const T* data, size_t size, LoongGpuBufferUsage usage = LoongGpuBufferUsage::kStaticDraw)
    {
        Bind();
        glBufferData(LoongGpuBufferTypeTrait<BufferType>::kTargetType, sizeof(T) * size, data, GLenum(usage));
        Unbind();
    }

    template <typename T>
    void SetSubData(const T* data, size_t offset)
    {
        Bind();
        glBufferSubData(LoongGpuBufferTypeTrait<BufferType>::kTargetType, offset, sizeof(T), data);
        Unbind();
    }

    void Bind() const
    {
        glBindBuffer(LoongGpuBufferTypeTrait<BufferType>::kTargetType, id_);
    }

    void Unbind() const
    {
        glBindBuffer(LoongGpuBufferTypeTrait<BufferType>::kTargetType, 0);
    }

    GLuint GetID() const
    {
        return id_;
    }

private:
    GLuint id_ { 0 };
};

using LoongVertexBuffer = LoongGpuBuffer<LoongGpuBufferType::kVertexBuffer>;
using LoongIndexBuffer = LoongGpuBuffer<LoongGpuBufferType::kIndexBuffer>;

class LoongUniformBuffer : public LoongGpuBuffer<LoongGpuBufferType::kUniformBuffer> {
public:
    void SetBindingPoint(uint32_t bindPoint, size_t size)
    {
        Bind();
        glBindBufferRange(GL_UNIFORM_BUFFER, bindPoint, GetID(), 0, size);
    }
};

}