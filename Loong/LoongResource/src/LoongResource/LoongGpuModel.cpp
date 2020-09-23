//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#include "LoongResource/LoongGpuModel.h"
#include "LoongAsset/LoongModel.h"
#include "LoongResource/LoongGpuMesh.h"

namespace Loong::Resource {

LoongGpuModel::LoongGpuModel(const Asset::LoongModel& model, const std::string& path)
{
    meshes_.reserve(model.GetMeshes().size());
    materialNames_ = model.GetMaterialNames();
    aabb_ = model.GetAABB();
    for (auto* mesh : model.GetMeshes()) {
        meshes_.emplace_back(new Resource::LoongGpuMesh(*mesh));
    }
    path_ = path;
}

LoongGpuModel::~LoongGpuModel()
{
    for (auto* mesh : meshes_) {
        delete mesh;
    }
    meshes_.clear();
}

}
