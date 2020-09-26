//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#include "LoongAsset/LoongModel.h"
#include "LoongAsset/LoongMesh.h"
#include "LoongFileSystem/LoongFileSystem.h"
#include "LoongFoundation/LoongLogger.h"
#include "LoongFoundation/LoongSerializer.h"
#include "LoongFoundation/LoongStringUtils.h"
#include <algorithm>

namespace Loong::Asset {

struct MemoryInputStream : public Foundation::LoongArchiveInputStream {
    explicit MemoryInputStream(uint8_t* buffer, size_t size)
        : buffer_(buffer)
        , size_(size)
    {
    }
    bool operator()(void* d, size_t l)
    {
        if (l <= size_) {
            memcpy(d, buffer_, l);
            buffer_ += l;
            size_ -= l;
            return true;
        }
        return false;
    }

private:
    uint8_t* buffer_ { nullptr };
    size_t size_ { 0 };
};

LoongModel::LoongModel(const std::string& path)
{
    int64_t fileSize = FS::LoongFileSystem::GetFileSize(path);
    if (fileSize <= 0) {
        LOONG_ERROR("Failed to load model '{}': Wrong file size", path);
        return;
    }
    std::vector<uint8_t> buffer(fileSize);
    auto size = FS::LoongFileSystem::LoadFileContent(path, buffer.data(), fileSize);
    (void)size;
    assert(size == fileSize);

    MemoryInputStream inputStream(buffer.data(), buffer.size());

    if (Foundation::Serialize(*this, inputStream)) {
        LOONG_TRACE("Load model '{}' to 0x{:0X} succeed", path, intptr_t(this));
    } else {
        Clear();
        LOONG_ERROR("Load model '{}' to 0x{:0X} failed", path, intptr_t(this));
    }
}

LoongModel::~LoongModel()
{
    LOONG_TRACE("Unload model '0x{:0X}'", intptr_t(this));
    Clear();
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
    for (auto* mesh : meshes_) {
        delete mesh;
    }
    meshes_.clear();
    materialNames_.clear();
}

}
