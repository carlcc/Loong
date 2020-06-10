//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#include "LoongResource/LoongResourceManager.h"
#include "LoongAsset/LoongImage.h"
#include "LoongAsset/LoongModel.h"
#include "LoongAsset/LoongShaderCode.h"
#include "LoongFoundation/LoongDefer.h"
#include "LoongFoundation/LoongLogger.h"
#include "LoongResource/LoongGpuModel.h"
#include "LoongResource/LoongMaterial.h"
#include "LoongResource/LoongShader.h"
#include "LoongResource/LoongTexture.h"
#include "loader/LoongTextureLoader.h"
#include <cassert>
#include <map>
#include <vector>

namespace Loong::Resource {

static std::map<std::string, std::weak_ptr<LoongTexture>> gLoadedTextures;
static std::map<std::string, std::weak_ptr<LoongGpuModel>> gLoadedModels;
static std::map<std::string, std::weak_ptr<LoongShader>> gLoadedShaders;
static std::map<std::string, std::weak_ptr<LoongMaterial>> gLoadedMaterials;

bool LoongResourceManager::Initialize()
{
    return true;
}

void LoongResourceManager::Uninitialize()
{
    gLoadedTextures.clear();
    gLoadedModels.clear();
    gLoadedShaders.clear();
    gLoadedMaterials.clear();
}

std::shared_ptr<LoongTexture> LoongResourceManager::GetTexture(const std::string& path)
{
    auto it = gLoadedTextures.find(path);
    if (it != gLoadedTextures.end()) {
        auto sp = it->second.lock();
        assert(sp != nullptr);
        return sp;
    }

    Asset::LoongImage image(path);
    if (!image) {
        LOONG_ERROR("Load image '{}' failed", path);
        return nullptr;
    }

    LOONG_TRACE("Load texture '{}'", path);
    auto texture = LoongTextureLoader::Create(image, true, [](const std::string& p) {
        gLoadedTextures.erase(p);
        LOONG_TRACE("Unload texture '{}'", p);
    });
    if (texture != nullptr) {
        gLoadedTextures.insert({ path, texture });
        LOONG_TRACE("Load texture '{}' succeed", path);
    } else {
        LOONG_ERROR("Load texture '{}' failed", path);
    }
    return texture;
}

std::shared_ptr<LoongGpuModel> LoongResourceManager::GetModel(const std::string& path)
{
    auto it = gLoadedModels.find(path);
    if (it != gLoadedModels.end()) {
        auto sp = it->second.lock();
        assert(sp != nullptr);
        return sp;
    }

    Asset::LoongModel model(path);
    if (!model) {
        LOONG_ERROR("Load model '{}' failed", path);
        return nullptr;
    }

    LOONG_TRACE("Load GPU model '{}'", path);
    auto* gpuModel = new LoongGpuModel(model, path);
    std::shared_ptr<LoongGpuModel> spGpuModel(gpuModel, [path](LoongGpuModel* m) {
        gLoadedModels.erase(path);
        delete m;
    });
    assert(gpuModel != nullptr);

    gLoadedModels.insert({ path, spGpuModel });
    LOONG_TRACE("Load GPU model '{}' succeed", path);
    return spGpuModel;
}

static const char* GetShaderTypeName(uint32_t type)
{
    switch (type) {
    case GL_VERTEX_SHADER:
        return "VERTEX SHADER";
    case GL_FRAGMENT_SHADER:
        return "FRAGMENT SHADER";
    case GL_GEOMETRY_SHADER:
        return "GEOMETRY SHADER";
    default:
        return "UNKNOWN SHADER";
    }
}
static GLuint CompileShader(const std::string& filePath, uint32_t type, const std::string& source)
{
    const uint32_t id = glCreateShader(type);
    const char* src = source.c_str();

    glShaderSource(id, 1, &src, nullptr);
    glCompileShader(id);

    GLint compileStatus;
    glGetShaderiv(id, GL_COMPILE_STATUS, &compileStatus);
    if (compileStatus == GL_FALSE) {
        GLint maxLength;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &maxLength);
        maxLength++;

        std::vector<char> errorLog(maxLength);
        glGetShaderInfoLog(id, maxLength, &maxLength, errorLog.data());

        LOONG_ERROR("Compile {} {} failed: {}", GetShaderTypeName(type), filePath, errorLog.data());

        glDeleteShader(id);
        return 0;
    }

    return id;
}

static GLuint CreateProgram(const std::string& filePath, const std::vector<std::pair<uint32_t, const std::string&>>& shaders)
{
    const uint32_t program = glCreateProgram();
    if (program == 0) {
        LOONG_ERROR("Create shader program for {} failed", filePath);
        return 0;
    }
    Foundation::LoongDefer deferDeleteProgram([program]() {
        glDeleteProgram(program);
    });

    std::vector<Foundation::LoongDefer> deferDeleteShaders;

    for (auto& [shaderType, shaderSource] : shaders) {
        const uint32_t shaderId = CompileShader(filePath, shaderType, shaderSource);
        if (shaderId == 0) {
            LOONG_ERROR("Create shader for {} failed", filePath);
            return 0;
        }
        deferDeleteShaders.emplace_back([shaderId]() {
            glDeleteShader(shaderId);
        });
        glAttachShader(program, shaderId);
    }
    glLinkProgram(program);

    GLint linkStatus;
    glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);

    if (linkStatus == GL_FALSE) {
        GLint maxLength;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);
        ++maxLength;

        std::vector<char> errorLog(maxLength);
        glGetProgramInfoLog(program, maxLength, &maxLength, errorLog.data());

        LOONG_ERROR("Link program for {} failed: {}", filePath, errorLog.data());

        return 0;
    }

    glValidateProgram(program);

    deferDeleteProgram.Cancel();
    return program;
}

std::shared_ptr<LoongShader> LoongResourceManager::GetShader(const std::string& path)
{
    auto it = gLoadedShaders.find(path);
    if (it != gLoadedShaders.end()) {
        auto sp = it->second.lock();
        assert(sp != nullptr);
        return sp;
    }

    Asset::LoongShaderCode code(path);
    if (!code) {
        LOONG_ERROR("Load shader code '{}' failed", path);
        return nullptr;
    }

    LOONG_TRACE("Load shader '{}'", path);
    std::vector<std::pair<uint32_t, const std::string&>> shaderSources;
    // clang-format off
    if (!code.GetVertexShader().empty())   shaderSources.emplace_back(GL_VERTEX_SHADER,   code.GetVertexShader());
    if (!code.GetFragmentShader().empty()) shaderSources.emplace_back(GL_FRAGMENT_SHADER, code.GetFragmentShader());
    if (!code.GetGeometryShader().empty()) shaderSources.emplace_back(GL_GEOMETRY_SHADER, code.GetGeometryShader());
    // clang-format on

    uint32_t program = CreateProgram(path, shaderSources);
    if (program == 0) {
        return nullptr;
    }
    auto* shaderProgram = new LoongShader(program);
    std::shared_ptr<LoongShader> spShaderProgram(shaderProgram, [path](LoongShader* m) {
        gLoadedShaders.erase(path);
        delete m;
    });
    assert(spShaderProgram != nullptr);

    gLoadedShaders.insert({ path, spShaderProgram });
    LOONG_TRACE("Load shader '{}' succeed", path);
    return spShaderProgram;
}

std::shared_ptr<LoongMaterial> LoongResourceManager::GetMaterial(const std::string& path)
{
    return nullptr; // TODO: NYI
//    auto it = gLoadedMaterials.find(path);
//    if (it != gLoadedMaterials.end()) {
//        auto sp = it->second.lock();
//        assert(sp != nullptr);
//        return sp;
//    }
//
//    Asset::LoongImage image(path);
//    if (!image) {
//        LOONG_ERROR("Load image '{}' failed", path);
//        return nullptr;
//    }
//
//    LOONG_TRACE("Load texture '{}'", path);
//    auto texture = LoongTextureLoader::Create(image, true, [](const std::string& p) {
//        gLoadedTextures.erase(p);
//        LOONG_TRACE("Unload texture '{}'", p);
//    });
//    if (texture != nullptr) {
//        gLoadedTextures.insert({ path, texture });
//        LOONG_TRACE("Load texture '{}' succeed", path);
//    } else {
//        LOONG_ERROR("Load texture '{}' failed", path);
//    }
//    return texture;
}

}
