//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//
#pragma once

#include "LoongFoundation/LoongMath.h"
#include <assimp/matrix4x4.h>

namespace Loong::AssetConverter {

inline Math::Matrix4 AiMatrix2LoongMatrix(const aiMatrix4x4& matrix)
{
    // aiMatrix4x4 is row major, while our Matrix4 is column major
    return Math::Matrix4(
        matrix.a1, matrix.b1, matrix.c1, matrix.d1,
        matrix.a2, matrix.b2, matrix.c2, matrix.d2,
        matrix.a3, matrix.b3, matrix.c3, matrix.d3,
        matrix.a4, matrix.b4, matrix.c4, matrix.d4);
}

}