//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#include "LoongResource/LoongGpuMesh.h"
#include "LoongAsset/LoongMesh.h"

namespace Loong::Resource {

LoongGpuMesh::LoongGpuMesh(const Asset::LoongMesh& mesh)
{
    CreateBuffers(mesh.GetVertices().data(), mesh.GetVertices().size(), mesh.GetIndices().data(), mesh.GetIndices().size());
    aabb_ = mesh.GetAABB();
}

void LoongGpuMesh::CreateBuffers(const Asset::LoongVertex* vertices, size_t verticesCount, const uint32_t* indices, size_t indicesCount)
{
    vbo_ = std::make_unique<LoongVertexBuffer>();
    vbo_->BufferData(vertices, verticesCount);
    ibo_ = std::make_unique<LoongIndexBuffer>();
    ibo_->BufferData(indices, indicesCount);

    const GLsizei vertexSize = sizeof(Asset::LoongVertex);

    vao_.Bind();
    vbo_->Bind();
    ibo_->Bind();
    vao_.BindAttribute<float>(0, *vbo_, 3, vertexSize, 0);
    vao_.BindAttribute<float>(1, *vbo_, 2, vertexSize, sizeof(float) * 3);
    vao_.BindAttribute<float>(2, *vbo_, 3, vertexSize, sizeof(float) * 5);
    vao_.BindAttribute<float>(3, *vbo_, 3, vertexSize, sizeof(float) * 8);
    vao_.BindAttribute<float>(4, *vbo_, 3, vertexSize, sizeof(float) * 11);
    vao_.Unbind();
}

}
