//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//
#pragma once

#include "LoongRHI/LoongRHIManager.h"
#include <functional>
#include <memory>
#include <string>

namespace Loong::Asset {

class LoongImage;

}

namespace Loong::Resource {

class LoongTextureLoader {
public:
    LoongTextureLoader() = delete;

    static RHI::RefCntAutoPtr<RHI::ITexture> Create(const std::string& vfsPath, RHI::RefCntAutoPtr<RHI::IRenderDevice> device, bool isSrgb = true);

    static RHI::RefCntAutoPtr<RHI::ITexture> CreateColor(uint8_t data[4], bool generateMipmap, const std::function<void(const std::string&)>& onDestroy);

    static RHI::RefCntAutoPtr<RHI::ITexture> CreateFromMemory(uint8_t* data, uint32_t width, uint32_t height, bool generateMipmap, const std::function<void(const std::string&)>& onDestroy, int channelCount = 4);
};

} // namespace Loong