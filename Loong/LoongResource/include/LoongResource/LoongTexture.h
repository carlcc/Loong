//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#pragma once

#include "LoongRHI/LoongRHIManager.h"
#include <string>

namespace Loong::Resource {

class LoongTexture {
public:
    RHI::RefCntAutoPtr<RHI::ITexture> GetTexture() const { return texture_; }

    const std::string& GetPath() const { return path_; }

private:
    LoongTexture(RHI::RefCntAutoPtr<RHI::ITexture> texture, const std::string& path);
    ~LoongTexture() = default;
    LoongTexture(const LoongTexture&) = delete;
    LoongTexture(LoongTexture&& t) noexcept { *this = std::move(t); }

    LoongTexture& operator=(const LoongTexture&) = delete;
    LoongTexture& operator=(LoongTexture&& t)
    {
        texture_ = std::move(t.texture_);
        path_ = std::move(t.path_);
        return *this;
    }

private:
    RHI::RefCntAutoPtr<RHI::ITexture> texture_ { nullptr };
    std::string path_ {};
    friend class LoongTextureLoader;
};

}