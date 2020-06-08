//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#include "LoongResource/LoongResourceManager.h"
#include "LoongAsset/LoongImage.h"
#include "LoongFoundation/LoongLogger.h"
#include "LoongResource/LoongTexture.h"
#include "loader/LoongTextureLoader.h"
#include <cassert>
#include <map>

namespace Loong::Resource {

static std::map<std::string, std::weak_ptr<LoongTexture>> gLoadedTextures;

bool LoongResourceManager::Initialize()
{
    return true;
}

void LoongResourceManager::Uninitialize()
{
    gLoadedTextures.clear();
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

}
