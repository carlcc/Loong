#include "../GetNativeWindow.h"
#define GLFW_EXPOSE_NATIVE_WAYLAND
#define GLFW_EXPOSE_NATIVE_X11
#include <GLFW/glfw3native.h>
#include <NativeWindow.h>
// #include <X11/Xlib-xcb.h>

namespace Loong::RHI {

Diligent::NativeWindow GetNativeWindow(GLFWwindow* window)
{
    Diligent::NativeWindow nativeWindow {};
    nativeWindow.WindowId = glfwGetX11Window(window);
    nativeWindow.pDisplay = glfwGetX11Display();
    // nativeWindow.pXCBConnection = XGetXCBConnection(glfwGetX11Display());

    return nativeWindow;
}

}