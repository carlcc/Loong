//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#include "LoongAsset/LoongModel.h"
#include "LoongAsset/LoongMesh.h"
#include "fbs/LoongModel_generated.h"
#include "LoongFileSystem/LoongFileSystem.h"
#include "LoongFoundation/LoongLogger.h"
#include <algorithm>
#include <cstring>
#include <fstream>

namespace Loong::Asset {

LoongModel::~LoongModel()
{
    Clear();
}

static constexpr const char* kLoongModelFileMagicAndVersion = "LGMDL\1";
static constexpr uint32_t kLoongModelFileMagicAndVersionLen = 6;

inline Math::Vector3 ToVector3(const Fbs::Vec3& v) { return { v.x(), v.y(), v.z() }; }
inline Math::Vector2 ToVector2(const Fbs::Vec2& v) { return { v.x(), v.y() }; }
inline Fbs::Vec3 ToVec3(const Math::Vector3& v) { return { v.x, v.y, v.z }; }
inline Fbs::Vec2 ToVec2(const Math::Vector2& v) { return { v.x, v.y }; }

bool LoongModel::LoadFromFile(const std::string& path)
{
    int64_t fileSize = FS::LoongFileSystem::GetFileSize(path);
    if (fileSize <= 0) {
        LOONG_ERROR("Failed to load model '{}': Wrong file size", path);
        return false;
    }
    std::vector<uint8_t> buffer(fileSize);
    auto size = FS::LoongFileSystem::LoadFileContent(path, buffer.data(), fileSize);
    (void)size;
    if (size != fileSize) {
        LOONG_ERROR("Load model '{}' to 0x{:0X} failed: Load file failed", path, intptr_t(this));
        return false;
    }

    if (memcmp(kLoongModelFileMagicAndVersion, buffer.data(), kLoongModelFileMagicAndVersionLen) != 0) {
        LOONG_ERROR("Load model '{}' to 0x{:0X} failed: Check magic number failed", path, intptr_t(this));
        return false;
    }

    auto* model = Fbs::GetLoongModel(buffer.data() + kLoongModelFileMagicAndVersionLen);
    if (model == nullptr) {
        LOONG_ERROR("Load model '{}' to 0x{:0X} failed: Parse buffer failed", path, intptr_t(this));
        return false;
    }

    for (auto m : *model->Meshes()) {
        std::vector<LoongVertex> vertices;
        std::vector<uint32_t> indices;
        vertices.resize(m->Vertices()->size());
        indices.resize(m->Indices()->size());

        // NOTE: For endian issue, don't use memcpy
        for (size_t i = 0; i < vertices.size(); ++i) {
            vertices[i].position = ToVector3(m->Vertices()->Get(i)->Position());
            vertices[i].normal = ToVector3(m->Vertices()->Get(i)->Normal());
            vertices[i].uv0 = ToVector2(m->Vertices()->Get(i)->Uv0());
            vertices[i].uv1 = ToVector2(m->Vertices()->Get(i)->Uv1());
            vertices[i].tangent = ToVector3(m->Vertices()->Get(i)->Tangent());
            vertices[i].bitangent = ToVector3(m->Vertices()->Get(i)->Bitangent());
        }
        for (size_t i = 0; i < indices.size(); ++i) {
            indices[i] = m->Indices()->Get(i);
        }
        meshes_.push_back(new LoongMesh(std::move(vertices), std::move(indices), m->MaterialIndex()));
    }

    for (auto n : *model->MaterialNames()) {
        materialNames_.push_back(n->str());
    }
    aabb_.max = ToVector3(model->Aabb()->Max());
    aabb_.min = ToVector3(model->Aabb()->Min());

    LOONG_TRACE("Load model '{}' to 0x{:0X} succeed", path, intptr_t(this));
    return true;
}

bool LoongModel::WriteToFilePhysical(const std::string& physicalPath) const
{
    flatbuffers::FlatBufferBuilder fbb;
    WriteToBuffer(&fbb, physicalPath);

    auto* buffer = fbb.GetBufferPointer();
    auto bufferSize = (int64_t)fbb.GetSize();

    std::ofstream ofs(physicalPath, std::ios::out | std::ios::binary);
    if (!ofs) {
        LOONG_ERROR("Store model '{}' failed: Open file failed", physicalPath);
        return false;
    }
    ofs.write(kLoongModelFileMagicAndVersion, kLoongModelFileMagicAndVersionLen);
    ofs.write((char*)buffer, bufferSize);
    if (!ofs.good()) {
        LOONG_ERROR("Store model '{}' failed: Write file failed", physicalPath);
        return false;
    }

    return true;
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

void LoongModel::Clear()
{
    if (!meshes_.empty()) {
        LOONG_TRACE("Unload model '0x{:0X}'", intptr_t(this));
        for (auto* mesh : meshes_) {
            delete mesh;
        }
        meshes_.clear();
    }
    materialNames_.clear();
}

bool LoongModel::WriteToBuffer(void* flatbuffer, const std::string& path) const
{
    if (meshes_.empty()) {
        LOONG_WARNING("You are writing an empty model to '{}'", path);
    }

    Fbs::LoongModelT modelT;
    // Mesh
    for (auto* mesh : meshes_) {
        auto pMeshT = std::make_unique<Fbs::LoongMeshT>();

        auto& meshT = *pMeshT;
        auto& vertices = mesh->GetVertices();
        auto& indices = mesh->GetIndices();
        meshT.Vertices.resize(mesh->GetVertices().size());
        meshT.Indices.resize(indices.size());

        // For endian issue, don't use memcpy
        for (size_t i = 0; i < vertices.size(); ++i) {
            meshT.Vertices[i].mutable_Position() = ToVec3(vertices[i].position);
            meshT.Vertices[i].mutable_Normal() = ToVec3(vertices[i].normal);
            meshT.Vertices[i].mutable_Uv0() = ToVec2(vertices[i].uv0);
            meshT.Vertices[i].mutable_Uv1() = ToVec2(vertices[i].uv1);
            meshT.Vertices[i].mutable_Tangent() = ToVec3(vertices[i].tangent);
            meshT.Vertices[i].mutable_Bitangent() = ToVec3(vertices[i].bitangent);
        }
        for (size_t i = 0; i < indices.size(); ++i) {
            meshT.Indices[i] = indices[i];
        }
        meshT.Aabb = std::make_unique<Fbs::AABB>();
        meshT.Aabb->mutable_Max() = ToVec3(mesh->GetAABB().max);
        meshT.Aabb->mutable_Min() = ToVec3(mesh->GetAABB().min);

        meshT.MaterialIndex = mesh->GetMaterialIndex();

        modelT.Meshes.push_back(std::move(pMeshT));
    }
    // material names
    for (auto& n : materialNames_) {
        modelT.MaterialNames.push_back(n);
    }
    modelT.Aabb = std::make_unique<Fbs::AABB>();
    modelT.Aabb->mutable_Min() = ToVec3(aabb_.min);
    modelT.Aabb->mutable_Max() = ToVec3(aabb_.max);

    auto& fbb = *reinterpret_cast<flatbuffers::FlatBufferBuilder*>(flatbuffer);
    fbb.Finish(Fbs::LoongModel::Pack(fbb, &modelT));

    return true;
}

}
