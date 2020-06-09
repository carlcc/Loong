//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#pragma once

#include "LoongFoundation/LoongLogger.h"
#include "LoongResource/LoongPipelineFixedState.h"
#include "LoongResource/LoongTexture.h"
#include <any>
#include <map>
#include <string>

namespace Loong::Resource {

class LoongShader;
class LoongTexture;

class LoongMaterial {
public:
    void SetShader(std::shared_ptr<LoongShader> shader);

    void SetShaderByFile(const std::string& shaderFile);

    std::shared_ptr<LoongShader> GetShader() const { return shader_; }

    void Bind(LoongTexture* emptyTexture) const;

    void UnBind() const;

    template <typename T>
    void Set(const std::string& key, const T& value)
    {
        if (HasShader()) {
            if (uniformsData_.find(key) != uniformsData_.end())
                uniformsData_[key] = std::any(value);
        } else {
            LOONG_ERROR("Material Set failed: No attached shader");
        }
    }

    template <typename T>
    const T& Get(const std::string& key)
    {
        if (auto it = uniformsData_.find(key); it == uniformsData_.end() || it->second.type() != typeid(T)) {
            return T();
        } else {
            return std::any_cast<T>(it->second);
        }
    }

    std::shared_ptr<LoongShader> GetShader() { return shader_; }

    bool HasShader() const { return shader_ != nullptr; }

    void SetBlendable(bool b) { blendable_ = b; }

    void SetBackFaceCulling(bool b) { backFaceCulling_ = b; }

    void SetFrontFaceCulling(bool b) { frontFaceCulling_ = b; }

    void SetDepthTest(bool b) { depthTest_ = b; }

    void SetDepthWriting(bool b) { depthWriting_ = b; }

    void SetColorWriting(bool b) { colorWriting_ = b; }

    void SetGPUInstances(int instances) { gpuInstances_ = instances; }

    bool IsBlendable() const { return blendable_; }

    bool HasBackFaceCulling() const { return backFaceCulling_; }

    bool HasFrontFaceCulling() const { return frontFaceCulling_; }

    bool HasDepthTest() const { return depthTest_; }

    bool HasDepthWriting() const { return depthWriting_; }

    bool HasColorWriting() const { return colorWriting_; }

    LoongPipelineFixedState GenerateStateMask() const;

    std::map<std::string, std::any>& GetUniformsData() { return uniformsData_; }

    const std::map<std::string, std::any>& GetUniformsData() const { return uniformsData_; }

    const std::string GetPath() const { return path_; }

    void SetPath(const std::string& path) { path_ = path; }

private:
    void FillUniform();

private:
    std::shared_ptr<LoongShader> shader_ { nullptr };

    std::map<std::string, std::any> uniformsData_ {};

    bool blendable_ { false };
    bool backFaceCulling_ { true };
    bool frontFaceCulling_ { false };
    bool depthTest_ { true };
    bool depthWriting_ { true };
    bool colorWriting_ { true };
    int gpuInstances_ { 1 };
    std::string path_ {};
};

}