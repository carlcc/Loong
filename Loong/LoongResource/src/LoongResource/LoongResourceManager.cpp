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

// clang-format off
template <class T> struct ResourceLoadCallback;
template <> struct ResourceLoadCallback<std::shared_ptr<LoongTexture>>  { using Type = LoongResourceManager::OnTextureLoadCallback; };
template <> struct ResourceLoadCallback<std::shared_ptr<LoongGpuModel>> { using Type = LoongResourceManager::OnModelLoadCallback; };
template <> struct ResourceLoadCallback<std::shared_ptr<LoongMaterial>> { using Type = LoongResourceManager::OnMaterialLoadCallback; };
// clang-format on

template <class T>
struct ResourceLoaderInternal;
template <>
struct ResourceLoaderInternal<std::shared_ptr<LoongTexture>> {
    static std::shared_ptr<Foundation::LoongThreadTask> Load(const std::string& path, std::function<void(std::shared_ptr<LoongTexture>)>&& onRsrc)
    {
        return Foundation::LoongThreadPool::AddTask([path, onRsrc = std::move(onRsrc)](auto* task) -> bool {
            LOONG_TRACE("Load texture '{}'", path);
            auto tex = Resource::LoongTextureLoader::Create(path, RHI::LoongRHIManager::GetDevice(), true, [](const std::string& p) {
                LOONG_TRACE("Unload texture '{}'", p);
            });
            if (tex != nullptr) {
                LOONG_TRACE("Load texture '{}' succeed", path);
            } else {
                LOONG_TRACE("Load texture '{}' failed", path);
            }
            onRsrc(tex);
            return tex != nullptr;
        });
    }
};
template <>
struct ResourceLoaderInternal<std::shared_ptr<LoongGpuModel>> {
    static std::shared_ptr<Foundation::LoongThreadTask> Load(const std::string& path, std::function<void(std::shared_ptr<LoongGpuModel>)>&& onRsrc)
    {
        return Foundation::LoongThreadPool::AddTask([path, onRsrc = std::move(onRsrc)](auto* task) -> bool {
            Asset::LoongModel model(path);
            if (!model) {
                LOONG_ERROR("Load model '{}' failed", path);
                onRsrc(nullptr);
                return false;
            }
            LOONG_TRACE("Load GPU model '{}'", path);
            auto spGpuModel = std::shared_ptr<LoongGpuModel>(new LoongGpuModel(model, path), [](LoongGpuModel* mdl) {
                LOONG_TRACE("Unload GPU model '{}'", mdl->GetPath());
                delete mdl;
            });
            LOONG_ASSERT(spGpuModel != nullptr, "");
            onRsrc(spGpuModel);
            return true;
        });
    }
};
template <>
struct ResourceLoaderInternal<std::shared_ptr<LoongMaterial>> {
    static std::shared_ptr<Foundation::LoongThreadTask> Load(const std::string& path, std::function<void(std::shared_ptr<LoongMaterial>)>&& onRsrc)
    {
        LOONG_TRACE("Load material '{}'", path);
        return LoongMaterialLoader::CreateAsync(
            path,
            [](const std::string& path) {
                LOONG_TRACE("Unload material '{}'", path);
            },
            [onRsrc, path](const std::shared_ptr<LoongMaterial>& material) {
                if (material == nullptr) {
                    LOONG_TRACE("Load material '{}' failed", path);
                } else {
                    LOONG_TRACE("Load material '{}' succeed", path);
                }
                onRsrc(material);
            } //
        );
    }
};

// TODO: Replace it with a better cache: with resource budget and release strategy
// This class is thread safe
// TODO: Optimize, a more parallel implementation
template <class T>
struct ResourceCache {
public:
    using CallbackType = typename ResourceLoadCallback<T>::Type;
    struct LoadingInfo {
        /// The loading task
        std::shared_ptr<Foundation::LoongThreadTask> task;
        /// The callback to perform
        std::vector<CallbackType> callbacks;
    };

    std::shared_ptr<Foundation::LoongThreadTask> GetAsync(const std::string& key, CallbackType&& cb)
    {
        std::unique_lock<std::mutex> lck(poolMutex_);
        if (auto it = pool_.find(key); it != pool_.end()) {
            // 1. If the resource has been loaded, return it
            return Foundation::LoongThreadPool::AddTask([rsrc = it->second, cb = std::move(cb)](auto* task) {
                cb(rsrc);
                return true;
            });
        } else if (auto lit = resourceBeingLoaded_.find(key); lit != resourceBeingLoaded_.end()) {
            // 2. Else if the resource is being loaded, add the callback to pending callbacks
            lit->second.callbacks.push_back(std::move(cb));
            return lit->second.task;
        } else {
            // 3. Else load the resource
            auto task = ResourceLoaderInternal<T>::Load(key, [this, key](T rsrc) {
                std::vector<CallbackType> callbacks;
                {
                    std::unique_lock<std::mutex> lck(poolMutex_);
                    if (rsrc != nullptr) {
                        pool_.emplace(key, rsrc);
                    }

                    auto it = resourceBeingLoaded_.find(key);
                    LOONG_ASSERT(it != resourceBeingLoaded_.end(), "");
                    callbacks = std::move(it->second.callbacks);
                    resourceBeingLoaded_.erase(it);
                }
                for (auto& callback : callbacks) {
                    callback(rsrc);
                }
            });
            resourceBeingLoaded_.emplace(key, LoadingInfo { task, std::vector<CallbackType> { std::move(cb) } });
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

std::shared_ptr<Foundation::LoongThreadTask> LoongResourceManager::GetTextureAsync(const std::string& path, LoongResourceManager::OnTextureLoadCallback&& cb)
{
    return gTextureCache.GetAsync(path, std::move(cb));
}

std::shared_ptr<Foundation::LoongThreadTask> LoongResourceManager::GetModelAsync(const std::string& path, LoongResourceManager::OnModelLoadCallback&& cb)
{
    return gModelCache.GetAsync(path, std::move(cb));
}

std::shared_ptr<Foundation::LoongThreadTask> LoongResourceManager::GetMaterialAsync(const std::string& path, LoongResourceManager::OnMaterialLoadCallback&& cb)
{
    return gMaterialCache.GetAsync(path, std::move(cb));
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
