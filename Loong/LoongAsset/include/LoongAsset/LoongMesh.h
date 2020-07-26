//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//
#pragma once

#include "LoongAsset/LoongVertex.h"
#include <cstdint>
#include <string>
#include <vector>

namespace Loong::Math {
class AABB;
}

namespace Loong::Asset {

class LoongMesh {
public:
    struct Bone {
        struct Weight {
            uint32_t vertexId;
            float weight;

            template <class Archive>
            bool Serialize(Archive& archive) { return archive(vertexId, weight); }
        };
        std::string name;
        std::vector<Weight> weights;

        template <class Archive>
        bool Serialize(Archive& archive) { return archive(name, weights); }
    };

    LoongMesh() = default;
    LoongMesh(std::vector<LoongVertex>&& vertices, std::vector<uint32_t>&& indices, std::vector<Bone>&& bones, uint32_t materialIndex);
    virtual ~LoongMesh() = default;
    const std::vector<LoongVertex>& GetVertices() const { return vertices_; }
    const std::vector<uint32_t>& GetIndices() const { return indices_; }
    uint32_t GetMaterialIndex() const { return materialIndex_; }

    const Math::AABB& GetAABB() const { return aabb_; }

    template <class Archive>
    bool Serialize(Archive& archive) { return archive(vertices_, indices_, bones_, materialIndex_, aabb_); }

private:
    void UpdateAABB();

protected:
    std::vector<LoongVertex> vertices_;
    std::vector<uint32_t> indices_;
    std::vector<Bone> bones_;
    uint32_t materialIndex_ { 0 };
    Math::AABB aabb_ {};
};

}
