//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#include "LoongWindow/Driver.h"
#include <GLFW/glfw3.h>

namespace Loong::Window {

bool Initialize()
{
    return glfwInit() != 0;
}

void Uninitialize()
{
    glfwTerminate();
}

}