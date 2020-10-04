//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#pragma once

#include "LoongFoundation/LoongMacros.h"
#include "LoongRHI/LoongRHIManager.h"
#include <memory>
#include <string>

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

    LG_NODISCARD static std::shared_ptr<LoongTexture> GetTexture(const std::string& path);

    LG_NODISCARD static std::shared_ptr<LoongGpuModel> GetModel(const std::string& path);

    LG_NODISCARD static std::shared_ptr<LoongMaterial> GetMaterial(const std::string& path);

    LG_NODISCARD static std::shared_ptr<LoongGpuMesh> GetSkyboxMesh();
};

}