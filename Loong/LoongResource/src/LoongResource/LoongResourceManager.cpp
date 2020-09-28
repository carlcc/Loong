//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#include "LoongResource/LoongResourceManager.h"
#include "LoongAsset/LoongMesh.h"
#include "LoongAsset/LoongModel.h"
#include "LoongFoundation/LoongDefer.h"
#include "LoongFoundation/LoongLogger.h"
#include "LoongResource/LoongGpuMesh.h"
#include "LoongResource/LoongGpuModel.h"
#include "LoongResource/LoongMaterial.h"
#include "LoongResource/loader/LoongMaterialLoader.h"
#include "LoongResource/loader/LoongTextureLoader.h"
#include <cassert>
#include <map>
#include <vector>

namespace Loong::Resource {

// TODO: Replace it with a better cache: with resource budget and release strategy
// This class is not thread safe
template <class T>
struct ResourceCache {
public:
    T Get(const std::string& key)
    {
        auto it = pool_.find(key);
        if (it == pool_.end()) {
            return T {};
        }
        return it->second;
    }

    void Insert(const std::string& key, const T& t)
    {
        pool_.emplace(key, t);
    }

    void Clear() { pool_.clear(); }

private:
    std::map<std::string, T> pool_ {};
};

static ResourceCache<std::shared_ptr<LoongTexture>> gTextureCache;
static ResourceCache<std::shared_ptr<LoongGpuModel>> gModelCache;
static ResourceCache<std::shared_ptr<LoongMaterial>> gMaterialCache;

static std::shared_ptr<LoongGpuMesh> gSkyBoxMesh;

bool LoongResourceManager::Initialize()
{
    return true;
}

void LoongResourceManager::Uninitialize()
{
    gTextureCache.Clear();
    gModelCache.Clear();
    gMaterialCache.Clear();
    gSkyBoxMesh = nullptr;
}

std::shared_ptr<LoongTexture> LoongResourceManager::GetTexture(const std::string& path)
{
    auto texture = gTextureCache.Get(path);
    if (texture != nullptr) {
        return texture;
    }

    texture = Resource::LoongTextureLoader::Create(path, RHI::LoongRHIManager::GetDevice(), true, [](const std::string& p) {
        LOONG_TRACE("Unload texture '{}'", p);
    });
    if (texture != nullptr) {
        gTextureCache.Insert(path, texture);
        LOONG_TRACE("Load texture '{}' succeed", path);
    } else {
        LOONG_ERROR("Load texture '{}' failed", path);
    }
    return texture;
}

std::shared_ptr<LoongGpuModel> LoongResourceManager::GetModel(const std::string& path)
{
    auto spGpuModel = gModelCache.Get(path);
    if (spGpuModel != nullptr) {
        return spGpuModel;
    }

    Asset::LoongModel model(path);
    if (!model) {
        LOONG_ERROR("Load model '{}' failed", path);
        return nullptr;
    }

    LOONG_TRACE("Load GPU model '{}'", path);
    spGpuModel = std::make_shared<LoongGpuModel>(model, path);
    assert(spGpuModel != nullptr);

    gModelCache.Insert(path, spGpuModel);
    LOONG_TRACE("Load GPU model '{}' succeed", path);
    return spGpuModel;
}

std::shared_ptr<LoongMaterial> LoongResourceManager::GetMaterial(const std::string& path)
{
    auto spMaterial = gMaterialCache.Get(path);
    if (spMaterial != nullptr) {
        return spMaterial;
    }

    LOONG_TRACE("Load material '{}'", path);
    spMaterial = LoongMaterialLoader::Create(path, [](const std::string& path) {
        LOONG_TRACE("Unload material '{}'", path);
    });
    if (spMaterial != nullptr) {
        return nullptr;
    }

    gMaterialCache.Insert(path, spMaterial);
    LOONG_TRACE("Load material '{}' succeed", path);
    return spMaterial;
}

std::shared_ptr<LoongGpuMesh> LoongResourceManager::GetSkyboxMesh()
{
    if (gSkyBoxMesh == nullptr) {
        std::vector<Asset::LoongVertex> vertices {
            { Math::Vector3 { -1.0f, 1.0f, -1.0f }, Math::Vector2 {}, Math::Vector3 {}, Math::Vector3 {}, Math::Vector3 {} },
            { Math::Vector3 { -1.0f, -1.0f, -1.0f }, Math::Vector2 {}, Math::Vector3 {}, Math::Vector3 {}, Math::Vector3 {} },
            { Math::Vector3 { 1.0f, -1.0f, -1.0f }, Math::Vector2 {}, Math::Vector3 {}, Math::Vector3 {}, Math::Vector3 {} },
            { Math::Vector3 { 1.0f, -1.0f, -1.0f }, Math::Vector2 {}, Math::Vector3 {}, Math::Vector3 {}, Math::Vector3 {} },
            { Math::Vector3 { 1.0f, 1.0f, -1.0f }, Math::Vector2 {}, Math::Vector3 {}, Math::Vector3 {}, Math::Vector3 {} },
            { Math::Vector3 { -1.0f, 1.0f, -1.0f }, Math::Vector2 {}, Math::Vector3 {}, Math::Vector3 {}, Math::Vector3 {} },

            { Math::Vector3 { -1.0f, -1.0f, 1.0f }, Math::Vector2 {}, Math::Vector3 {}, Math::Vector3 {}, Math::Vector3 {} },
            { Math::Vector3 { -1.0f, -1.0f, -1.0f }, Math::Vector2 {}, Math::Vector3 {}, Math::Vector3 {}, Math::Vector3 {} },
            { Math::Vector3 { -1.0f, 1.0f, -1.0f }, Math::Vector2 {}, Math::Vector3 {}, Math::Vector3 {}, Math::Vector3 {} },
            { Math::Vector3 { -1.0f, 1.0f, -1.0f }, Math::Vector2 {}, Math::Vector3 {}, Math::Vector3 {}, Math::Vector3 {} },
            { Math::Vector3 { -1.0f, 1.0f, 1.0f }, Math::Vector2 {}, Math::Vector3 {}, Math::Vector3 {}, Math::Vector3 {} },
            { Math::Vector3 { -1.0f, -1.0f, 1.0f }, Math::Vector2 {}, Math::Vector3 {}, Math::Vector3 {}, Math::Vector3 {} },

            { Math::Vector3 { 1.0f, -1.0f, -1.0f }, Math::Vector2 {}, Math::Vector3 {}, Math::Vector3 {}, Math::Vector3 {} },
            { Math::Vector3 { 1.0f, -1.0f, 1.0f }, Math::Vector2 {}, Math::Vector3 {}, Math::Vector3 {}, Math::Vector3 {} },
            { Math::Vector3 { 1.0f, 1.0f, 1.0f }, Math::Vector2 {}, Math::Vector3 {}, Math::Vector3 {}, Math::Vector3 {} },
            { Math::Vector3 { 1.0f, 1.0f, 1.0f }, Math::Vector2 {}, Math::Vector3 {}, Math::Vector3 {}, Math::Vector3 {} },
            { Math::Vector3 { 1.0f, 1.0f, -1.0f }, Math::Vector2 {}, Math::Vector3 {}, Math::Vector3 {}, Math::Vector3 {} },
            { Math::Vector3 { 1.0f, -1.0f, -1.0f }, Math::Vector2 {}, Math::Vector3 {}, Math::Vector3 {}, Math::Vector3 {} },

            { Math::Vector3 { -1.0f, -1.0f, 1.0f }, Math::Vector2 {}, Math::Vector3 {}, Math::Vector3 {}, Math::Vector3 {} },
            { Math::Vector3 { -1.0f, 1.0f, 1.0f }, Math::Vector2 {}, Math::Vector3 {}, Math::Vector3 {}, Math::Vector3 {} },
            { Math::Vector3 { 1.0f, 1.0f, 1.0f }, Math::Vector2 {}, Math::Vector3 {}, Math::Vector3 {}, Math::Vector3 {} },
            { Math::Vector3 { 1.0f, 1.0f, 1.0f }, Math::Vector2 {}, Math::Vector3 {}, Math::Vector3 {}, Math::Vector3 {} },
            { Math::Vector3 { 1.0f, -1.0f, 1.0f }, Math::Vector2 {}, Math::Vector3 {}, Math::Vector3 {}, Math::Vector3 {} },
            { Math::Vector3 { -1.0f, -1.0f, 1.0f }, Math::Vector2 {}, Math::Vector3 {}, Math::Vector3 {}, Math::Vector3 {} },

            { Math::Vector3 { -1.0f, 1.0f, -1.0f }, Math::Vector2 {}, Math::Vector3 {}, Math::Vector3 {}, Math::Vector3 {} },
            { Math::Vector3 { 1.0f, 1.0f, -1.0f }, Math::Vector2 {}, Math::Vector3 {}, Math::Vector3 {}, Math::Vector3 {} },
            { Math::Vector3 { 1.0f, 1.0f, 1.0f }, Math::Vector2 {}, Math::Vector3 {}, Math::Vector3 {}, Math::Vector3 {} },
            { Math::Vector3 { 1.0f, 1.0f, 1.0f }, Math::Vector2 {}, Math::Vector3 {}, Math::Vector3 {}, Math::Vector3 {} },
            { Math::Vector3 { -1.0f, 1.0f, 1.0f }, Math::Vector2 {}, Math::Vector3 {}, Math::Vector3 {}, Math::Vector3 {} },
            { Math::Vector3 { -1.0f, 1.0f, -1.0f }, Math::Vector2 {}, Math::Vector3 {}, Math::Vector3 {}, Math::Vector3 {} },

            { Math::Vector3 { -1.0f, -1.0f, -1.0f }, Math::Vector2 {}, Math::Vector3 {}, Math::Vector3 {}, Math::Vector3 {} },
            { Math::Vector3 { -1.0f, -1.0f, 1.0f }, Math::Vector2 {}, Math::Vector3 {}, Math::Vector3 {}, Math::Vector3 {} },
            { Math::Vector3 { 1.0f, -1.0f, -1.0f }, Math::Vector2 {}, Math::Vector3 {}, Math::Vector3 {}, Math::Vector3 {} },
            { Math::Vector3 { 1.0f, -1.0f, -1.0f }, Math::Vector2 {}, Math::Vector3 {}, Math::Vector3 {}, Math::Vector3 {} },
            { Math::Vector3 { -1.0f, -1.0f, 1.0f }, Math::Vector2 {}, Math::Vector3 {}, Math::Vector3 {}, Math::Vector3 {} },
            { Math::Vector3 { 1.0f, -1.0f, 1.0f }, Math::Vector2 {}, Math::Vector3 {}, Math::Vector3 {}, Math::Vector3 {} },
        };
        std::vector<uint32_t> indices {
            0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
            10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
            20, 21, 22, 23, 24, 25, 26, 27, 28, 29,
            30, 31, 32, 33, 34, 35
        };
        Asset::LoongMesh mesh(std::move(vertices), std::move(indices), 0);
        gSkyBoxMesh = std::make_shared<LoongGpuMesh>(mesh);
    }

    return gSkyBoxMesh;
}

}
