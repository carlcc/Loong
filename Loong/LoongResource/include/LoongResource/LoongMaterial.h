//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#pragma once

#include "LoongFoundation/LoongLogger.h"
#include "LoongFoundation/LoongMacros.h"
#include "LoongFoundation/LoongMath.h"
#include "LoongRHI/LoongRHIManager.h"
#include "LoongResource/LoongPipelineFixedState.h"
#include "LoongResource/LoongRuntimeShader.h"
#include <any>
#include <map>
#include <string>
#include <utility>

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

    LG_NODISCARD bool IsBlendable() const { return blendable_; }

    LG_NODISCARD bool HasBackFaceCulling() const { return backFaceCulling_; }

    LG_NODISCARD bool HasFrontFaceCulling() const { return frontFaceCulling_; }

    LG_NODISCARD bool HasDepthTest() const { return depthTest_; }

    LG_NODISCARD TextureRef GetAlbedoMap() const { return uniforms_.albedoMap; }
    void SetAlbedoMap(TextureRef value) { uniforms_.albedoMap = std::move(value); }

    LG_NODISCARD TextureRef GetNormalMap() const { return uniforms_.normalMap; }
    void SetNormalMap(TextureRef value) { uniforms_.normalMap = std::move(value); }

    LG_NODISCARD TextureRef GetMetallicMap() const { return uniforms_.metallicMap; }
    void SetMetallicMap(TextureRef value) { uniforms_.metallicMap = std::move(value); }

    LG_NODISCARD TextureRef GetRoughnessMap() const { return uniforms_.roughnessMap; }
    void SetRoughnessMap(TextureRef value) { uniforms_.roughnessMap = std::move(value); }

    LG_NODISCARD TextureRef GetEmissiveMap() const { return uniforms_.emissiveMap; }
    void SetEmissiveMap(TextureRef value) { uniforms_.emissiveMap = std::move(value); }

    LG_NODISCARD TextureRef GetAmbientOcclusionMap() const { return uniforms_.ambientOcclusionMap; }
    void SetAmbientOcclusionMap(TextureRef value) { uniforms_.ambientOcclusionMap = std::move(value); }

    LG_NODISCARD const Math::Vector3& GetAlbedo() const { return uniforms_.albedo; }
    void SetAlbedo(const Math::Vector3& value) { uniforms_.albedo = value; }

    LG_NODISCARD float GetMetallic() const { return uniforms_.metallic; }
    void SetMetallic(float value) { uniforms_.metallic = value; }

    LG_NODISCARD float GetRoughness() const { return uniforms_.roughness; }
    void SetRoughness(float value) { uniforms_.roughness = value; }

    LG_NODISCARD float GetEmissiveFactor() const { return uniforms_.emissiveFactor; }
    void SetEmissiveFactor(float value) { uniforms_.emissiveFactor = value; }

    LG_NODISCARD float GetClearCoat() const { return uniforms_.clearCoat; }
    void SetClearCoat(float value) { uniforms_.clearCoat = value; }

    LG_NODISCARD float GetClearCoatRoughness() const { return uniforms_.clearCoatRoughness; }
    void SetClearCoatRoughness(float value) { uniforms_.clearCoatRoughness = value; }

    LG_NODISCARD const Math::Vector2& GetTextureTiling() const { return uniforms_.textureTiling; }
    void SetTextureTiling(const Math::Vector2& value) { uniforms_.textureTiling = value; }

    LG_NODISCARD const Math::Vector2& GetTextureOffset() const { return uniforms_.textureOffset; }
    void SetTextureOffset(const Math::Vector2& value) { uniforms_.textureOffset = value; }

    LG_NODISCARD const std::string& GetPath() const { return path_; }

public:
    LoongMaterial() = default;
    ~LoongMaterial() = default;

private:
    // Pipeline states
    bool blendable_ { false };
    bool backFaceCulling_ { true };
    bool frontFaceCulling_ { false };
    bool depthTest_ { true };

    // Uniform data
    UniformData uniforms_ {};
    std::string path_ {};

    friend class LoongMaterialLoader;
};

}