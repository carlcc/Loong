//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#pragma once

#include <set>
#include <string>
#include <string_view>

namespace Loong::Editor::Constants {

extern const std::set<std::string_view> kMaterialFileSuffixes;
extern const std::set<std::string_view> kModelFileSuffixes;
extern const std::set<std::string_view> kShaderFileSuffixes;
extern const std::set<std::string_view> kTextureFileSuffixes;
constexpr const char* kCubeModelPath = "/Models/cube.lgmdl";
constexpr const char* kConeModelPath = "/Models/cone.lgmdl";
constexpr const char* kCylinderModelPath = "/Models/cylinder.lgmdl";
constexpr const char* kPipeModelPath = "/Models/pipe.lgmdl";
constexpr const char* kPlaneModelPath = "/Models/plane.lgmdl";
constexpr const char* kPyramidModelPath = "/Models/pyramid.lgmdl";
constexpr const char* kSphereModelPath = "/Models/sphere.lgmdl";

}