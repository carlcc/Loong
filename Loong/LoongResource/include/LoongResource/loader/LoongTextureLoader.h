//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//
#pragma once

#include "LoongFoundation/LoongMacros.h"
#include "LoongRHI/LoongRHIManager.h"
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

    LG_NODISCARD static std::shared_ptr<LoongTexture> Create(const std::string& vfsPath, RHI::RefCntAutoPtr<RHI::IRenderDevice> device, bool isSrgb = true, std::function<void(const std::string&)>&& onDestroy = nullptr);

    LG_NODISCARD static std::shared_ptr<LoongTexture> CreateColor(uint8_t data[4], bool generateMipmap, const std::function<void(const std::string&)>& onDestroy);

    LG_NODISCARD static std::shared_ptr<LoongTexture> CreateFromMemory(uint8_t* data, uint32_t width, uint32_t height, bool generateMipmap, const std::function<void(const std::string&)>& onDestroy, int channelCount = 4);
};

} // namespace Loong