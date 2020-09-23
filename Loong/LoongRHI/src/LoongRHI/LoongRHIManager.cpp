//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//
#include "LoongRHI/LoongRHIManager.h"
#include "GetNativeWindow.h"
#include "LoongFoundation/LoongLogger.h"
#include <GraphicsUtilities.h>
#include <TextureUtilities.h>
#if D3D11_SUPPORTED
#include <EngineFactoryD3D11.h>
#endif
#if D3D12_SUPPORTED
#include <EngineFactoryD3D12.h>
#endif
#if VULKAN_SUPPORTED
#include <EngineFactoryVk.h>
#endif
#if METAL_SUPPORTED
#include <EngineFactoryMtl.h>
#endif

namespace Loong::RHI {

struct LoongRHIImpl {

    static void GetEngineInitializationAttribs(RENDER_DEVICE_TYPE deviceType, EngineCreateInfo& engineCI)
    {
        switch (deviceType) {
#if D3D11_SUPPORTED
        case RENDER_DEVICE_TYPE_D3D11: {
            //EngineD3D11CreateInfo& EngineD3D11CI = static_cast<EngineD3D11CreateInfo&>(engineCI);
        } break;
#endif

#if D3D12_SUPPORTED
        case RENDER_DEVICE_TYPE_D3D12: {
            EngineD3D12CreateInfo& EngineD3D12CI = static_cast<EngineD3D12CreateInfo&>(engineCI);
            EngineD3D12CI.GPUDescriptorHeapDynamicSize[0] = 32768;
            EngineD3D12CI.GPUDescriptorHeapSize[1] = 128;
            EngineD3D12CI.GPUDescriptorHeapDynamicSize[1] = 2048 - 128;
            EngineD3D12CI.DynamicDescriptorAllocationChunkSize[0] = 32;
            EngineD3D12CI.DynamicDescriptorAllocationChunkSize[1] = 8; // D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER
        } break;
#endif

#if VULKAN_SUPPORTED
        case RENDER_DEVICE_TYPE_VULKAN: {
            // EngineVkCreateInfo& EngVkAttribs = static_cast<EngineVkCreateInfo&>(engineCI);
        } break;
#endif

#if METAL_SUPPORTED
        case RENDER_DEVICE_TYPE_METAL: {
            // Nothing to do
        } break;
#endif

        default:
            LOONG_ERROR("Unknown device type");
            abort();
        }
    }

    RefCntAutoPtr<ISwapChain> CreateSwapChain(NativeWindow nativeWindow)
    {
#if PLATFORM_MACOS
        // We need at least 3 buffers on Metal to avoid massive
        // peformance degradation in full screen mode.
        // https://github.com/KhronosGroup/MoltenVK/issues/808
        swapChainInitDesc_.BufferCount = 3;
#endif
        RefCntAutoPtr<ISwapChain> swapChain {};

        switch (deviceType_) {
#if D3D11_SUPPORTED
        case RENDER_DEVICE_TYPE_D3D11: {
            auto* pFactoryD3D11 = GetEngineFactoryD3D11();
            pFactoryD3D11->CreateSwapChainD3D11(device_, immediateContext_, swapChainInitDesc_, FullScreenModeDesc {}, nativeWindow, &swapChain);
        } break;
#endif

#if D3D12_SUPPORTED
        case RENDER_DEVICE_TYPE_D3D12: {
            auto* pFactoryD3D12 = GetEngineFactoryD3D12();
            pFactoryD3D12->CreateSwapChainD3D12(device_, immediateContext_, swapChainInitDesc_, FullScreenModeDesc {}, nativeWindow, &swapChain);
        } break;
#endif

#if VULKAN_SUPPORTED
        case RENDER_DEVICE_TYPE_VULKAN: {
            auto* pFactoryVk = GetEngineFactoryVk();
            pFactoryVk->CreateSwapChainVk(device_, immediateContext_, swapChainInitDesc_, nativeWindow, &swapChain);
        } break;
#endif

#if METAL_SUPPORTED
        case RENDER_DEVICE_TYPE_METAL: {
            auto* pFactoryMtl = GetEngineFactoryMtl();
            pFactoryMtl->CreateSwapChainMtl(device_, immediateContext_, swapChainInitDesc_, nativeWindow, &swapChain);
        } break;
#endif

        default:
            LOG_ERROR_AND_THROW("Unknown device type");
            break;
        }
        return swapChain;
    }

    bool Initialize(NativeWindow nativeWindow, RENDER_DEVICE_TYPE deviceType)
    {
        deviceType_ = deviceType;

        std::vector<IDeviceContext*> ppContexts;
        switch (deviceType_) {
#if D3D11_SUPPORTED
        case RENDER_DEVICE_TYPE_D3D11: {
            EngineD3D11CreateInfo EngineCI;

#ifdef DILIGENT_DEVELOPMENT
            EngineCI.DebugFlags |= D3D11_DEBUG_FLAG_CREATE_DEBUG_DEVICE | D3D11_DEBUG_FLAG_VERIFY_COMMITTED_SHADER_RESOURCES;
#endif
#ifdef DILIGENT_DEBUG
            EngineCI.DebugFlags |= D3D11_DEBUG_FLAG_VERIFY_COMMITTED_RESOURCE_RELEVANCE;
#endif

            if (validationLevel_ >= 1) {
                EngineCI.DebugFlags = D3D11_DEBUG_FLAG_CREATE_DEBUG_DEVICE | D3D11_DEBUG_FLAG_VERIFY_COMMITTED_SHADER_RESOURCES | D3D11_DEBUG_FLAG_VERIFY_COMMITTED_RESOURCE_RELEVANCE;
            } else if (validationLevel_ == 0) {
                EngineCI.DebugFlags = D3D11_DEBUG_FLAG_NONE;
            }

            GetEngineInitializationAttribs(deviceType_, EngineCI);

#if ENGINE_DLL
            // Load the dll and import GetEngineFactoryD3D11() function
            auto GetEngineFactoryD3D11 = LoadGraphicsEngineD3D11();
#endif
            auto* pFactoryD3D11 = GetEngineFactoryD3D11();
            engineFactory_ = pFactoryD3D11;
            Uint32 NumAdapters = 0;
            pFactoryD3D11->EnumerateAdapters(EngineCI.MinimumFeatureLevel, NumAdapters, 0);
            std::vector<GraphicsAdapterInfo> Adapters(NumAdapters);
            if (NumAdapters > 0) {
                pFactoryD3D11->EnumerateAdapters(EngineCI.MinimumFeatureLevel, NumAdapters, Adapters.data());
            } else {
                LOG_ERROR_AND_THROW("Failed to find Direct3D11-compatible hardware adapters");
            }

            if (adapterType_ == ADAPTER_TYPE_SOFTWARE) {
                for (Uint32 i = 0; i < Adapters.size(); ++i) {
                    if (Adapters[i].Type == adapterType_) {
                        adapterId_ = i;
                        LOG_INFO_MESSAGE("Found software adapter '", Adapters[i].Description, "'");
                        break;
                    }
                }
            }

            adapterAttribs_ = Adapters[adapterId_];
            if (adapterType_ != ADAPTER_TYPE_SOFTWARE) {
                Uint32 NumDisplayModes = 0;
                pFactoryD3D11->EnumerateDisplayModes(EngineCI.MinimumFeatureLevel, adapterId_, 0, TEX_FORMAT_RGBA8_UNORM_SRGB, NumDisplayModes, nullptr);
                displayModes_.resize(NumDisplayModes);
                pFactoryD3D11->EnumerateDisplayModes(EngineCI.MinimumFeatureLevel, adapterId_, 0, TEX_FORMAT_RGBA8_UNORM_SRGB, NumDisplayModes, displayModes_.data());
            }

            EngineCI.AdapterId = adapterId_;
            ppContexts.resize(1 + EngineCI.NumDeferredContexts);
            pFactoryD3D11->CreateDeviceAndContextsD3D11(EngineCI, &device_, ppContexts.data());
            if (!device_) {
                LOG_ERROR_AND_THROW("Failed to create Direct3D11 render device and contexts.");
            }
        } break;
#endif

#if D3D12_SUPPORTED
        case RENDER_DEVICE_TYPE_D3D12: {
            EngineD3D12CreateInfo EngineCI;

#ifdef DILIGENT_DEVELOPMENT
            EngineCI.EnableDebugLayer = true;
#endif
            if (validationLevel_ >= 1) {
                EngineCI.EnableDebugLayer = true;
                if (validationLevel_ >= 2)
                    EngineCI.EnableGPUBasedValidation = true;
            } else if (validationLevel_ == 0) {
                EngineCI.EnableDebugLayer = false;
            }

            GetEngineInitializationAttribs(deviceType_, EngineCI);

#if ENGINE_DLL
            // Load the dll and import GetEngineFactoryD3D12() function
            auto GetEngineFactoryD3D12 = LoadGraphicsEngineD3D12();
#endif
            auto* pFactoryD3D12 = GetEngineFactoryD3D12();
            if (!pFactoryD3D12->LoadD3D12()) {
                LOG_ERROR_AND_THROW("Failed to load Direct3D12");
            }

            engineFactory_ = pFactoryD3D12;
            Uint32 NumAdapters = 0;
            pFactoryD3D12->EnumerateAdapters(EngineCI.MinimumFeatureLevel, NumAdapters, 0);
            std::vector<GraphicsAdapterInfo> Adapters(NumAdapters);
            if (NumAdapters > 0) {
                pFactoryD3D12->EnumerateAdapters(EngineCI.MinimumFeatureLevel, NumAdapters, Adapters.data());
            } else {
#if D3D11_SUPPORTED
                LOG_ERROR_MESSAGE("Failed to find Direct3D12-compatible hardware adapters. Attempting to initialize the engine in Direct3D11 mode.");
                deviceType_ = RENDER_DEVICE_TYPE_D3D11;
                return Initialize(nativeWindow, deviceType_);
#else
                LOG_ERROR_AND_THROW("Failed to find Direct3D12-compatible hardware adapters.");
#endif
            }

            if (adapterType_ == ADAPTER_TYPE_SOFTWARE) {
                for (Uint32 i = 0; i < Adapters.size(); ++i) {
                    if (Adapters[i].Type == adapterType_) {
                        adapterId_ = i;
                        LOG_INFO_MESSAGE("Found software adapter '", Adapters[i].Description, "'");
                        break;
                    }
                }
            }

            adapterAttribs_ = Adapters[adapterId_];
            if (adapterType_ != ADAPTER_TYPE_SOFTWARE) {
                Uint32 NumDisplayModes = 0;
                pFactoryD3D12->EnumerateDisplayModes(EngineCI.MinimumFeatureLevel, adapterId_, 0, TEX_FORMAT_RGBA8_UNORM_SRGB, NumDisplayModes, nullptr);
                displayModes_.resize(NumDisplayModes);
                pFactoryD3D12->EnumerateDisplayModes(EngineCI.MinimumFeatureLevel, adapterId_, 0, TEX_FORMAT_RGBA8_UNORM_SRGB, NumDisplayModes, displayModes_.data());
            }

            EngineCI.AdapterId = adapterId_;
            ppContexts.resize(1 + EngineCI.NumDeferredContexts);
            pFactoryD3D12->CreateDeviceAndContextsD3D12(EngineCI, &device_, ppContexts.data());
            if (!device_) {
                LOG_ERROR_AND_THROW("Failed to create Direct3D12 render device and contexts.");
            }
        } break;
#endif

#if VULKAN_SUPPORTED
        case RENDER_DEVICE_TYPE_VULKAN: {
#if EXPLICITLY_LOAD_ENGINE_VK_DLL
            // Load the dll and import GetEngineFactoryVk() function
            auto GetEngineFactoryVk = LoadGraphicsEngineVk();
#endif
            EngineVkCreateInfo EngVkAttribs;
#ifdef DILIGENT_DEVELOPMENT
            EngVkAttribs.EnableValidation = true;
#endif
            if (validationLevel_ >= 1) {
                EngVkAttribs.EnableValidation = true;
            } else if (validationLevel_ == 0) {
                EngVkAttribs.EnableValidation = false;
            }

            GetEngineInitializationAttribs(deviceType_, EngVkAttribs);
            ppContexts.resize(1 + EngVkAttribs.NumDeferredContexts);
            auto* pFactoryVk = GetEngineFactoryVk();
            engineFactory_ = pFactoryVk;
            pFactoryVk->CreateDeviceAndContextsVk(EngVkAttribs, &device_, ppContexts.data());
            if (!device_) {
                LOG_ERROR_AND_THROW("Failed to create Vulkan render device and contexts.");
            }
        } break;
#endif

#if METAL_SUPPORTED
        case RENDER_DEVICE_TYPE_METAL: {
            EngineMtlCreateInfo MtlAttribs;

            GetEngineInitializationAttribs(deviceType_, MtlAttribs);
            ppContexts.resize(1 + MtlAttribs.NumDeferredContexts);
            auto* pFactoryMtl = GetEngineFactoryMtl();
            pFactoryMtl->CreateDeviceAndContextsMtl(MtlAttribs, &device_, ppContexts.data());
        } break;
#endif

        default:
            LOG_ERROR_AND_THROW("Unknown device type");
            break;
        }

        immediateContext_.Attach(ppContexts[0]);
        auto NumDeferredCtx = ppContexts.size() - 1;
        deferredContexts_.resize(NumDeferredCtx);
        for (Uint32 ctx = 0; ctx < NumDeferredCtx; ++ctx)
            deferredContexts_[ctx].Attach(ppContexts[1 + ctx]);

        // The first swapchain is primary, the reset are not
        swapChainInitDesc_.IsPrimary = true;
        swapChain_ = CreateSwapChain(nativeWindow);
        swapChainInitDesc_.IsPrimary = false;

        return true;
    }

    void Uninitialize()
    {
        swapChain_ = nullptr;
        deferredContexts_.clear();
        immediateContext_ = nullptr;
        device_ = nullptr;
        engineFactory_ = nullptr;
    }

    RENDER_DEVICE_TYPE deviceType_ { RENDER_DEVICE_TYPE_UNDEFINED };
    RefCntAutoPtr<IEngineFactory> engineFactory_ { nullptr };
    RefCntAutoPtr<IRenderDevice> device_ { nullptr };
    RefCntAutoPtr<IDeviceContext> immediateContext_ { nullptr };
    std::vector<RefCntAutoPtr<IDeviceContext>> deferredContexts_ {};
    RefCntAutoPtr<ISwapChain> swapChain_ { nullptr };
    GraphicsAdapterInfo adapterAttribs_ {};
    std::vector<DisplayModeAttribs> displayModes_ {};

    int validationLevel_ { -1 };
    Uint32 adapterId_ = 0;
    ADAPTER_TYPE adapterType_ = ADAPTER_TYPE_UNKNOWN;
    Uint32 maxFrameLatency_ = SwapChainDesc {}.BufferCount;

    // We will need this when we have to recreate the swap chain (on Android)
    SwapChainDesc swapChainInitDesc_;
};

static LoongRHIImpl gRhiImpl {};

bool LoongRHIManager::Initialize(GLFWwindow* glfwWindow, RENDER_DEVICE_TYPE deviceType)
{
    NativeWindow nativeWindow = GetNativeWindow(glfwWindow);
    return gRhiImpl.Initialize(nativeWindow, deviceType);
}

void LoongRHIManager::Uninitialize()
{
    gRhiImpl.Uninitialize();
}

RefCntAutoPtr<ISwapChain> LoongRHIManager::GetPrimarySwapChain()
{
    return gRhiImpl.swapChain_;
}

RefCntAutoPtr<IRenderDevice> LoongRHIManager::GetDevice()
{
    return gRhiImpl.device_;
}

RefCntAutoPtr<IDeviceContext> LoongRHIManager::GetImmediateContext()
{
    return gRhiImpl.immediateContext_;
}

RefCntAutoPtr<ISwapChain> LoongRHIManager::CreateSwapChain(GLFWwindow* glfwWindow)
{
    NativeWindow nativeWindow = GetNativeWindow(glfwWindow);
    return gRhiImpl.CreateSwapChain(nativeWindow);
}

RHI::RefCntAutoPtr<RHI::IPipelineState> LoongRHIManager::CreateGraphicsPSOForCurrentSwapChain(
    ISwapChain* swapChain,
    const char* pipelineName, const ShaderCreateInfo& vs, const ShaderCreateInfo& ps,
    InputLayoutDesc inputLayout, PipelineResourceLayoutDesc resourceLayout,
    bool depthEnabled, CULL_MODE cullMode, PRIMITIVE_TOPOLOGY topology)
{
    auto& device = gRhiImpl.device_;

    RHI::PipelineStateCreateInfo psoCreateInfo;
    RHI::PipelineStateDesc& psoDesc = psoCreateInfo.PSODesc;

    // Pipeline state name is used by the engine to report issues.
    // It is always a good idea to give objects descriptive names.
    psoDesc.Name = pipelineName;

    // This is a graphics pipeline
    psoDesc.PipelineType = RHI::PIPELINE_TYPE_GRAPHICS;

    // This tutorial will render to a single render target
    psoDesc.GraphicsPipeline.NumRenderTargets = 1;
    // Set render target format which is the format of the swap chain's color buffer
    psoDesc.GraphicsPipeline.RTVFormats[0] = swapChain->GetDesc().ColorBufferFormat;
    // Use the depth buffer format from the swap chain
    psoDesc.GraphicsPipeline.DSVFormat = swapChain->GetDesc().DepthBufferFormat;
    // Primitive topology defines what kind of primitives will be rendered by this pipeline state
    psoDesc.GraphicsPipeline.PrimitiveTopology = topology;
    // No back face culling for this tutorial
    psoDesc.GraphicsPipeline.RasterizerDesc.CullMode = cullMode;
    // Disable depth testing
    psoDesc.GraphicsPipeline.DepthStencilDesc.DepthEnable = depthEnabled;

    // Create a vertex shader
    RHI::RefCntAutoPtr<RHI::IShader> pVS;
    device->CreateShader(vs, &pVS);

    // Create a pixel shader
    RHI::RefCntAutoPtr<RHI::IShader> pPS;
    device->CreateShader(ps, &pPS);

    // Finally, create the pipeline state
    psoDesc.GraphicsPipeline.pVS = pVS;
    psoDesc.GraphicsPipeline.pPS = pPS;

    psoDesc.GraphicsPipeline.InputLayout = inputLayout;
    psoDesc.ResourceLayout = resourceLayout;

    RefCntAutoPtr<IPipelineState> pso;
    device->CreatePipelineState(psoCreateInfo, &pso);

    return pso;
}

RefCntAutoPtr<IBuffer> LoongRHIManager::CreateUniformBuffer(const char* bufferName, uint32_t size, const void* initialData, USAGE usage, BIND_FLAGS bindFlags, CPU_ACCESS_FLAGS cpuAccessFlags)
{
    RefCntAutoPtr<IBuffer> buffer;
    Diligent::CreateUniformBuffer(gRhiImpl.device_, size, bufferName, &buffer, usage, bindFlags, cpuAccessFlags, const_cast<void*>(initialData));
    return buffer;
}

RefCntAutoPtr<IBuffer> LoongRHIManager::CreateVertexBuffer(const char* bufferName, uint32_t size, const void* initialData, USAGE usage, BIND_FLAGS bindFlags)
{
    assert(initialData != nullptr);
    assert(size > 0);

    RefCntAutoPtr<IBuffer> buffer;
    BufferDesc vertBuffDesc;
    vertBuffDesc.Name = bufferName;
    vertBuffDesc.Usage = usage;
    vertBuffDesc.BindFlags = bindFlags;
    vertBuffDesc.uiSizeInBytes = size;
    BufferData vbData;
    vbData.pData = initialData;
    vbData.DataSize = size;
    gRhiImpl.device_->CreateBuffer(vertBuffDesc, &vbData, &buffer);

    return buffer;
}

RefCntAutoPtr<ITexture> LoongRHIManager::CreateTextureFromFile(const char* file, bool isSrgb)
{
    RefCntAutoPtr<ITexture> texture;
    TextureLoadInfo loadInfo;
    loadInfo.IsSRGB = isSrgb;
    Diligent::CreateTextureFromFile(file, loadInfo, gRhiImpl.device_, &texture);
    return texture;
}

}