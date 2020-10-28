//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#include "LoongResource/LoongResourceManager.h"
#include "LoongAsset/LoongMesh.h"
#include "LoongAsset/LoongModel.h"
#include "LoongFoundation/LoongAssert.h"
#include "LoongFoundation/LoongDefer.h"
#include "LoongFoundation/LoongLogger.h"
#include "LoongFoundation/LoongThreadPool.h"
#include "LoongResource/LoongGpuMesh.h"
#include "LoongResource/LoongGpuModel.h"
#include "LoongResource/LoongMaterial.h"
#include "LoongResource/loader/LoongMaterialLoader.h"
#include "LoongResource/loader/LoongTextureLoader.h"
#include <cassert>
#include <map>
#include <vector>

namespace Loong::Resource {

template <class T>
struct ResourceLoaderInternal;

template <>
struct ResourceLoaderInternal<std::shared_ptr<LoongTexture>> {
    static tpl::Task<std::shared_ptr<LoongTexture>> Load(const std::string& path)
    {
        return tpl::MakeTaskAndStart([path]() -> std::shared_ptr<LoongTexture> {
            LOONG_TRACE("Load texture '{}'", path);
            auto tex = Resource::LoongTextureLoader::Create(path, RHI::LoongRHIManager::GetDevice(), true, [](const std::string& p) {
                LOONG_TRACE("Unload texture '{}'", p);
            });
            if (tex != nullptr) {
                LOONG_TRACE("Load texture '{}' succeed", path);
            } else {
                LOONG_TRACE("Load texture '{}' failed", path);
            }
            return tex;
        }, nullptr);
    }
};

template <>
struct ResourceLoaderInternal<std::shared_ptr<LoongGpuModel>> {
    static tpl::Task<std::shared_ptr<LoongGpuModel>> Load(const std::string& path)
    {
        return tpl::MakeTaskAndStart([path]() -> std::shared_ptr<LoongGpuModel> {
            Asset::LoongModel model(path);
            if (!model) {
                LOONG_ERROR("Load model '{}' failed", path);
                return nullptr;
            }
            LOONG_TRACE("Load GPU model '{}'", path);
            std::shared_ptr<LoongGpuModel> spGpuModel(new LoongGpuModel(model, path), [](LoongGpuModel* mdl) {
                LOONG_TRACE("Unload GPU model '{}'", mdl->GetPath());
                delete mdl;
            });
            LOONG_ASSERT(spGpuModel != nullptr, "");
            return spGpuModel;
        }, nullptr);
    }
};

template <>
struct ResourceLoaderInternal<std::shared_ptr<LoongMaterial>> {
    static tpl::Task<std::shared_ptr<LoongMaterial>> Load(const std::string& path)
    {
        LOONG_TRACE("Load material '{}'", path);
        return LoongMaterialLoader::CreateAsync(
            path,
            [](const std::string& path) {
                LOONG_TRACE("Unload material '{}'", path);
            }
        );
    }
};

// TODO: Replace it with a better cache: with resource budget and release strategy
// This class is thread safe
// TODO: Optimize, a more parallel implementation
template <class T>
struct ResourceCache {
public:
    struct LoadingInfo {
        /// The loading task
        tpl::Task<T> task;
    };

    tpl::Task<T> GetAsync(const std::string& key)
    {
        std::unique_lock<std::mutex> lck(poolMutex_);
        if (auto it = pool_.find(key); it != pool_.end()) {
            // 1. If the resource has been loaded, return it
            T rsrc = it->second;
            return tpl::MakeTaskFromValue(rsrc, nullptr);
        } else if (auto lit = resourceBeingLoaded_.find(key); lit != resourceBeingLoaded_.end()) {
            // 2. Else if the resource is being loaded, return the loading task
            return lit->second.task;
        } else {
            // 3. Else load the resource
            auto task = ResourceLoaderInternal<T>::Load(key).Then(
                [this, key](const tpl::Task<T>& rsrcTask) {
                    {
                        T rsrc = rsrcTask.GetFuture().GetValue();
                        std::unique_lock<std::mutex> lck(poolMutex_);
                        if (rsrc != nullptr) {
                            pool_.emplace(key, rsrc);
                        }

                        auto it = resourceBeingLoaded_.find(key);
                        LOONG_ASSERT(it != resourceBeingLoaded_.end(), "");
                        resourceBeingLoaded_.erase(it);
                        return rsrc;
                    }
                });
            resourceBeingLoaded_.emplace(key, LoadingInfo { task });
            return task;
        }
    }

    void Clear()
    {
        std::unique_lock<std::mutex> lck(poolMutex_);
        pool_.clear();
    }

private:
    /// The loaded resources
    std::map<std::string, T> pool_ {};
    std::mutex poolMutex_ {};
    /// The resources being loaded
    /// The pending callbacks to invoke once the resource is loaded
    std::map<std::string, LoadingInfo> resourceBeingLoaded_ {};
};

static ResourceCache<std::shared_ptr<LoongTexture>> gTextureCache;
static ResourceCache<std::shared_ptr<LoongGpuModel>> gModelCache;
static ResourceCache<std::shared_ptr<LoongMaterial>> gMaterialCache;

static std::shared_ptr<LoongGpuMesh> gSkyBoxMesh;

bool LoongResourceManager::Initialize()
{
    LoongResourceManager::Uninitialize();
    return true;
}

void LoongResourceManager::Uninitialize()
{
    gTextureCache.Clear();
    gModelCache.Clear();
    gMaterialCache.Clear();
    gSkyBoxMesh = nullptr;
}

tpl::Task<LoongResourceManager::TextureRef> LoongResourceManager::GetTextureAsync(const std::string& path)
{
    return gTextureCache.GetAsync(path);
}

tpl::Task<LoongResourceManager::ModelRef> LoongResourceManager::GetModelAsync(const std::string& path)
{
    return gModelCache.GetAsync(path);
}

tpl::Task<LoongResourceManager::MaterialRef> LoongResourceManager::GetMaterialAsync(const std::string& path)
{
    return gMaterialCache.GetAsync(path);
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
