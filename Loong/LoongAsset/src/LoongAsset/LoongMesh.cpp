//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#include "LoongAsset/LoongMesh.h"
#include <algorithm>

namespace Loong::Asset {

LoongMesh::LoongMesh(std::vector<LoongVertex>&& vertices, std::vector<uint32_t>&& indices, uint32_t materialIndex)
    : vertices_(std::move(vertices))
    , indices_(std::move(indices))
    , materialIndex_(materialIndex)
{
    UpdateAABB();
}

void LoongMesh::UpdateAABB()
{
    if (vertices_.empty()) {
        aabb_.min = { 0.F, 0.F, 0.F };
        aabb_.max = { 0.F, 0.F, 0.F };
        return;
    }
    constexpr float kIninity = std::numeric_limits<float>::infinity();
    aabb_.min = { kIninity, kIninity, kIninity };
    aabb_.max = { -kIninity, -kIninity, -kIninity };

    auto& minX = aabb_.min.x;
    auto& minY = aabb_.min.y;
    auto& minZ = aabb_.min.z;
    auto& maxX = aabb_.max.x;
    auto& maxY = aabb_.max.y;
    auto& maxZ = aabb_.max.z;
    for (const auto& vertex : vertices_) {
        minX = std::min(minX, vertex.position[0]);
        minY = std::min(minY, vertex.position[1]);
        minZ = std::min(minZ, vertex.position[2]);

        maxX = std::max(maxX, vertex.position[0]);
        maxY = std::max(maxY, vertex.position[1]);
        maxZ = std::max(maxZ, vertex.position[2]);
    }
}

}