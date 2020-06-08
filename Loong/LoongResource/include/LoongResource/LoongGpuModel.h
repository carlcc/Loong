//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#pragma once

#include "LoongFoundation/LoongMath.h"
#include <vector>

namespace Loong::Asset {
class LoongModel;
}

namespace Loong::Resource {

class LoongGpuMesh;

class LoongGpuModel {
public:
    explicit LoongGpuModel(const Asset::LoongModel& model);
    LoongGpuModel(const LoongGpuModel&) = delete;
    LoongGpuModel(LoongGpuModel&) = delete;
    ~LoongGpuModel();
    LoongGpuModel& operator=(const LoongGpuModel&) = delete;
    LoongGpuModel& operator=(LoongGpuModel&) = delete;

private:
    std::vector<LoongGpuMesh*> meshes_ {};
    std::vector<std::string> materialNames_ {};

    Math::AABB aabb_ {};
};

}