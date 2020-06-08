//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#include "LoongAsset/LoongShaderCode.h"
#include "LoongFileSystem/LoongFileSystem.h"
#include "LoongFoundation/LoongLogger.h"
#include <sstream>

namespace Loong::Asset {

LoongShaderCode::LoongShaderCode(const std::string& path)
{
    int64_t fileSize = FS::LoongFileSystem::GetFileSize(path);
    if (fileSize <= 0) {
        LOONG_ERROR("Failed to load image '{}': Wrong file size", path);
        return;
    }
    std::vector<char> buffer(fileSize);
    assert(FS::LoongFileSystem::LoadFileContent(path, buffer.data(), fileSize) == fileSize);

    LOONG_TRACE("Load shader '{}' to '0x{:0X}'", path, intptr_t(this));
    std::stringstream ss;
    ss << std::string_view(buffer.data(), buffer.size());
    enum ShaderType {
        NONE = -1,
        VERTEX = 0,
        FRAGMENT = 1,
        GEOMETRY = 2,
        COUNT,
    };

    std::string line;
    std::stringstream shaders[ShaderType::COUNT];
    ShaderType type = ShaderType::NONE;

    auto IsEmptyLine = [](const std::string& s) -> bool {
        return std::all_of(s.begin(), s.end(), [](char c) { return std::isspace(c); });
    };
    while (std::getline(ss, line)) {
        if (IsEmptyLine(line)) {
            continue;
        }
        if (line.find("#shader") != std::string::npos) {
            if (line.find("vertex") != std::string::npos)
                type = ShaderType::VERTEX;
            else if (line.find("fragment") != std::string::npos)
                type = ShaderType::FRAGMENT;
            else if (line.find("geometry") != std::string::npos)
                type = ShaderType::GEOMETRY;
            else {
                LOONG_ERROR("Failed to load shader file '{}': Unknown shader type declare '{}'", path, line);
                return;
            }
        } else if (type != ShaderType::NONE) {
            shaders[type] << line << '\n';
        }
    }

    vertexShader_ = shaders[ShaderType::VERTEX].str();
    fragmentShader_ = shaders[ShaderType::FRAGMENT].str();
    geometryShader_ = shaders[ShaderType::GEOMETRY].str();
    isValid_ = true;
}

LoongShaderCode::~LoongShaderCode()
{
    LOONG_TRACE("Unload shader '0x{:0X}'", intptr_t(this));
}

}
