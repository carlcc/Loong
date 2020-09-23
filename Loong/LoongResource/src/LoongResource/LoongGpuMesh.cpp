//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#include "LoongResource/LoongGpuMesh.h"
#include "LoongAsset/LoongMesh.h"

namespace Loong::Resource {

LoongGpuMesh::LoongGpuMesh(const Asset::LoongMesh& mesh)
    : verticesCount_(uint32_t(mesh.GetVertices().size()))
    , indicesCount_(uint32_t(mesh.GetIndices().size()))
    , materialIndex_(uint32_t(mesh.GetMaterialIndex()))
{
    verticesCount_ = (uint32_t)mesh.GetVertices().size();
    indicesCount_ = (uint32_t)mesh.GetIndices().size();
    materialIndex_ = mesh.GetMaterialIndex();
    vbo_ = RHI::LoongRHIManager::CreateVertexBuffer("Mesh vertex buffer", verticesCount_ * sizeof(mesh.GetVertices()[0]), mesh.GetVertices().data());
    ibo_ = RHI::LoongRHIManager::CreateIndexBuffer("Mesh index buffer", indicesCount_ * sizeof(mesh.GetIndices()[0]), mesh.GetIndices().data());
    aabb_ = mesh.GetAABB();
}

}
