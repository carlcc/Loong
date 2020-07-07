//
// Copyright (c) carlcc. All rights reserved.
//

#include "LoongAsset/LoongImage.h"
#include "LoongFileSystem/LoongFileSystem.h"
#include "LoongFoundation/LoongLogger.h"

#ifdef _MSC_VER
#pragma warning(push)
// e.g. This function or variable may be unsafe. Consider using fopen_s instead.
#pragma warning(disable : 4996)
#endif

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#ifdef _MSC_VER
#pragma warning(pop)
#endif

namespace Loong::Asset {

LoongImage::LoongImage(const std::string& path)
{
    Load(path);
}

LoongImage::~LoongImage()
{
    Clear();
}

void LoongImage::FlipVertically()
{
    if (!*this) {
        return;
    }
    stbi__vertical_flip(buffer_, width_, height_, channelCount_);
}

bool LoongImage::Load(const std::string& path)
{
    int64_t fileSize = FS::LoongFileSystem::GetFileSize(path);
    if (fileSize <= 0) {
        LOONG_ERROR("Failed to load image '{}': Wrong file size", path);
        return false;
    }
    std::vector<uint8_t> buffer(fileSize);
    assert(FS::LoongFileSystem::LoadFileContent(path, buffer.data(), fileSize) == fileSize);

    LOONG_TRACE("Load image '{}' to '0x{:0X}'", path, intptr_t(this));
    buffer_ = reinterpret_cast<char*>(stbi_load_from_memory(buffer.data(), int(fileSize), &width_, &height_, &channelCount_, 0));
    if (buffer_ == nullptr) {
        LOONG_ERROR("Failed to load image '{}'", path);
        width_ = 0;
        height_ = 0;
        channelCount_ = 0;
    }
    path_ = path;
    return true;
}

bool LoongImage::LoadFromPhysicalPath(const std::string& path)
{
    LOONG_TRACE("Load image '{}' to '0x{:0X}'", path, intptr_t(this));
    buffer_ = reinterpret_cast<char*>(stbi_load(path.c_str(), &width_, &height_, &channelCount_, 0));
    if (buffer_ == nullptr) {
        LOONG_ERROR("Failed to load image '{}'", path);
        width_ = 0;
        height_ = 0;
        channelCount_ = 0;
    }
    path_ = path;
    return true;
}

void LoongImage::Clear()
{
    LOONG_TRACE("Release image '0x{:0X}'", intptr_t(this));
    if (buffer_ != nullptr) {
        delete[] buffer_;
        buffer_ = nullptr;
        width_ = 0;
        height_ = 0;
        channelCount_ = 0;
    }
}

}