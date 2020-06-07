//
// Copyright (c) carlcc. All rights reserved.
//

#pragma once

#include <string>

namespace Loong::Asset {

class LoongImage {
public:
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

    int GetChannelCount() const { return channelCount_; }

    int GetWidth() const { return width_; }

    int GetHeight() const { return height_; }

    char* GetData() { return buffer_; }

    const char* GetData() const { return buffer_; }

    const std::string& GetPath() const { return path_; }

    void FlipVertically();

    bool operator!() const { return buffer_ == nullptr; }

    explicit operator bool() const { return buffer_ != nullptr; }

private:
    char* buffer_ { nullptr };
    int width_ { 0 };
    int height_ { 0 };
    int channelCount_ { 0 };
    std::string path_ {};
};

}
