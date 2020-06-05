//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#include "LoongApp/Driver.h"
#include <GLFW/glfw3.h>

namespace Loong::App {

bool Initialize()
{
    return glfwInit() != 0;
}

void Uninitialize()
{
    glfwTerminate();
}

}