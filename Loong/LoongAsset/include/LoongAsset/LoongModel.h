//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//
#pragma once

#include "LoongFoundation/LoongMacros.h"
#include "LoongFoundation/LoongMath.h"
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace Loong::Asset {

class LoongMesh;

class LoongModel {
public:
    LoongModel() = default;
    LoongModel(std::vector<LoongMesh*>&& meshes, std::vector<std::string>&& materialNames)
        : meshes_(std::move(meshes))
        , materialNames_(std::move(materialNames))
    {
        UpdateAABB();
    }

    ~LoongModel();

    LG_NODISCARD const std::vector<LoongMesh*>& GetMeshes() const { return meshes_; }

    LG_NODISCARD const std::vector<std::string>& GetMaterialNames() const { return materialNames_; }

    LG_NODISCARD const Math::AABB& GetAABB() const { return aabb_; }

    bool operator!() const { return meshes_.empty() && materialNames_.empty(); }

    explicit operator bool() const { return meshes_.size() > 0 || materialNames_.size() > 0; }

    bool LoadFromFile(const std::string& vfsPath);

    bool WriteToFilePhysical(const std::string& physicalPath) const;

private:
    void UpdateAABB();

    void Clear();

    bool WriteToBuffer(void* flatbuffer, const std::string& path) const;

private:
    std::vector<LoongMesh*> meshes_ {};
    std::vector<std::string> materialNames_ {};

    Math::AABB aabb_ {};
};

}