//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//
#pragma once

// Diligent engine header files, are put here intentionally to avoid include them in other file
#include <BasicMath.hpp>
#include <DeviceContext.h>
#include <EngineFactory.h>
#include <Errors.hpp>
#include <GraphicsTypes.h>
#include <MapHelper.hpp>
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

    static RefCntAutoPtr<ISwapChain> GetPrimarySwapChain();

    static RefCntAutoPtr<IRenderDevice> GetDevice();

    static RefCntAutoPtr<IDeviceContext> GetImmediateContext();

    //=== Create methods

    static RefCntAutoPtr<ISwapChain> CreateSwapChain(GLFWwindow* glfwWindow);

    static RHI::RefCntAutoPtr<RHI::IPipelineState> CreateGraphicsPSOForCurrentSwapChain(ISwapChain* swapChain,
        const char* pipelineName, const RHI::ShaderCreateInfo& vs, const RHI::ShaderCreateInfo& ps,
        InputLayoutDesc inputLayout, PipelineResourceLayoutDesc resourceLayout = {}, bool depthEnabled = true,
        CULL_MODE cullMode = CULL_MODE_BACK, PRIMITIVE_TOPOLOGY topology = PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);

    static RefCntAutoPtr<IBuffer> CreateUniformBuffer(const char* bufferName, uint32_t size, const void* initialData = nullptr,
        USAGE usage = USAGE_DYNAMIC, BIND_FLAGS bindFlags = BIND_UNIFORM_BUFFER, CPU_ACCESS_FLAGS cpuAccessFlags = CPU_ACCESS_WRITE);

    static RefCntAutoPtr<IBuffer> CreateVertexBuffer(const char* bufferName, uint32_t size, const void* initialData = nullptr,
        USAGE usage = USAGE_STATIC, BIND_FLAGS bindFlags = BIND_VERTEX_BUFFER);

    static RefCntAutoPtr<IBuffer> CreateIndexBuffer(const char* bufferName, uint32_t size, const void* initialData = nullptr,
        USAGE usage = USAGE_STATIC, BIND_FLAGS bindFlags = BIND_INDEX_BUFFER)
    {
        return CreateVertexBuffer(bufferName, size, initialData, usage, bindFlags);
    }

    static RefCntAutoPtr<ITexture> CreateTextureFromFile(const char* file, bool isSrgb);
};

}
