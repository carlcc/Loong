#pragma once
#include <GLFW/glfw3.h>
#include <NativeWindow.h>

namespace Loong::RHI {

Diligent::NativeWindow GetNativeWindow(GLFWwindow* window);

}