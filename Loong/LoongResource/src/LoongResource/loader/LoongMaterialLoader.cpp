//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#include "LoongResource/loader/LoongMaterialLoader.h"
#include "LoongFileSystem/LoongFileSystem.h"
#include "LoongFoundation/LoongAssert.h"
#include "LoongFoundation/LoongLogger.h"
#include "LoongFoundation/LoongMath.h"
#include "LoongFoundation/LoongThreadPool.h"
#include "LoongResource/LoongMaterial.h"
#include "LoongResource/LoongResourceManager.h"
#include "LoongResource/LoongTexture.h"
#include <atomic>
#include <jsonmapper/JsonMapper.h>
#include <jsonmapper/types/deserialize/string.h>
#include <jsonmapper/types/serialize/string.h>
#include <sstream>
#include <string>
#include <string_view>

namespace Loong::Resource {

using TextureRef = std::shared_ptr<LoongTexture>;

inline Math::Vector4 ParseStringToVector4(const std::string& string)
{
    std::stringstream ss;
    ss << string;
    Math::Vector4 vec { 0.F };
    ss >> vec.x;
    ss >> vec.y;
    ss >> vec.z;
    ss >> vec.w;
    return vec;
}
inline std::string Vec4ToString(const Math::Vector4& vec)
{
    std::stringstream ss;
    ss << vec.x << " " << vec.y << " " << vec.z << " " << vec.w;
    return ss.str();
}
inline Math::Vector3 ParseStringToVector3(const std::string& string)
{
    std::stringstream ss;
    ss << string;
    Math::Vector3 vec { 0.F };
    ss >> vec.x;
    ss >> vec.y;
    ss >> vec.z;
    return vec;
}
inline std::string Vec3ToString(const Math::Vector3& vec)
{
    std::stringstream ss;
    ss << vec.x << " " << vec.y << " " << vec.z;
    return ss.str();
}
inline Math::Vector2 ParseStringToVector2(const std::string& string)
{
    std::stringstream ss;
    ss << string;
    Math::Vector2 vec { 0.F };
    ss >> vec.x;
    ss >> vec.y;
    return vec;
}
inline std::string Vec2ToString(const Math::Vector2& vec)
{
    std::stringstream ss;
    ss << vec.x << " " << vec.y;
    return ss.str();
}
inline int ParseStringToInt(const std::string& string)
{
    std::stringstream ss;
    ss << string;
    int v { 0 };
    ss >> v;
    return v;
}
inline float ParseStringToFloat(const std::string& string)
{
    std::stringstream ss;
    ss << string;
    float v { 0.0F };
    ss >> v;
    return v;
}

struct MaterialData {
    struct Uniforms {
        std::string albedoMap {};
        std::string normalMap {};
        std::string metallicMap {};
        std::string roughnessMap {};
        std::string emissiveMap {};
        std::string ambientOcclusionMap {};
        std::string albedo {}; // Color 3f
        std::string textureTiling {}; // vector 2f
        std::string textureOffset {}; // vector 2f
        float metallic { 0.0F };
        float roughness { 1.0F };
        float emissiveFactor { 0.0F };
        float clearCoat { 0.0F };
        float clearCoatRoughness { 0.0F };

        template <class Archiver>
        bool SerializeToJson(Archiver& ar) const
        {
            return ar(
                JMKVP(albedoMap),
                JMKVP(normalMap),
                JMKVP(metallicMap),
                JMKVP(roughnessMap),
                JMKVP(emissiveMap),
                JMKVP(ambientOcclusionMap),
                JMKVP(albedo),
                JMKVP(textureTiling),
                JMKVP(textureOffset),
                JMKVP(metallic),
                JMKVP(roughness),
                JMKVP(emissiveFactor),
                JMKVP(clearCoat),
                JMKVP(clearCoatRoughness));
        }
        template <class Archiver>
        bool DeserializeFromJson(Archiver& ar)
        {
            return ar(
                JMOPTKVP(albedoMap),
                JMOPTKVP(normalMap),
                JMOPTKVP(metallicMap),
                JMOPTKVP(roughnessMap),
                JMOPTKVP(emissiveMap),
                JMOPTKVP(ambientOcclusionMap),
                JMOPTKVP(albedo),
                JMOPTKVP(textureTiling),
                JMOPTKVP(textureOffset),
                JMOPTKVP(metallic),
                JMOPTKVP(roughness),
                JMOPTKVP(emissiveFactor),
                JMOPTKVP(clearCoat),
                JMOPTKVP(clearCoatRoughness));
        }
    };

    struct PipelineState {
        bool blendable { false };
        bool backFaceCulling { true };
        bool frontFaceCulling { false };
        bool depthTest { true };

        template <class Archiver>
        bool SerializeToJson(Archiver& ar) const
        {
            return ar(
                JMKVP(blendable),
                JMKVP(backFaceCulling),
                JMKVP(frontFaceCulling),
                JMKVP(depthTest));
        }

        template <class Archiver>
        bool DeserializeFromJson(Archiver& ar)
        {
            return ar(
                JMOPTKVP(blendable),
                JMOPTKVP(backFaceCulling),
                JMOPTKVP(frontFaceCulling),
                JMOPTKVP(depthTest));
        }
    };

    PipelineState pipelineState;
    Uniforms uniforms;

    template <class Archiver>
    bool SerializeToJson(Archiver& ar) const
    {
        return ar(
            JMKVP(pipelineState),
            JMKVP(uniforms));
    }

    template <class Archiver>
    bool DeserializeFromJson(Archiver& ar)
    {
        return ar(
            JMOPTKVP(pipelineState),
            JMOPTKVP(uniforms));
    }
};

tpl::Task<std::shared_ptr<LoongMaterial>> LoongMaterialLoader::CreateAsync(const std::string& filePath, const std::function<void(const std::string&)>& onDestroy)
{
    return tpl::MakeTaskAndStart(
        [filePath]() -> tpl::Task<std::shared_ptr<LoongMaterial>> {
            int64_t fileSize = FS::LoongFileSystem::GetFileSize(filePath);
            if (fileSize <= 0) {
                LOONG_ERROR("Failed to load material '{}': Wrong file size", filePath);
                return tpl::MakeTaskFromValue<std::shared_ptr<LoongMaterial>>(nullptr, nullptr);
            }
            std::vector<uint8_t> buffer(fileSize);
            auto size = FS::LoongFileSystem::LoadFileContent(filePath, buffer.data(), fileSize);
            (void)size;
            LOONG_ASSERT(size == fileSize, "");

            MaterialData materialData;
            if (!jsonmapper::DeserializeFromJsonString(materialData, std::string_view { (const char*)buffer.data(), buffer.size() })) {
                LOONG_ERROR("Failed to load material '{}': deserialize json failed", filePath);
                return tpl::MakeTaskFromValue<std::shared_ptr<LoongMaterial>>(nullptr, nullptr);
            }

            auto pMaterial = new LoongMaterial;
            std::shared_ptr<LoongMaterial> material(pMaterial, [](LoongMaterial* m) {
                delete m;
            });
            // Pipeline State
            material->blendable_ = materialData.pipelineState.blendable;
            material->backFaceCulling_ = materialData.pipelineState.backFaceCulling;
            material->frontFaceCulling_ = materialData.pipelineState.frontFaceCulling;
            material->depthTest_ = materialData.pipelineState.depthTest;

            material->uniforms_.metallic = materialData.uniforms.metallic;
            material->uniforms_.roughness = materialData.uniforms.roughness;
            material->uniforms_.emissiveFactor = materialData.uniforms.emissiveFactor;
            material->uniforms_.clearCoat = materialData.uniforms.clearCoat;
            material->uniforms_.clearCoatRoughness = materialData.uniforms.clearCoatRoughness;
            material->uniforms_.albedo = ParseStringToVector3(materialData.uniforms.albedo);
            material->uniforms_.textureTiling = ParseStringToVector2(materialData.uniforms.textureTiling);
            material->uniforms_.textureOffset = ParseStringToVector2(materialData.uniforms.textureOffset);

            auto albedoTask = LoongResourceManager::GetTextureAsync(materialData.uniforms.albedoMap);
            auto normalTask = LoongResourceManager::GetTextureAsync(materialData.uniforms.normalMap);
            auto metallicTask = LoongResourceManager::GetTextureAsync(materialData.uniforms.metallicMap);
            auto roughnessTask = LoongResourceManager::GetTextureAsync(materialData.uniforms.roughnessMap);
            auto emissiveTask = LoongResourceManager::GetTextureAsync(materialData.uniforms.emissiveMap);
            auto ambientOcclusionTask = LoongResourceManager::GetTextureAsync(materialData.uniforms.ambientOcclusionMap);

            auto task = tpl::MakeTask(
                [material](const auto& albedoTask, const auto& normalTask, const auto& metallicTask,
                    const auto& roughnessTask, const auto& emissiveTask, const auto& ambientOcclusionTask) {
                    material->SetAlbedoMap(albedoTask.GetFuture().GetValue());
                    material->SetNormalMap(normalTask.GetFuture().GetValue());
                    material->SetMetallicMap(metallicTask.GetFuture().GetValue());
                    material->SetRoughnessMap(roughnessTask.GetFuture().GetValue());
                    material->SetEmissiveMap(emissiveTask.GetFuture().GetValue());
                    material->SetAmbientOcclusionMap(ambientOcclusionTask.GetFuture().GetValue());
                    return material;
                },
                nullptr, albedoTask, normalTask, metallicTask, roughnessTask, emissiveTask, ambientOcclusionTask);
            return task;
        },
        nullptr)
        .Unwrap();
}

inline const std::string& GetTexturePath(LoongTexture* tex)
{
    static const std::string kEmpty;
    if (tex == nullptr) {
        return kEmpty;
    }
    return tex->GetPath();
}

bool LoongMaterialLoader::Write(const std::string& filePath, const LoongMaterial* material)
{
    MaterialData materialData;
    // Pipeline State
    materialData.pipelineState.blendable = material->blendable_;
    materialData.pipelineState.backFaceCulling = material->backFaceCulling_;
    materialData.pipelineState.frontFaceCulling = material->frontFaceCulling_;
    materialData.pipelineState.depthTest = material->depthTest_;

    // Uniforms
    materialData.uniforms.albedoMap = GetTexturePath(material->uniforms_.albedoMap.get());
    materialData.uniforms.normalMap = GetTexturePath(material->uniforms_.normalMap.get());
    materialData.uniforms.metallicMap = GetTexturePath(material->uniforms_.metallicMap.get());
    materialData.uniforms.roughnessMap = GetTexturePath(material->uniforms_.roughnessMap.get());
    materialData.uniforms.emissiveMap = GetTexturePath(material->uniforms_.emissiveMap.get());
    materialData.uniforms.ambientOcclusionMap = GetTexturePath(material->uniforms_.ambientOcclusionMap.get());
    materialData.uniforms.metallic = material->uniforms_.metallic;
    materialData.uniforms.roughness = material->uniforms_.roughness;
    materialData.uniforms.emissiveFactor = material->uniforms_.emissiveFactor;
    materialData.uniforms.clearCoat = material->uniforms_.clearCoat;
    materialData.uniforms.clearCoatRoughness = material->uniforms_.clearCoatRoughness;
    materialData.uniforms.albedo = Vec3ToString(material->uniforms_.albedo);
    materialData.uniforms.textureTiling = Vec2ToString(material->uniforms_.textureTiling);
    materialData.uniforms.textureOffset = Vec2ToString(material->uniforms_.textureOffset);

    std::string dataToWrite;
    if (!jsonmapper::SerializeToJsonString(materialData, dataToWrite, false)) {
        LOONG_ERROR("Failed to write material '{}': Serialize to json failed", filePath);
        return false;
    }

    auto count = FS::LoongFileSystem::StoreFileContent(filePath, dataToWrite.data(), dataToWrite.size());
    if (size_t(count) != dataToWrite.size()) {
        LOONG_ERROR("Failed to write material '{}': {}", filePath, FS::LoongFileSystem::GetLastError());
        return false;
    }

    return true;
}
}
