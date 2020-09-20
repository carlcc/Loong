//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//
#pragma once

#include <GraphicsTypes.h>
#include <NativeWindow.h>

namespace Loong::RHI {

using namespace Diligent;

class LoongRHIManager {
public:
    static bool Initialize(NativeWindow nativeWindow, RENDER_DEVICE_TYPE deviceType);

    static void Uninitialize();
};

}
