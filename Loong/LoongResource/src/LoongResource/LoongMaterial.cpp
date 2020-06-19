//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#include "LoongResource/LoongMaterial.h"
#include "LoongResource/LoongResourceManager.h"
#include "LoongResource/LoongShader.h"

namespace Loong::Resource {

void LoongMaterial::SetShader(std::shared_ptr<LoongShader> shader)
{
    shader_ = std::move(shader);
    uniformsData_.clear();
    if (shader_) {
        if (auto index = shader_->GetUniformBlockLocation("BasicUBO"); index != -1) {
            shader_->BindUniformBlock(index, 0);
        }
        if (auto index = shader_->GetUniformBlockLocation("LightUBO"); index != -1) {
            shader_->BindUniformBlock(index, 1);
        }

        FillUniform();
    }
}

void LoongMaterial::SetShaderByFile(const std::string& shaderFile)
{
    auto shader = LoongResourceManager::GetShader(shaderFile);
    if (shader == nullptr) {
        LOONG_ERROR("Load shader '{}' failed for material '{}'", shaderFile, path_);
    } else {
        SetShader(shader);
    }
}

void LoongMaterial::Bind(LoongTexture* emptyTexture) const
{
    if (!HasShader())
        return;

    shader_->Bind();
    int textureSlot = 0;

    auto uniformInfos = shader_->GetUniformInfo();
    for (const auto& info : uniformInfos) {
        auto it = uniformsData_.find(info.name);
        if (it == uniformsData_.end()) {
            continue;
        }
        auto& name = it->first;
        auto& value = it->second;

        switch (info.type) {
            // clang-format off
        case LoongShader::UniformType::kUniformBool:      if (value.type() == typeid(bool)) shader_->SetUniformInt(name, std::any_cast<bool>(value));                     else { LOONG_WARNING("Wrong value type for shader variable {}, expect Bool!", name); } break;
        case LoongShader::UniformType::kUniformInt:       if (value.type() == typeid(GLint)) shader_->SetUniformInt(name, std::any_cast<GLint>(value));                   else { LOONG_WARNING("Wrong value type for shader variable {}, expect Int!", name); } break;
        case LoongShader::UniformType::kUniformFloat:     if (value.type() == typeid(float)) shader_->SetUniformFloat(name, std::any_cast<float>(value));                 else { LOONG_WARNING("Wrong value type for shader variable {}, expect Float!", name); } break;
        case LoongShader::UniformType::kUniformFloatVec2: if (value.type() == typeid(Math::Vector2)) shader_->SetUniformVec2(name, std::any_cast<Math::Vector2>(value));  else { LOONG_WARNING("Wrong value type for shader variable {}, expect FloatVec2!", name); } break;
        case LoongShader::UniformType::kUniformFloatVec3: if (value.type() == typeid(Math::Vector3)) shader_->SetUniformVec3(name, std::any_cast<Math::Vector3>(value));  else { LOONG_WARNING("Wrong value type for shader variable {}, expect FloatVec3!", name); } break;
        case LoongShader::UniformType::kUniformFloatVec4: if (value.type() == typeid(Math::Vector4)) shader_->SetUniformVec4(name, std::any_cast<Math::Vector4>(value));  else { LOONG_WARNING("Wrong value type for shader variable {}, expect FloatVec4!", name); } break;
        case LoongShader::UniformType::kUniformFloatMat4: if (value.type() == typeid(Math::Matrix4)) shader_->SetUniformMat4(name, std::any_cast<Math::Matrix4 >(value)); else { LOONG_WARNING("Wrong value type for shader variable {}, expect FloatMat4!", name); } break;
            // clang-format on
        case LoongShader::UniformType::kUniformSampler2D: {
            if (value.type() == typeid(std::shared_ptr<LoongTexture>)) {
                if (auto tex = std::any_cast<std::shared_ptr<LoongTexture>>(value); tex) {
                    tex->Bind(textureSlot);
                    shader_->SetUniformInt(name, textureSlot++);
                } else if (emptyTexture) {
                    emptyTexture->Bind(textureSlot);
                    shader_->SetUniformInt(name, textureSlot++);
                }
            }
        } break;
        case LoongShader::UniformType::kUniformSamplerCube:
        default:
            assert(false); // impossible, or thers is a bug in Shader::GetUniformInfo();
        }
    }
}

void LoongMaterial::UnBind() const
{
    if (HasShader()) {
        shader_->Unbind();
    }
}

LoongPipelineFixedState LoongMaterial::GenerateStateMask() const
{
    LoongPipelineFixedState result;

    // clang-format off
    if (depthWriting_)                         { result.SetDepthWriteEnabled(true); }
    if (colorWriting_)                         { result.SetColorWriteEnabled(true); }
    if (blendable_)                            { result.SetBlendEnabled(true); }
    if (backFaceCulling_ || frontFaceCulling_) { result.SetFaceCullEnabled(true); }
    if (depthTest_)                            { result.SetDepthTestEnabled(true); }
    // clang-format on
    if (backFaceCulling_) {
        if (frontFaceCulling_) {
            result.SetFrontAndBackCullEnabled(true);
        } else {
            result.SetBackCullEnabled(true);
        }
    } else if (frontFaceCulling_) {
        result.SetFrontCullEnabled(true);
    }

    return result;
}

void LoongMaterial::FillUniform()
{
    for (const auto& uniformInfo : shader_->GetUniformInfo()) {
        uniformsData_.insert({ uniformInfo.name, uniformInfo.defaultValue });
    }
}

}
