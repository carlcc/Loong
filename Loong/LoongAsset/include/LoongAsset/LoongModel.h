//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//
#pragma once

#include "LoongFoundation/LoongMath.h"
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace Loong::Asset {

class LoongMesh;

class LoongModel {
public:
    explicit LoongModel(const std::string& path);

    ~LoongModel();

    const std::vector<LoongMesh*>& GetMeshes() const
    {
        return meshes_;
    }

    const std::vector<std::string>& GetMaterialNames() const
    {
        return materialNames_;
    }

    const Math::AABB& GetAABB() const
    {
        return aabb_;
    }

    bool operator!() const { return meshes_.empty() && materialNames_.empty(); }

    explicit operator bool() const { return meshes_.size() > 0 || materialNames_.size() > 0; }

private:
    void UpdateAABB();

private:
    std::vector<LoongMesh*> meshes_ {};
    std::vector<std::string> materialNames_ {};

    Math::AABB aabb_ {};
};

}