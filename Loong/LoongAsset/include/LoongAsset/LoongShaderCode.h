//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#pragma once

#include <string>

namespace Loong::Asset {

class LoongShaderCode {
public:
    explicit LoongShaderCode(const std::string& path);
    LoongShaderCode(const LoongShaderCode&) = delete;
    LoongShaderCode(LoongShaderCode&& i) noexcept
    {
        *this = std::move(i);
    }

    ~LoongShaderCode();

    LoongShaderCode& operator=(const LoongShaderCode&) = delete;
    LoongShaderCode& operator=(LoongShaderCode&& i) noexcept
    {
        std::swap(vertexShader_, i.vertexShader_);
        std::swap(fragmentShader_, i.fragmentShader_);
        std::swap(geometryShader_, i.geometryShader_);
        return *this;
    }

    const std::string& GetVertexShader() const { return vertexShader_; }
    const std::string& GetFragmentShader() const { return fragmentShader_; }
    const std::string& GetGeometryShader() const { return geometryShader_; }

    bool operator!() const { return !isValid_; }

    explicit operator bool() const { return isValid_; }

private:
    std::string vertexShader_ {};
    std::string fragmentShader_ {};
    std::string geometryShader_ {};
    bool isValid_ { false };
};

}