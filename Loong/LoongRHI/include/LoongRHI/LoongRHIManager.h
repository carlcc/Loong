//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//
#pragma once

// Diligent engine header files, are put here intentionally to avoid include them in other file
#include <DeviceContext.h>
#include <EngineFactory.h>
#include <Errors.hpp>
#include <GraphicsTypes.h>
#include <PlatformDefinitions.h>
#include <RefCntAutoPtr.hpp>
#include <RenderDevice.h>
#include <ScreenCapture.hpp>
#include <SwapChain.h>

struct GLFWwindow;

namespace Loong::RHI {

using namespace Diligent;

class LoongRHIManager {
public:
    static bool Initialize(GLFWwindow* glfwWindow, RENDER_DEVICE_TYPE deviceType);

    static void Uninitialize();

    static void Present(bool vsync);

    static RefCntAutoPtr<ISwapChain> GetSwapChain();

    static RefCntAutoPtr<IRenderDevice> GetDevice();

    static RefCntAutoPtr<IDeviceContext> GetImmediateContext();

    //=== Create methods
    static RHI::RefCntAutoPtr<RHI::IPipelineState> CreateGraphicsPSOForCurrentSwapChain(
        const char* pipelineName,
        const RHI::ShaderCreateInfo& vs, const RHI::ShaderCreateInfo& ps, bool depthEnabled = true,
        CULL_MODE cullMode = CULL_MODE_BACK, PRIMITIVE_TOPOLOGY topology = PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
};

}
