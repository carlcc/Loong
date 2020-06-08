//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#pragma once

#include <glad/glad.h>
#include <utility>

#include "LoongResource/LoongGpuBuffer.h"

namespace Loong::Resource {

class LoongVertexArray {
public:
    LoongVertexArray()
    {
        glGenVertexArrays(1, &id_);
    }

    LoongVertexArray(const LoongVertexArray&) = delete;

    LoongVertexArray(LoongVertexArray&& b) noexcept
        : id_(b.id_)
    {
        b.id_ = 0;
    }

    ~LoongVertexArray()
    {
        if (id_ != 0) {
            glDeleteVertexArrays(1, &id_);
            id_ = 0;
        }
    }

    LoongVertexArray& operator=(const LoongVertexArray&) = delete;

    LoongVertexArray& operator=(LoongVertexArray&& b) noexcept
    {
        std::swap(id_, b.id_);
        return *this;
    }

    template <class T>
    void BindAttribute(GLuint attrib, const LoongVertexBuffer& vertexBuffer, GLint count, GLsizei stride, intptr_t offset)
    {
        Bind();
        vertexBuffer.Bind();
        glEnableVertexAttribArray(attrib);

        if constexpr (std::is_same_v<T, GLbyte>) {
            glVertexAttribPointer(attrib, count, GL_BYTE, GL_FALSE, stride, reinterpret_cast<const GLvoid*>(offset));
        } else if constexpr (std::is_same_v<T, GLubyte>) {
            glVertexAttribPointer(attrib, count, GL_UNSIGNED_BYTE, GL_FALSE, stride, reinterpret_cast<const GLvoid*>(offset));
        } else if constexpr (std::is_same_v<T, GLshort>) {
            glVertexAttribPointer(attrib, count, GL_SHORT, GL_FALSE, stride, reinterpret_cast<const GLvoid*>(offset));
        } else if constexpr (std::is_same_v<T, GLushort>) {
            glVertexAttribPointer(attrib, count, GL_UNSIGNED_SHORT, GL_FALSE, stride, reinterpret_cast<const GLvoid*>(offset));
        } else if constexpr (std::is_same_v<T, GLint>) {
            glVertexAttribPointer(attrib, count, GL_INT, GL_FALSE, stride, reinterpret_cast<const GLvoid*>(offset));
        } else if constexpr (std::is_same_v<T, GLuint>) {
            glVertexAttribPointer(attrib, count, GL_UNSIGNED_INT, GL_FALSE, stride, reinterpret_cast<const GLvoid*>(offset));
        } else if constexpr (std::is_same_v<T, GLfloat>) {
            glVertexAttribPointer(attrib, count, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<const GLvoid*>(offset));
        } else if constexpr (std::is_same_v<T, GLdouble>) {
            glVertexAttribPointer(attrib, count, GL_DOUBLE, GL_FALSE, stride, reinterpret_cast<const GLvoid*>(offset));
        } else {
            static_assert(std::is_same_v<T, GLubyte>, "Not supported"); // == static_assert(false), which causes compile error on some compiler
        }
    }

    void Bind() const
    {
        glBindVertexArray(id_);
    }

    void Unbind() const
    {
        glBindVertexArray(0);
    }

    GLint GetId() const
    {
        return id_;
    }

private:
    GLuint id_ { 0 };
};

}