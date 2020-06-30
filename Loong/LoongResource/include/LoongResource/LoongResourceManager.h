//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#pragma once

#include <memory>
#include <string>

namespace Loong::Resource {

class LoongTexture;
class LoongGpuModel;
class LoongShader;
class LoongMaterial;
class LoongRuntimeShader;

class LoongResourceManager {
public:
    LoongResourceManager() = delete;

    static bool Initialize();

    static void Uninitialize();

    static std::shared_ptr<LoongTexture> GetTexture(const std::string& path);

    static std::shared_ptr<LoongGpuModel> GetModel(const std::string& path);

    static std::shared_ptr<LoongShader> GetShader(const std::string& path);

    static std::shared_ptr<LoongShader> GetRuntimeShader(const LoongRuntimeShader& rs);

    static std::shared_ptr<LoongMaterial> GetMaterial(const std::string& path);
};

}