#include "../GetNativeWindow.h"
#define GLFW_EXPOSE_NATIVE_WAYLAND
#define GLFW_EXPOSE_NATIVE_X11
#include <GLFW/glfw3native.h>
#include <NativeWindow.h>

Diligent::NativeWindow GetNativeWindow(GLFWwindow* window)
{
    Diligent::NativeWindow nativeWindow {};
    nativeWindow.WindowId = glfwGetX11Window(window);
    nativeWindow.pDisplay = glfwGetX11Display();

    return nativeWindow;
}
