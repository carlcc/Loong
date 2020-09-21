//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//
#pragma once

#include <GraphicsTypes.h>

struct GLFWwindow;

namespace Loong::RHI {

using namespace Diligent;

class LoongRHIManager {
public:
    static bool Initialize(GLFWwindow* glfwWindow, RENDER_DEVICE_TYPE deviceType);

    static void Uninitialize();
};

}
