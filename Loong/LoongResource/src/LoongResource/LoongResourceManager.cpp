//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#include "LoongResource/LoongResourceManager.h"
#include "LoongAsset/LoongImage.h"
#include "LoongAsset/LoongModel.h"
#include "LoongFoundation/LoongLogger.h"
#include "LoongResource/LoongGpuModel.h"
#include "LoongResource/LoongTexture.h"
#include "loader/LoongTextureLoader.h"
#include <cassert>
#include <map>

namespace Loong::Resource {

static std::map<std::string, std::weak_ptr<LoongTexture>> gLoadedTextures;
static std::map<std::string, std::weak_ptr<LoongGpuModel>> gLoadedModels;

bool LoongResourceManager::Initialize()
{
    return true;
}

void LoongResourceManager::Uninitialize()
{
    gLoadedTextures.clear();
    gLoadedModels.clear();
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
    auto* gpuModel = new LoongGpuModel(model);
    std::shared_ptr<LoongGpuModel> spGpuModel(gpuModel, [path](LoongGpuModel* m) {
        gLoadedModels.erase(path);
        delete m;
    });
    assert(gpuModel != nullptr);

    gLoadedModels.insert({ path, spGpuModel });
    LOONG_TRACE("Load GPU model '{}' succeed", path);
    return spGpuModel;
}

}
