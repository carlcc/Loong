//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#include "LoongEditorConstants.h"
#include <set>
#include <string>
#include <string_view>

namespace Loong::Editor::Constants {

const std::set<std::string_view> kMaterialFileSuffixes = { ".lgmtl" };
const std::set<std::string_view> kModelFileSuffixes = { ".lgmdl" };
const std::set<std::string_view> kShaderFileSuffixes = { ".glsl", ".shader" };
const std::set<std::string_view> kTextureFileSuffixes = { ".jpg", ".tga", ".png", ".bmp" };

}