#include "../GetNativeWindow.h"
#define GLFW_EXPOSE_NATIVE_COCOA
#include <GLFW/glfw3native.h>
#if VULKAN_SUPPORTED
#import <QuartzCore/CAMetalLayer.h>
#else
#error vulkan is the only supported backend on macos
#endif

namespace Loong::RHI {

Diligent::NativeWindow GetNativeWindow(GLFWwindow* window)
{
    Diligent::NativeWindow nativeWindow {};
    id cocoaWindow = glfwGetCocoaWindow(window);
    auto* nsWindow = (NSWindow*)cocoaWindow;
    nativeWindow.pNSView = nsWindow.contentView;

    // Create metal surface for vulkan
    NSBundle* bundle = [NSBundle bundleWithPath:@"/System/Library/Frameworks/QuartzCore.framework"];
    if (!bundle) {
        abort();
    }
    nsWindow.contentView.layer = [[bundle classNamed:@"CAMetalLayer"] layer];
    return nativeWindow;
}

}