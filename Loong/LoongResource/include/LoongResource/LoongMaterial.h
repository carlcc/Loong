//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#pragma once

#include "LoongFoundation/LoongLogger.h"
#include "LoongFoundation/LoongMath.h"
#include "LoongRHI/LoongRHIManager.h"
#include "LoongResource/LoongPipelineFixedState.h"
#include "LoongResource/LoongRuntimeShader.h"
#include <any>
#include <map>
#include <string>

namespace Loong::Resource {

class LoongTexture;

// TODO: Generate material class from shader file
class LoongMaterial {
public:
    using TextureRef = std::shared_ptr<LoongTexture>;
    struct UniformData {
        TextureRef albedoMap;
        TextureRef normalMap;
        TextureRef metallicMap;
        TextureRef roughnessMap;
        TextureRef emissiveMap;
        TextureRef ambientOcclusionMap;
        Math::Vector3 albedo;
        float metallic;
        float roughness;
        float emissiveFactor;
        float clearCoat;
        float clearCoatRoughness;
        Math::Vector2 textureTiling;
        Math::Vector2 textureOffset;
    };

    void SetBlendable(bool b) { blendable_ = b; }

    void SetBackFaceCulling(bool b) { backFaceCulling_ = b; }

    void SetFrontFaceCulling(bool b) { frontFaceCulling_ = b; }

    void SetDepthTest(bool b) { depthTest_ = b; }

    bool IsBlendable() const { return blendable_; }

    bool HasBackFaceCulling() const { return backFaceCulling_; }

    bool HasFrontFaceCulling() const { return frontFaceCulling_; }

    bool HasDepthTest() const { return depthTest_; }

    LoongPipelineFixedState GenerateStateMask() const;

    const std::string& GetPath() const { return path_; }

    void SetPath(const std::string& path) { path_ = path; }

private:
    void FillUniform();

private:
    // Pipeline states
    bool blendable_ { false };
    bool backFaceCulling_ { true };
    bool frontFaceCulling_ { false };
    bool depthTest_ { true };

    // Uniform data
    UniformData uniforms_ {};
    std::string path_ {};
};

}