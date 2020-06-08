//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#include "LoongAsset/LoongModel.h"
#include "LoongAsset/LoongMesh.h"
#include "LoongAssimpModelLoader.h"
#include <algorithm>

namespace Loong::Asset {

LoongModel::LoongModel(const std::string& path)
{
    LoongAssimpModelLoader parser;
    if (parser.LoadModel(path, meshes_, materialNames_)) {
        UpdateAABB();
    }
}

LoongModel::~LoongModel()
{
    for (auto* mesh : meshes_) {
        delete mesh;
    }
}

void LoongModel::UpdateAABB()
{
    if (meshes_.empty()) {
        aabb_ = {
            { 0.F, 0.F, 0.F },
            { 0.F, 0.F, 0.F },
        };
        return;
    }

    aabb_ = meshes_[0]->GetAABB();
    for (size_t i = 1; i < meshes_.size(); ++i) {
        auto& mesh = *meshes_[i];
        auto& meshAabb = mesh.GetAABB();
        aabb_.min.x = std::min(aabb_.min.x, meshAabb.min.x);
        aabb_.min.y = std::min(aabb_.min.y, meshAabb.min.y);
        aabb_.min.z = std::min(aabb_.min.z, meshAabb.min.z);
        aabb_.max.x = std::min(aabb_.max.x, meshAabb.max.x);
        aabb_.max.y = std::min(aabb_.max.y, meshAabb.max.y);
        aabb_.max.z = std::min(aabb_.max.z, meshAabb.max.z);
    }
}

}
