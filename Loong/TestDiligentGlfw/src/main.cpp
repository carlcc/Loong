
#if PLATFORM_WIN32
#elif PLATFORM_LINUX
#define GLFW_EXPOSE_NATIVE_X11
#define GLFW_EXPOSE_NATIVE_WAYLAND
#elif PLATFORM_MACOS
#else
#error Unknown platform.
#endif
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include "../platform/GetNativeWindow.h"
#include <PlatformDefinitions.h>
#include <Errors.hpp>
#include <StringTools.hpp>
#include <MapHelper.hpp>
#include <FileWrapper.hpp>
#include <DeviceContext.h>
#include <EngineFactory.h>
#include <LoongFoundation/LoongLogger.h>
#include <RefCntAutoPtr.hpp>
#include <RenderDevice.h>
#include <ScreenCapture.hpp>
#include <SwapChain.h>
#include <cassert>
#include <iostream>

#if D3D11_SUPPORTED
#include <EngineFactoryD3D11.h>
#endif

#if D3D12_SUPPORTED
#include <EngineFactoryD3D12.h>
#endif

#if GL_SUPPORTED || GLES_SUPPORTED
#include <EngineFactoryOpenGL.h>
#endif

#if VULKAN_SUPPORTED
#include <EngineFactoryVk.h>
#endif

#if METAL_SUPPORTED
#include <EngineFactoryMtl.h>
#endif

using namespace Diligent;

static const char* VSSource = R"(
struct PSInput 
{ 
    float4 Pos   : SV_POSITION; 
    float3 Color : COLOR; 
};

void main(in  uint    VertId : SV_VertexID,
          out PSInput PSIn) 
{
    float4 Pos[3];
    Pos[0] = float4(-0.5, -0.5, 0.0, 1.0);
    Pos[1] = float4( 0.0, +0.5, 0.0, 1.0);
    Pos[2] = float4(+0.5, -0.5, 0.0, 1.0);

    float3 Col[3];
    Col[0] = float3(1.0, 0.0, 0.0); // red
    Col[1] = float3(0.0, 1.0, 0.0); // green
    Col[2] = float3(0.0, 0.0, 1.0); // blue

    PSIn.Pos   = Pos[VertId];
    PSIn.Color = Col[VertId];
}
)";

// Pixel shader simply outputs interpolated vertex color
static const char* PSSource = R"(
struct PSInput 
{ 
    float4 Pos   : SV_POSITION; 
    float3 Color : COLOR; 
};

struct PSOutput
{ 
    float4 Color : SV_TARGET; 
};

void main(in  PSInput  PSIn,
          out PSOutput PSOut)
{
    PSOut.Color = float4(PSIn.Color.rgb, 1.0);
}
)";

struct WindowConfig {
    int width = 640;
    int height = 480;
    const char* title = "Test Diligent Glfw";
};

struct DiligentAppInitInfo {
    IEngineFactory* pEngineFactory = nullptr;
    IRenderDevice* pDevice = nullptr;
    IDeviceContext** ppContexts = nullptr;
    Uint32 NumDeferredCtx = 0;
    ISwapChain* pSwapChain = nullptr;
    // ImGuiImplDiligent* pImGui = nullptr;
};

class DiligentApp {
public:
    RefCntAutoPtr<IEngineFactory> engineFactory_;
    RefCntAutoPtr<IRenderDevice> device_;
    RefCntAutoPtr<IDeviceContext> immediateContext_;
    std::vector<RefCntAutoPtr<IDeviceContext>> deferredContexts_;
    RefCntAutoPtr<ISwapChain> swapChain_;
    // ImGuiImplDiligent* imGui_ = nullptr;

    /// Child
    RefCntAutoPtr<IPipelineState> pso_;

    bool Initialize(const DiligentAppInitInfo& info)
    {
        engineFactory_ = info.pEngineFactory;
        device_ = info.pDevice;
        swapChain_ = info.pSwapChain;
        immediateContext_ = info.ppContexts[0];
        deferredContexts_.resize(info.NumDeferredCtx);
        for (Uint32 ctx = 0; ctx < info.NumDeferredCtx; ++ctx)
            deferredContexts_[ctx] = info.ppContexts[1 + ctx];
        // imGui_ = InitInfo.pImGui;

        /// Child app
        // Pipeline state object encompasses configuration of all GPU stages
        // Pipeline state object encompasses configuration of all GPU stages

        PipelineStateCreateInfo psoCreateInfo;
        PipelineStateDesc& psoDesc = psoCreateInfo.PSODesc;

        // Pipeline state name is used by the engine to report issues.
        // It is always a good idea to give objects descriptive names.
        psoDesc.Name = "Simple triangle PSO";

        // This is a graphics pipeline
        psoDesc.PipelineType = PIPELINE_TYPE_GRAPHICS;

        // clang-format off
        // This tutorial will render to a single render target
        psoDesc.GraphicsPipeline.NumRenderTargets             = 1;
        // Set render target format which is the format of the swap chain's color buffer
        psoDesc.GraphicsPipeline.RTVFormats[0]                = swapChain_->GetDesc().ColorBufferFormat;
        // Use the depth buffer format from the swap chain
        psoDesc.GraphicsPipeline.DSVFormat                    = swapChain_->GetDesc().DepthBufferFormat;
        // Primitive topology defines what kind of primitives will be rendered by this pipeline state
        psoDesc.GraphicsPipeline.PrimitiveTopology            = PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        // No back face culling for this tutorial
        psoDesc.GraphicsPipeline.RasterizerDesc.CullMode      = CULL_MODE_NONE;
        // Disable depth testing
        psoDesc.GraphicsPipeline.DepthStencilDesc.DepthEnable = False;
        // clang-format on

        ShaderCreateInfo ShaderCI;
        // Tell the system that the shader source code is in HLSL.
        // For OpenGL, the engine will convert this into GLSL under the hood.
        ShaderCI.SourceLanguage = SHADER_SOURCE_LANGUAGE_HLSL;
        // OpenGL backend requires emulated combined HLSL texture samplers (g_Texture + g_Texture_sampler combination)
        ShaderCI.UseCombinedTextureSamplers = true;
        // Create a vertex shader
        RefCntAutoPtr<IShader> pVS;
        {
            ShaderCI.Desc.ShaderType = SHADER_TYPE_VERTEX;
            ShaderCI.EntryPoint = "main";
            ShaderCI.Desc.Name = "Triangle vertex shader";
            ShaderCI.Source = VSSource;
            device_->CreateShader(ShaderCI, &pVS);
        }

        // Create a pixel shader
        RefCntAutoPtr<IShader> pPS;
        {
            ShaderCI.Desc.ShaderType = SHADER_TYPE_PIXEL;
            ShaderCI.EntryPoint = "main";
            ShaderCI.Desc.Name = "Triangle pixel shader";
            ShaderCI.Source = PSSource;
            device_->CreateShader(ShaderCI, &pPS);
        }

        // Finally, create the pipeline state
        psoDesc.GraphicsPipeline.pVS = pVS;
        psoDesc.GraphicsPipeline.pPS = pPS;
        device_->CreatePipelineState(psoCreateInfo, &pso_);
        return true;
    }

    void Update() {}

    void Render()
    {
        {
            if (!immediateContext_ || !swapChain_)
                return;

            ITextureView* pRTV = swapChain_->GetCurrentBackBufferRTV();
            ITextureView* pDSV = swapChain_->GetDepthBufferDSV();
            immediateContext_->SetRenderTargets(1, &pRTV, pDSV, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
        }

        // Clear the back buffer
        const float ClearColor[] = { 0.350f, 0.350f, 0.350f, 1.0f };
        // Let the engine perform required state transitions
        auto* pRTV = swapChain_->GetCurrentBackBufferRTV();
        auto* pDSV = swapChain_->GetDepthBufferDSV();
        immediateContext_->ClearRenderTarget(pRTV, ClearColor, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
        immediateContext_->ClearDepthStencil(pDSV, CLEAR_DEPTH_FLAG, 1.f, 0, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

        // Set the pipeline state in the immediate context
        immediateContext_->SetPipelineState(pso_);

        // Typically we should now call CommitShaderResources(), however shaders in this example don't
        // use any resources.

        DrawAttribs drawAttrs;
        drawAttrs.NumVertices = 3; // We will render 3 vertices
        immediateContext_->Draw(drawAttrs);
    }

    void Present()
    {
        if (!swapChain_)
            return;
        bool vsync = 1;
        swapChain_->Present(vsync ? 1 : 0);
    }
};

class DiligentTest {
public:
    DiligentApp app;

#if 1
    void GetEngineInitializationAttribs(RENDER_DEVICE_TYPE DeviceType, EngineCreateInfo& EngineCI, SwapChainDesc& /*SCDesc*/)
    {
        switch (DeviceType) {
#if D3D11_SUPPORTED
        case RENDER_DEVICE_TYPE_D3D11: {
            //EngineD3D11CreateInfo& EngineD3D11CI = static_cast<EngineD3D11CreateInfo&>(EngineCI);
        } break;
#endif

#if D3D12_SUPPORTED
        case RENDER_DEVICE_TYPE_D3D12: {
            EngineD3D12CreateInfo& EngineD3D12CI = static_cast<EngineD3D12CreateInfo&>(EngineCI);
            EngineD3D12CI.GPUDescriptorHeapDynamicSize[0] = 32768;
            EngineD3D12CI.GPUDescriptorHeapSize[1] = 128;
            EngineD3D12CI.GPUDescriptorHeapDynamicSize[1] = 2048 - 128;
            EngineD3D12CI.DynamicDescriptorAllocationChunkSize[0] = 32;
            EngineD3D12CI.DynamicDescriptorAllocationChunkSize[1] = 8; // D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER
        } break;
#endif

#if VULKAN_SUPPORTED
        case RENDER_DEVICE_TYPE_VULKAN: {
            // EngineVkCreateInfo& EngVkAttribs = static_cast<EngineVkCreateInfo&>(EngineCI);
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

    bool InitializeDiligentEngine()
    {
#if PLATFORM_MACOS
        // We need at least 3 buffers on Metal to avoid massive
        // peformance degradation in full screen mode.
        // https://github.com/KhronosGroup/MoltenVK/issues/808
        swapChainInitDesc_.BufferCount = 3;
#endif
        NativeWindow nativeWindow = GetNativeWindow(glfwWindow_);

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

            GetEngineInitializationAttribs(deviceType_, EngineCI, swapChainInitDesc_);

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

            pFactoryD3D11->CreateSwapChainD3D11(device_, ppContexts[0], swapChainInitDesc_, FullScreenModeDesc {}, nativeWindow, &swapChain_);
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

            GetEngineInitializationAttribs(deviceType_, EngineCI, swapChainInitDesc_);

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
                return InitializeDiligentEngine();
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

            if (!swapChain_)
                pFactoryD3D12->CreateSwapChainD3D12(device_, ppContexts[0], swapChainInitDesc_, FullScreenModeDesc {}, nativeWindow, &swapChain_);
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

            GetEngineInitializationAttribs(deviceType_, EngVkAttribs, swapChainInitDesc_);
            ppContexts.resize(1 + EngVkAttribs.NumDeferredContexts);
            auto* pFactoryVk = GetEngineFactoryVk();
            engineFactory_ = pFactoryVk;
            pFactoryVk->CreateDeviceAndContextsVk(EngVkAttribs, &device_, ppContexts.data());
            if (!device_) {
                LOG_ERROR_AND_THROW("Failed to create Vulkan render device and contexts.");
            }

            if (!swapChain_)
                pFactoryVk->CreateSwapChainVk(device_, ppContexts[0], swapChainInitDesc_, nativeWindow, &swapChain_);
        } break;
#endif

#if METAL_SUPPORTED
        case RENDER_DEVICE_TYPE_METAL: {
            EngineMtlCreateInfo MtlAttribs;

            GetEngineInitializationAttribs(deviceType_, MtlAttribs, swapChainInitDesc_);
            ppContexts.resize(1 + MtlAttribs.NumDeferredContexts);
            auto* pFactoryMtl = GetEngineFactoryMtl();
            pFactoryMtl->CreateDeviceAndContextsMtl(MtlAttribs, &device_, ppContexts.data());

            if (!swapChain_)
                pFactoryMtl->CreateSwapChainMtl(device_, ppContexts[0], swapChainInitDesc_, nativeWindow, &swapChain_);
        } break;
#endif

        default:
            LOG_ERROR_AND_THROW("Unknown device type");
            break;
        }

        switch (deviceType_) {
        // clang-format off
        case RENDER_DEVICE_TYPE_D3D11:  appTitle_.append(" (D3D11)");    break;
        case RENDER_DEVICE_TYPE_D3D12:  appTitle_.append(" (D3D12)");    break;
        case RENDER_DEVICE_TYPE_GL:     appTitle_.append(" (OpenGL)");   break;
        case RENDER_DEVICE_TYPE_GLES:   appTitle_.append(" (OpenGLES)"); break;
        case RENDER_DEVICE_TYPE_VULKAN: appTitle_.append(" (Vulkan)");   break;
        case RENDER_DEVICE_TYPE_METAL:  appTitle_.append(" (Metal)");    break;
        default: UNEXPECTED("Unknown/unsupported device type");
            // clang-format on
        }

        immediateContext_.Attach(ppContexts[0]);
        auto NumDeferredCtx = ppContexts.size() - 1;
        deferredContexts_.resize(NumDeferredCtx);
        for (Uint32 ctx = 0; ctx < NumDeferredCtx; ++ctx)
            deferredContexts_[ctx].Attach(ppContexts[1 + ctx]);

        return true;
    }
#endif

    bool Initialize()
    {
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

        WindowConfig config;
        glfwWindow_ = glfwCreateWindow(config.width, config.height, config.title, nullptr, nullptr);
        if (nullptr == glfwWindow_) {
            LOONG_ERROR("Create glfw window failed");
            return false;
        }
        glfwSetWindowUserPointer(glfwWindow_, this);

        // Config
        // NOTE: METAL is not really supported
        deviceType_ = RENDER_DEVICE_TYPE_VULKAN;

        if (!InitializeDiligentEngine()) {
            LOONG_ERROR("Initialize diligent engine failed");
            return false;
        }

        glfwSetWindowSizeCallback(glfwWindow_, [](GLFWwindow* w, int width, int height) {
            DiligentTest* t = (DiligentTest*)glfwGetWindowUserPointer(w);
            t->swapChain_->Resize(width, height);
            std::cout << "Window size" << std::endl;
        });
        glfwSetFramebufferSizeCallback(glfwWindow_, [](GLFWwindow* w, int width, int height) {
            DiligentTest* t = (DiligentTest*)glfwGetWindowUserPointer(w);
            t->swapChain_->Resize(width, height);
            std::cout << "Framebuffer size" << std::endl;
        });

        // Initialize app

        const auto& SCDesc = swapChain_->GetDesc();
        maxFrameLatency_ = SCDesc.BufferCount;

        std::vector<IDeviceContext*> ppContexts(1 + deferredContexts_.size());
        ppContexts[0] = immediateContext_;
        Uint32 NumDeferredCtx = static_cast<Uint32>(deferredContexts_.size());
        for (size_t ctx = 0; ctx < deferredContexts_.size(); ++ctx)
            ppContexts[1 + ctx] = deferredContexts_[ctx];

        DiligentAppInitInfo initInfo;
        initInfo.pEngineFactory = engineFactory_;
        initInfo.pDevice = device_;
        initInfo.ppContexts = ppContexts.data();
        initInfo.NumDeferredCtx = NumDeferredCtx;
        initInfo.pSwapChain = swapChain_;
        // initInfo.pImGui = imGui_.get();
        app.Initialize(initInfo);
        return true;
    }

    int Run()
    {
        glfwSetWindowTitle(glfwWindow_, appTitle_.c_str());

        while (!glfwWindowShouldClose(glfwWindow_)) {
            glfwPollEvents();

            app.Update();
            app.Render();
            app.Present();
        }

        return 0;
    }

    GLFWwindow* glfwWindow_;

    RENDER_DEVICE_TYPE deviceType_ = RENDER_DEVICE_TYPE_UNDEFINED;
    RefCntAutoPtr<IEngineFactory> engineFactory_;
    RefCntAutoPtr<IRenderDevice> device_;
    RefCntAutoPtr<IDeviceContext> immediateContext_;
    std::vector<RefCntAutoPtr<IDeviceContext>> deferredContexts_;
    RefCntAutoPtr<ISwapChain> swapChain_;
    GraphicsAdapterInfo adapterAttribs_;
    std::vector<DisplayModeAttribs> displayModes_;

    int validationLevel_ = -1;
    std::string appTitle_;
    Uint32 adapterId_ = 0;
    ADAPTER_TYPE adapterType_ = ADAPTER_TYPE_UNKNOWN;
    Uint32 maxFrameLatency_ = SwapChainDesc {}.BufferCount;

    // We will need this when we have to recreate the swap chain (on Android)
    SwapChainDesc swapChainInitDesc_;
    // std::unique_ptr<ImGuiImplDiligent> imGui_;
};

int main(int argc, char** argv)
{
    glfwInit();

    auto listener = Loong::Foundation::Logger::Get().SubscribeLog([](const Loong::Foundation::LogItem& logItem) {
        std::cout << "[" << logItem.level << "][" << logItem.location << "]: " << logItem.message << std::endl;
    });

    DiligentTest d;
    d.Initialize();
    d.Run();

    glfwTerminate();
    return 0;
}
