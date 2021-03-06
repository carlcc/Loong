//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#pragma once

#include <functional>
#include <memory>
#include <string>

namespace Loong::Resource {

class LoongMaterial;

class LoongMaterialLoader {
public:
    LoongMaterialLoader() = delete;

    static std::shared_ptr<LoongMaterial> Create(const std::string& filePath, const std::function<void(const std::string&)>& onDestroy);

    static bool Write(const std::string& filePath, const LoongMaterial* material);
};

}