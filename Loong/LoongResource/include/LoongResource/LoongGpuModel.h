//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#pragma once

#include "LoongFoundation/LoongMath.h"
#include <string>
#include <vector>

namespace Loong::Asset {
class LoongModel;
}

namespace Loong::Resource {

class LoongGpuMesh;

class LoongGpuModel {
public:
    explicit LoongGpuModel(const Asset::LoongModel& model, const std::string& path);
    LoongGpuModel(const LoongGpuModel&) = delete;
    LoongGpuModel(LoongGpuModel&) = delete;
    ~LoongGpuModel();
    LoongGpuModel& operator=(const LoongGpuModel&) = delete;
    LoongGpuModel& operator=(LoongGpuModel&) = delete;

    const std::vector<LoongGpuMesh*>& GetMeshes() const { return meshes_; }
    const std::vector<std::string>& GetMaterialNames() const { return materialNames_; }
    const Math::AABB GetAABB() const { return aabb_; }

    const std::string& GetPath() const { return path_; }

private:
    std::vector<LoongGpuMesh*> meshes_ {};
    std::vector<std::string> materialNames_ {};

    Math::AABB aabb_ {};
    std::string path_ {};
};

}