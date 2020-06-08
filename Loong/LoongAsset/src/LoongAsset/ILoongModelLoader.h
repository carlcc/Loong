//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#pragma once

#include <string>
#include <vector>

namespace Loong::Asset {

class LoongMesh;

// TODO: Parse models with plugin?
class ILoongModelLoader {
public:
    virtual bool LoadModel(const std::string& fileName, std::vector<LoongMesh*>& meshes, std::vector<std::string>& materials) = 0;
};

}