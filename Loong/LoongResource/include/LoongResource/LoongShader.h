//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//
#pragma once
#include <glad/glad.h>

#include "LoongFoundation/LoongMath.h"
#include <any>
#include <string>
#include <unordered_map>
#include <vector>

namespace Loong::Resource {

class LoongShader {
public:
    enum class UniformType {
        // clang-format off
        kUniformBool            = GL_BOOL,
        kUniformInt             = GL_INT,
        kUniformFloat           = GL_FLOAT,
        kUniformFloatVec2       = GL_FLOAT_VEC2,
        kUniformFloatVec3       = GL_FLOAT_VEC3,
        kUniformFloatVec4       = GL_FLOAT_VEC4,
        kUniformFloatMat4       = GL_FLOAT_MAT4,
        kUniformSampler2D       = GL_SAMPLER_2D,
        kUniformSamplerCube     = GL_SAMPLER_CUBE,
        // clang-format on
    };
    struct UniformInfo {
        UniformType type;
        std::string name;
        uint32_t location;
        std::any defaultValue;
    };

private:
    // Note: This construct will take over the ownship
    explicit LoongShader(GLuint id);
    friend class LoongResourceManager;

public:
    LoongShader(const LoongShader&) = delete;
    LoongShader(LoongShader&& s) noexcept;
    LoongShader& operator=(const LoongShader&) = delete;
    LoongShader& operator=(LoongShader&& s) noexcept;
    ~LoongShader();

    void Bind() const { glUseProgram(id_); }

    void Unbind() const { glUseProgram(0); }

    void SetUniformInt(const std::string& name, int value) { glUniform1i(GetUniformLocation(name), value); }

    void SetUniformFloat(const std::string& name, float value) { glUniform1f(GetUniformLocation(name), value); }

    void SetUniformVec2(const std::string& name, const Math::Vector2& value) { glUniform2f(GetUniformLocation(name), value.x, value.y); }

    void SetUniformVec3(const std::string& name, const Math::Vector3& value) { glUniform3f(GetUniformLocation(name), value.x, value.y, value.z); }

    void SetUniformVec4(const std::string& name, const Math::Vector4& value) { glUniform4f(GetUniformLocation(name), value.x, value.y, value.z, value.w); }

    void SetUniformMat4(const std::string& name, const Math::Matrix4& value) { glUniformMatrix4fv(GetUniformLocation(name), 1, GL_TRUE, &value[0].x); }

    int GetUniformInt(const std::string& name) const
    {
        int value;
        glGetUniformiv(id_, GetUniformLocation(name), &value);
        return value;
    }

    float GetUniformFloat(const std::string& name) const
    {
        float value;
        glGetUniformfv(id_, GetUniformLocation(name), &value);
        return value;
    }

    // glGetnUniformfv needs opengl 4.5+
    Math::Vector2 GetUniformVec2(const std::string& name) const
    {
        Math::Vector2 value;
        glGetUniformfv(id_, GetUniformLocation(name), &value.x);
        return value;
    }

    Math::Vector3 GetUniformVec3(const std::string& name) const
    {
        Math::Vector3 value;
        glGetUniformfv(id_, GetUniformLocation(name), &value.x);
        return value;
    }

    Math::Vector4 GetUniformVec4(const std::string& name) const
    {
        Math::Vector4 value;
        glGetUniformfv(id_, GetUniformLocation(name), &value.x);
        return value;
    }

    Math::Matrix4 GetUniformMat4(const std::string& name) const
    {
        Math::Matrix4 value;
        glGetUniformfv(id_, GetUniformLocation(name), &value[0].x);
        return value;
    }

    uint32_t GetUniformBlockLocation(const std::string& name) const;

    void BindUniformBlock(uint32_t uniformBlockIndex, uint32_t bindingPoint);

    const std::vector<UniformInfo>& GetUniformInfo() const { return uniforms_; }

private:
    void QueryUniforms();
    uint32_t GetUniformLocation(const std::string& name) const;

private:
    GLuint id_ { 0 };
    mutable std::unordered_map<std::string, int> locationCache_ {};
    std::vector<UniformInfo> uniforms_ {};
};

}