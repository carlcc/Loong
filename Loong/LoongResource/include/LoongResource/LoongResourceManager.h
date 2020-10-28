//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#pragma once

#include "LoongFoundation/LoongMacros.h"
#include "LoongRHI/LoongRHIManager.h"
#include <TPL/TPL.h>
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

    using TextureRef = std::shared_ptr<LoongTexture>;
    static tpl::Task<TextureRef> GetTextureAsync(const std::string& path);

    using ModelRef = std::shared_ptr<LoongGpuModel>;
    static tpl::Task<ModelRef> GetModelAsync(const std::string& path);

    using MaterialRef = std::shared_ptr<LoongMaterial>;
    static tpl::Task<MaterialRef> GetMaterialAsync(const std::string& path);

    LG_NODISCARD static std::shared_ptr<LoongGpuMesh> GetSkyboxMesh();
};

}