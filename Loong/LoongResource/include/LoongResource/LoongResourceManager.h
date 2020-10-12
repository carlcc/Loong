//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#pragma once

#include "LoongFoundation/LoongMacros.h"
#include "LoongRHI/LoongRHIManager.h"
#include <memory>
#include <string>

namespace Loong::Foundation {
class LoongThreadTask;
}

namespace Loong::Resource {

class LoongGpuModel;
class LoongGpuMesh;
class LoongMaterial;
class LoongTexture;

class LoongResourceManager {
public:
    LoongResourceManager() = delete;

    static bool Initialize();

    static void Uninitialize();

    using OnTextureLoadCallback = std::function<void(std::shared_ptr<LoongTexture>)>;
    static std::shared_ptr<Foundation::LoongThreadTask> GetTextureAsync(const std::string& path, OnTextureLoadCallback&& cb);

    using OnModelLoadCallback = std::function<void(std::shared_ptr<LoongGpuModel>)>;
    static std::shared_ptr<Foundation::LoongThreadTask> GetModelAsync(const std::string& path, OnModelLoadCallback&& cb);

    using OnMaterialLoadCallback = std::function<void(std::shared_ptr<LoongMaterial>)>;
    static std::shared_ptr<Foundation::LoongThreadTask> GetMaterialAsync(const std::string& path, OnMaterialLoadCallback&& cb);

    LG_NODISCARD static std::shared_ptr<LoongGpuMesh> GetSkyboxMesh();
};

}