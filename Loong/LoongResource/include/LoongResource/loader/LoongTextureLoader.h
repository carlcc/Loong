//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//
#pragma once

#include <functional>
#include <memory>
#include <string>

namespace Loong::Asset {

class LoongImage;

}

namespace Loong::Resource {

class LoongTexture;

class LoongTextureLoader {
public:
    LoongTextureLoader() = delete;

    static std::shared_ptr<LoongTexture> Create(const Asset::LoongImage& image, bool generateMipmap, const std::function<void(const std::string&)>& onDestroy);

    static std::shared_ptr<LoongTexture> CreateColor(uint8_t data[4], bool generateMipmap, const std::function<void(const std::string&)>& onDestroy);

    static std::shared_ptr<LoongTexture> CreateFromMemory(uint8_t* data, uint32_t width, uint32_t height, bool generateMipmap, const std::function<void(const std::string&)>& onDestroy, int channelCount = 4);
};

} // namespace Loong