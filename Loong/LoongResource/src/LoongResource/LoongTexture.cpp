//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#include "LoongResource/LoongTexture.h"

namespace Loong::Resource {

LoongTexture::LoongTexture(RHI::RefCntAutoPtr<RHI::ITexture> texture, const std::string& path)
    : texture_ { texture }
    , path_ { path }
{
}

}
