//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#include "LoongResource/loader/LoongMaterialLoader.h"
#include "LoongFileSystem/LoongFileSystem.h"
#include "LoongFoundation/LoongMath.h"
#include "LoongResource/LoongMaterial.h"
#include "LoongResource/LoongResourceManager.h"
#include "LoongResource/LoongShader.h"
#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/ostreamwrapper.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/rapidjson.h>
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

std::shared_ptr<LoongMaterial> LoongMaterialLoader::Create(const std::string& filePath, const std::function<void(const std::string&)>& onDestroy)
{
    rapidjson::Document root;
    {
        int64_t fileSize = FS::LoongFileSystem::GetFileSize(filePath);
        if (fileSize <= 0) {
            LOONG_ERROR("Failed to load material '{}': Wrong file size", filePath);
            return nullptr;
        }
        std::vector<char> buffer(fileSize);
        assert(FS::LoongFileSystem::LoadFileContent(filePath, buffer.data(), fileSize) == fileSize);

        std::stringstream iss(std::ios::binary | std::ios::in | std::ios::out);
        iss << std::string_view(buffer.data(), buffer.size());

        rapidjson::IStreamWrapper issw(iss);
        root.ParseStream(issw);
        if (root.HasParseError()) {
            LOONG_ERROR("Parse material file '{}' failed", filePath);
            return nullptr;
        }
    }

    std::shared_ptr<LoongMaterial> material;
    if (onDestroy != nullptr) {
        auto* mtl = new LoongMaterial;
        material.reset(mtl, [onDestroy](LoongMaterial* mtl) {
            onDestroy(mtl->GetPath());
            delete mtl;
        });
    } else {
        material = std::make_shared<LoongMaterial>();
    }
    material->SetPath(filePath);

    auto shaderIt = root.FindMember("shader");
    if (shaderIt == root.MemberEnd() || !shaderIt->value.IsString()) {
        LOONG_ERROR("Parse material file '{}' failed: Has no shader", filePath);
    } else {
        material->SetShaderByFile(shaderIt->value.GetString());
    }

    auto rendererStateIt = root.FindMember("state");
    if (rendererStateIt != root.MemberEnd()) {
        auto blendableIt = rendererStateIt->value.FindMember("blend");
        if (blendableIt != rendererStateIt->value.MemberEnd() && blendableIt->value.IsBool()) {
            material->SetBlendable(blendableIt->value.GetBool());
        }
        auto backFaceCullingIt = rendererStateIt->value.FindMember("backCulling");
        if (backFaceCullingIt != rendererStateIt->value.MemberEnd() && backFaceCullingIt->value.IsBool()) {
            material->SetBackFaceCulling(backFaceCullingIt->value.GetBool());
        }
        auto frontFaceCullingIt = rendererStateIt->value.FindMember("frontCulling");
        if (frontFaceCullingIt != rendererStateIt->value.MemberEnd() && frontFaceCullingIt->value.IsBool()) {
            material->SetFrontFaceCulling(frontFaceCullingIt->value.GetBool());
        }
        auto depthTestIt = rendererStateIt->value.FindMember("depthTest");
        if (depthTestIt != rendererStateIt->value.MemberEnd() && depthTestIt->value.IsBool()) {
            material->SetDepthTest(depthTestIt->value.GetBool());
        }
        auto depthWritingIt = rendererStateIt->value.FindMember("depthWriting");
        if (depthWritingIt != rendererStateIt->value.MemberEnd() && depthWritingIt->value.IsBool()) {
            material->SetDepthWriting(depthWritingIt->value.GetBool());
        }
        auto colorWritingIt = rendererStateIt->value.FindMember("colorWriting");
        if (colorWritingIt != rendererStateIt->value.MemberEnd() && colorWritingIt->value.IsBool()) {
            material->SetColorWriting(colorWritingIt->value.GetBool());
        }
        auto gpuInstancesIt = rendererStateIt->value.FindMember("gpuInstances");
        if (gpuInstancesIt != rendererStateIt->value.MemberEnd() && gpuInstancesIt->value.IsInt()) {
            material->SetGPUInstances(gpuInstancesIt->value.GetInt());
        }
    }

    auto paramsIt = root.FindMember("params");
    auto& materialUniforms = material->GetUniformsData();
    if (paramsIt != root.MemberEnd()) {
        auto memberIt = paramsIt->value.MemberBegin();
        auto memberEnd = paramsIt->value.MemberEnd();
        for (; memberIt != memberEnd; ++memberIt) {
            auto paramName = memberIt->name.GetString();
            auto materialUniformIt = materialUniforms.find(paramName);
            if (materialUniformIt == materialUniforms.end()) {
                LOONG_WARNING("Parse material file '{}' warning: Parameter '{}' is not found in shader skip", filePath, paramName);
                continue;
            }
            if (!memberIt->value.IsObject()) {
                LOONG_ERROR("Parse material file '{}' failed: Invalid file format, params should be object");
                return nullptr;
            }

            auto& valueObj = memberIt->value;
            auto typeIt = valueObj.FindMember("type");
            auto valueIt = valueObj.FindMember("value");
            if (typeIt == valueObj.MemberEnd() || valueIt == valueObj.MemberEnd()) {
                LOONG_ERROR("Parse material file '{}' failed: Invalid file format, missing type or value field");
                return nullptr;
            }
            if (!typeIt->value.IsString() || !valueIt->value.IsString()) {
                LOONG_ERROR("Parse material file '{}' failed: Invalid file format, type must be a string");
                return nullptr;
            }

            std::string typeString = typeIt->value.GetString();
            std::string valueString = valueIt->value.GetString();
            if (typeString == "vec4") {
                material->GetUniformsData()[paramName] = ParseStringToVector4(valueString);
            } else if (typeString == "vec3") {
                material->GetUniformsData()[paramName] = ParseStringToVector3(valueString);
            } else if (typeString == "vec2") {
                material->GetUniformsData()[paramName] = ParseStringToVector2(valueString);
            } else if (typeString == "int") {
                material->GetUniformsData()[paramName] = ParseStringToInt(valueString);
            } else if (typeString == "float") {
                material->GetUniformsData()[paramName] = ParseStringToFloat(valueString);
            } else if (typeString == "bool") {
                material->GetUniformsData()[paramName] = bool(ParseStringToInt(valueString));
            } else if (typeString == "tex2d") {
                if (!valueString.empty()) {
                    auto texture = LoongResourceManager::GetTexture(valueString);
                    material->GetUniformsData()[paramName] = texture;
                } else {
                    material->GetUniformsData()[paramName] = TextureRef(nullptr);
                }
            } else {
                abort(); // TODO: Implement
            }
        }
    }

    return material;
}

bool LoongMaterialLoader::Write(const std::string& filePath, const LoongMaterial* material)
{
    assert(material != nullptr);
    std::stringstream oss(std::ios::in | std::ios::out | std::ios::binary);
    if (!oss) {
        return false;
    }

    rapidjson::Document root(rapidjson::kObjectType);
    auto& allocator = root.GetAllocator();
    if (auto shader = material->GetShader(); shader != nullptr) {
        rapidjson::Value tmp(shader->GetPath().c_str(), allocator);
        root.AddMember("shader", tmp, allocator);

        if (auto& uniformInfo = shader->GetUniformInfo(); !uniformInfo.empty()) {
            rapidjson::Value params(rapidjson::kObjectType);

            auto& uniformData = material->GetUniformsData();
            for (auto& info : uniformInfo) {

                auto it = uniformData.find(info.name);
                if (it == uniformData.end()) {
                    continue;
                }
                auto& key = info.name;
                auto& value = it->second;
                tmp.SetString(key.c_str(), allocator);

                std::string typeString;
                std::string valueString;
                switch (info.type) {
                case LoongShader::UniformType::kUniformBool:
                    if (typeid(bool) == value.type()) {
                        typeString = "bool";
                        valueString = std::to_string((int)std::any_cast<bool>(value));
                    }
                    break;
                case LoongShader::UniformType::kUniformInt:
                    if (typeid(int) == value.type()) {
                        typeString = "int";
                        valueString = std::to_string(std::any_cast<int>(value));
                    }
                    break;
                case LoongShader::UniformType::kUniformFloat:
                    if (typeid(float) == value.type()) {
                        typeString = "float";
                        valueString = std::to_string((int)std::any_cast<float>(value));
                    }
                    break;
                case LoongShader::UniformType::kUniformFloatVec2:
                    if (typeid(Math::Vector2) == value.type()) {
                        typeString = "vec2";
                        valueString = Vec2ToString(std::any_cast<Math::Vector2>(value));
                    }
                    break;
                case LoongShader::UniformType::kUniformFloatVec3:
                    if (typeid(Math::Vector3) == value.type()) {
                        typeString = "vec3";
                        valueString = Vec3ToString(std::any_cast<Math::Vector3>(value));
                    }
                    break;
                case LoongShader::UniformType::kUniformFloatVec4:
                    if (typeid(Math::Vector4) == value.type()) {
                        typeString = "vec4";
                        valueString = Vec4ToString(std::any_cast<Math::Vector4>(value));
                    }
                    break;
                case LoongShader::UniformType::kUniformSampler2D:
                    if (typeid(TextureRef) == value.type()) {
                        typeString = "tex2d";
                        auto texture = std::any_cast<TextureRef>(value);
                        valueString = texture == nullptr ? "" : texture->GetPath();
                    }
                    break;
                case LoongShader::UniformType::kUniformFloatMat4:
                case LoongShader::UniformType::kUniformSamplerCube:
                default:
                    LOONG_ERROR("Unsupported uniform type: {}", info.type);
                    continue;
                }

                rapidjson::Value valueObj(rapidjson::kObjectType);
                valueObj.AddMember("type", rapidjson::Value(typeString.c_str(), allocator), allocator);
                valueObj.AddMember("value", rapidjson::Value(valueString.c_str(), allocator), allocator);
                params.AddMember(tmp, valueObj, allocator);
            }

            root.AddMember("params", params, allocator);
        }
    }
    {
        rapidjson::Value state(rapidjson::kObjectType);
        state.AddMember("blend", material->IsBlendable(), allocator);
        state.AddMember("backCulling", material->HasBackFaceCulling(), allocator);
        state.AddMember("frontCulling", material->HasFrontFaceCulling(), allocator);
        state.AddMember("depthTest", material->HasDepthTest(), allocator);
        state.AddMember("depthWriting", material->HasDepthWriting(), allocator);
        state.AddMember("colorWriting", material->HasColorWriting(), allocator);
        root.AddMember("state", std::move(state), allocator);
    }
    rapidjson::OStreamWrapper osw(oss);
    rapidjson::PrettyWriter<rapidjson::OStreamWrapper> writer(osw);
    root.Accept(writer);

    std::string dataToWrite = oss.str();
    auto count = FS::LoongFileSystem::StoreFileContent(filePath, dataToWrite.data(), dataToWrite.size());
    if (count != dataToWrite.size()) {
        LOONG_ERROR("Failed to write material '{}': {}", filePath, FS::LoongFileSystem::GetLastError());
        return false;
    }

    return true;
}

}
