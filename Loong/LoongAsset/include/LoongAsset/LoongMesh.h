//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//
#pragma once

#include "LoongAsset/LoongVertex.h"
#include <cstdint>
#include <vector>

namespace Loong::Math {
class AABB;
}

namespace Loong::Asset {

class LoongMesh {
public:
    LoongMesh(std::vector<LoongVertex>&& vertices, std::vector<uint32_t> indices, uint32_t materialIndex);
    virtual ~LoongMesh() = default;
    const std::vector<LoongVertex>& GetVertices() const { return vertices_; }
    const std::vector<uint32_t>& GetIndices() const { return indices_; }
    uint32_t GetMaterialIndex() const { return materialIndex_; }

    const Math::AABB& GetAABB() const { return aabb_; }

    template <class Archive>
    bool Serialize(Archive& archive) { return archive(vertices_, indices_, materialIndex_, aabb_); }

private:
    void UpdateAABB();

protected:
    std::vector<LoongVertex> vertices_;
    std::vector<uint32_t> indices_;
    uint32_t materialIndex_ { 0 };
    Math::AABB aabb_ {};
};

}
