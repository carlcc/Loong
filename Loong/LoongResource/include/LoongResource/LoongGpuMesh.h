//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#pragma once
#include "LoongFoundation/LoongMath.h"
#include "LoongResource/LoongGpuBuffer.h"
#include "LoongResource/LoongVertexArray.h"
#include <cstdint>
#include <memory>

namespace Loong::Asset {
class LoongMesh;
class LoongVertex;
}

namespace Loong::Resource {

class LoongGpuMesh {
public:
    explicit LoongGpuMesh(const Asset::LoongMesh& mesh);
    LoongGpuMesh(const LoongGpuMesh&) = delete;
    LoongGpuMesh(LoongGpuMesh&&) = delete;
    ~LoongGpuMesh() = default;
    LoongGpuMesh& operator=(const LoongGpuMesh&) = delete;
    LoongGpuMesh& operator=(LoongGpuMesh&&) = delete;

    void Bind() const { vao_.Bind(); }
    void Unbind() const { vao_.Unbind(); }
    uint32_t GetVertexCount() const { return verticesCount_; }
    uint32_t GetIndexCount() const { return indicesCount_; }
    uint32_t GetMaterialIndex() const { return materialIndex_; }
    const Math::AABB& GetAABB() const { return aabb_; }

private:
    void CreateBuffers(const Asset::LoongVertex* vertices, size_t verticesCount, const uint32_t* indices, size_t indicesCount);

private:
    const uint32_t verticesCount_ { 0 };
    const uint32_t indicesCount_ { 0 };
    const uint32_t materialIndex_ { 0 };

    LoongVertexArray vao_ {};
    std::unique_ptr<LoongVertexBuffer> vbo_ {};
    std::unique_ptr<LoongIndexBuffer> ibo_ {};

    Math::AABB aabb_ {};
};

}