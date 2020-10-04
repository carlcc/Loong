//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#pragma once
#include "LoongFoundation/LoongMacros.h"
#include "LoongFoundation/LoongMath.h"
#include "LoongRHI/LoongRHIManager.h"
#include <cstdint>
#include <memory>

namespace Loong::Asset {
class LoongMesh;
struct LoongVertex;
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

    LG_NODISCARD uint32_t GetVertexCount() const { return verticesCount_; }
    LG_NODISCARD uint32_t GetIndexCount() const { return indicesCount_; }
    LG_NODISCARD uint32_t GetMaterialIndex() const { return materialIndex_; }
    LG_NODISCARD const Math::AABB& GetAABB() const { return aabb_; }

    LG_NODISCARD RHI::RefCntAutoPtr<RHI::IBuffer> GetVBO() const { return vbo_; }
    LG_NODISCARD RHI::RefCntAutoPtr<RHI::IBuffer> GetIBO() const { return ibo_; }

private:
    uint32_t verticesCount_ { 0 };
    uint32_t indicesCount_ { 0 };
    uint32_t materialIndex_ { 0 };

    RHI::RefCntAutoPtr<RHI::IBuffer> vbo_ {};
    RHI::RefCntAutoPtr<RHI::IBuffer> ibo_ {};

    Math::AABB aabb_ {};
};

}