#include "../GetNativeWindow.h"
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include <NativeWindow.h>

Diligent::NativeWindow GetNativeWindow(GLFWwindow* window)
{
    NativeWindow nativeWindow {};
    nativeWindow.hWnd = glfwGetWin32Window(window);
    return nativeWindow;
}