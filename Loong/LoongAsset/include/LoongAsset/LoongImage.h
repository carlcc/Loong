//
// Copyright (c) carlcc. All rights reserved.
//

#pragma once

#include "LoongFoundation/LoongMacros.h"
#include <string>

namespace Loong::Asset {

class LoongImage {
public:
    LoongImage() = default;
    explicit LoongImage(const std::string& path);
    LoongImage(const LoongImage&) = delete;
    LoongImage(LoongImage&& i) noexcept
    {
        buffer_ = i.buffer_;
        width_ = i.width_;
        height_ = i.height_;
        channelCount_ = i.channelCount_;
        path_ = i.path_;

        i.buffer_ = nullptr;
        i.width_ = 0;
        i.height_ = 0;
        i.channelCount_ = 0;
        i.path_ = "";
    }

    ~LoongImage();

    bool Load(const std::string& path);

    bool LoadFromPhysicalPath(const std::string& path);

    LoongImage& operator=(const LoongImage&) = delete;
    LoongImage& operator=(LoongImage&& i) noexcept
    {
        std::swap(buffer_, i.buffer_);
        std::swap(width_, i.width_);
        std::swap(height_, i.height_);
        std::swap(channelCount_, i.channelCount_);
        std::swap(path_, i.path_);
        return *this;
    }

    LG_NODISCARD int GetChannelCount() const { return channelCount_; }

    LG_NODISCARD int GetWidth() const { return width_; }

    LG_NODISCARD int GetHeight() const { return height_; }

    LG_NODISCARD char* GetData() { return buffer_; }

    LG_NODISCARD const char* GetData() const { return buffer_; }

    LG_NODISCARD const std::string& GetPath() const { return path_; }

    void FlipVertically();

    bool operator!() const { return buffer_ == nullptr; }

    explicit operator bool() const { return buffer_ != nullptr; }

private:
    void Clear();

private:
    char* buffer_ { nullptr };
    int width_ { 0 };
    int height_ { 0 };
    int channelCount_ { 0 };
    std::string path_ {};
};

}
