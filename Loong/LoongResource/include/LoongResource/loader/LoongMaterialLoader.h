//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#pragma once

#include "LoongFoundation/LoongMacros.h"
#include <functional>
#include <memory>
#include <string>

namespace Loong::Foundation {
class LoongThreadTask;
}

namespace Loong::Resource {

class LoongMaterial;

class LoongMaterialLoader {
public:
    LoongMaterialLoader() = delete;

    static std::shared_ptr<Foundation::LoongThreadTask> CreateAsync(const std::string& filePath, const std::function<void(const std::string&)>& onDestroy, std::function<void(std::shared_ptr<LoongMaterial>)>&& onMaterial);

    static bool Write(const std::string& filePath, const LoongMaterial* material);
};

}