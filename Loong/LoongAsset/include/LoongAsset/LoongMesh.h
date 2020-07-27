//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//
#pragma once

#include "LoongAsset/LoongVertex.h"
#include <cstdint>
#include <map>
#include <string>
#include <vector>

namespace Loong::Math {
class AABB;
}

namespace Loong::Asset {

class LoongMesh {
public:
    using BoneNameToIndexMap = std::map<std::string, uint32_t>;
    // Each of this struct's instance is a vertex attribute
    struct BoneBinding {
        Math::IVector4 boneIndices { 0 };
        Math::Vector4 boneWeights { 0.0F };

        template <class Archive>
        bool Serialize(Archive& archive) { return archive(boneIndices, boneWeights); }
    };

    LoongMesh() = default;
    LoongMesh(std::vector<LoongVertex>&& vertices, std::vector<uint32_t>&& indices, std::vector<BoneBinding>&& bones,
        BoneNameToIndexMap&& boneNameToIndex, uint32_t materialIndex);
    virtual ~LoongMesh() = default;
    const std::vector<LoongVertex>& GetVertices() const { return vertices_; }
    const std::vector<uint32_t>& GetIndices() const { return indices_; }
    uint32_t GetMaterialIndex() const { return materialIndex_; }

    const Math::AABB& GetAABB() const { return aabb_; }

    template <class Archive>
    bool Serialize(Archive& archive) { return archive(vertices_, indices_, bones_, boneNameToIndex_, materialIndex_, aabb_); }

private:
    void UpdateAABB();

protected:
    std::vector<LoongVertex> vertices_;
    std::vector<uint32_t> indices_;
    std::vector<BoneBinding> bones_;
    BoneNameToIndexMap boneNameToIndex_;
    uint32_t materialIndex_ { 0 };
    Math::AABB aabb_ {};
};

}
