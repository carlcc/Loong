//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#include "LoongResource/loader/LoongMaterialLoader.h"
#include "LoongFileSystem/LoongFileSystem.h"
#include "LoongFoundation/LoongMath.h"
#include "LoongResource/LoongMaterial.h"
#include "LoongResource/LoongResourceManager.h"
#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/ostreamwrapper.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/rapidjson.h>
#include <sstream>
#include <string>
#include <string_view>

namespace Loong::Resource {

using TextureRef = std::shared_ptr<LoongTexture>;

inline Math::Vector4 ParseStringToVector4(const std::string& string)
{
    std::stringstream ss;
    ss << string;
    Math::Vector4 vec { 0.F };
    ss >> vec.x;
    ss >> vec.y;
    ss >> vec.z;
    ss >> vec.w;
    return vec;
}
inline std::string Vec4ToString(const Math::Vector4& vec)
{
    std::stringstream ss;
    ss << vec.x << " " << vec.y << " " << vec.z << " " << vec.w;
    return ss.str();
}
inline Math::Vector3 ParseStringToVector3(const std::string& string)
{
    std::stringstream ss;
    ss << string;
    Math::Vector3 vec { 0.F };
    ss >> vec.x;
    ss >> vec.y;
    ss >> vec.z;
    return vec;
}
inline std::string Vec3ToString(const Math::Vector3& vec)
{
    std::stringstream ss;
    ss << vec.x << " " << vec.y << " " << vec.z;
    return ss.str();
}
inline Math::Vector2 ParseStringToVector2(const std::string& string)
{
    std::stringstream ss;
    ss << string;
    Math::Vector2 vec { 0.F };
    ss >> vec.x;
    ss >> vec.y;
    return vec;
}
inline std::string Vec2ToString(const Math::Vector2& vec)
{
    std::stringstream ss;
    ss << vec.x << " " << vec.y;
    return ss.str();
}
inline int ParseStringToInt(const std::string& string)
{
    std::stringstream ss;
    ss << string;
    int v { 0 };
    ss >> v;
    return v;
}
inline float ParseStringToFloat(const std::string& string)
{
    std::stringstream ss;
    ss << string;
    float v { 0.0F };
    ss >> v;
    return v;
}

std::shared_ptr<LoongMaterial> LoongMaterialLoader::Create(const std::string& filePath, const std::function<void(const std::string&)>& onDestroy)
{
    return nullptr;
}

bool LoongMaterialLoader::Write(const std::string& filePath, const LoongMaterial* material)
{
    return true;
}

}
