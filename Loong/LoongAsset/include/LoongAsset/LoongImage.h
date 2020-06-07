//
// Copyright (c) carlcc. All rights reserved.
//

#pragma once

#include <string>

namespace Loong::Asset {

class LoongImage {
public:
    explicit LoongImage(const std::string& path);

    ~LoongImage();

    int GetChannelCount() const { return channelCount_; }

    int GetWidth() const { return width_; }

    int GetHeight() const { return height_; }

    char* GetData() { return buffer_; }

    const char* GetData() const { return buffer_; }

    void FlipVertically();

    bool operator!() const { return buffer_ == nullptr; }

    explicit operator bool() const { return buffer_ != nullptr; }

private:
    char* buffer_ { nullptr };
    int width_ { 0 };
    int height_ { 0 };
    int channelCount_ { 0 };
};

}
