//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#pragma once

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

    static std::shared_ptr<LoongTexture> GetTexture(const std::string& path);

    static std::shared_ptr<LoongGpuModel> GetModel(const std::string& path);

    static std::shared_ptr<LoongMaterial> GetMaterial(const std::string& path);

    static std::shared_ptr<LoongGpuMesh> GetSkyboxMesh();
};

}