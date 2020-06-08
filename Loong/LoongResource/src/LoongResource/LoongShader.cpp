//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//
#include "LoongResource/LoongShader.h"
#include "LoongFoundation/LoongLogger.h"
#include "LoongResource/LoongTexture.h"
#include <string>
#include <utility>

namespace Loong::Resource {

LoongShader::LoongShader(GLuint id)
    : id_(id)
{
    QueryUniforms();
}

LoongShader::LoongShader(LoongShader&& s) noexcept
    : id_(s.id_)
    , locationCache_(std::move(s.locationCache_))
{
    s.id_ = 0;
}

LoongShader& LoongShader::operator=(LoongShader&& s) noexcept
{
    std::swap(id_, s.id_);
    std::swap(locationCache_, s.locationCache_);
    return *this;
}

LoongShader::~LoongShader()
{
    glDeleteProgram(id_);
}

uint32_t LoongShader::GetUniformBlockLocation(const std::string& name) const
{
    if (locationCache_.find(name) != locationCache_.end()) {
        return locationCache_.at(name);
    }

    const int location = glGetUniformBlockIndex(id_, name.c_str());

    if (location == -1) {
        LOONG_ERROR("UniformBlock: '{}' doesn't exist", name);
    }

    locationCache_[name] = location;

    return location;
}

void LoongShader::BindUniformBlock(uint32_t uniformBlockIndex, uint32_t bindingPoint)
{
    glUniformBlockBinding(id_, uniformBlockIndex, bindingPoint);
}

uint32_t LoongShader::GetUniformLocation(const std::string& name) const
{
    if (locationCache_.find(name) != locationCache_.end()) {
        return locationCache_.at(name);
    }

    const int location = glGetUniformLocation(id_, name.c_str());

    if (location == -1) {
        LOONG_ERROR("Uniform: '{}' doesn't exist", name);
    }

    locationCache_[name] = location;

    return location;
}

inline bool IsEngineUBOMember(const std::string& uniformName)
{
    return uniformName.compare(0, 4, "ub_") == 0;
}

void LoongShader::QueryUniforms()
{
    GLint numActiveUniforms = 0;
    uniforms_.clear();
    glGetProgramiv(id_, GL_ACTIVE_UNIFORMS, &numActiveUniforms);
    std::vector<GLchar> nameData(256);
    for (int unif = 0; unif < numActiveUniforms; ++unif) {
        GLint arraySize = 0;
        GLenum type = 0;
        GLsizei actualLength = 0;
        glGetActiveUniform(id_, unif, static_cast<GLsizei>(nameData.size()), &actualLength, &arraySize, &type, &nameData[0]);
        std::string name(static_cast<char*>(nameData.data()), actualLength);

        if (!IsEngineUBOMember(name)) {
            std::any defaultValue;

            switch (UniformType(type)) {
            case UniformType::kUniformBool:
                defaultValue = std::make_any<bool>(GetUniformInt(name));
                break;
            case UniformType::kUniformInt:
                defaultValue = std::make_any<int>(GetUniformInt(name));
                break;
            case UniformType::kUniformFloat:
                defaultValue = std::make_any<float>(GetUniformFloat(name));
                break;
            case UniformType::kUniformFloatVec2:
                defaultValue = std::make_any<Math::Vector2>(GetUniformVec2(name));
                break;
            case UniformType::kUniformFloatVec3:
                defaultValue = std::make_any<Math::Vector3>(GetUniformVec3(name));
                break;
            case UniformType::kUniformFloatVec4:
                defaultValue = std::make_any<Math::Vector4>(GetUniformVec4(name));
                break;
            case UniformType::kUniformFloatMat4:
                defaultValue = std::make_any<Math::Matrix4>(GetUniformMat4(name));
                break;
            case UniformType::kUniformSampler2D:
                defaultValue = std::make_any<std::shared_ptr<LoongTexture>>(nullptr);
                break;
            case UniformType::kUniformSamplerCube:
            default:
                LOONG_ERROR("Uniform of type {} is not supported", type);
                continue;
            }

            if (defaultValue.has_value()) {
                uniforms_.push_back({
                    static_cast<UniformType>(type),
                    name,
                    GetUniformLocation(nameData.data()),
                    defaultValue,
                });
            }
        }
    }
}

}