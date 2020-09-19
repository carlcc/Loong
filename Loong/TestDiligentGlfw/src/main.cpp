
#if PLATFORM_WIN32
#define GLFW_EXPOSE_NATIVE_WIN32
#elif PLATFORM_LINUX
#define GLFW_EXPOSE_NATIVE_X11
#define GLFW_EXPOSE_NATIVE_WAYLAND
#elif PLATFORM_MACOS
#define GLFW_EXPOSE_NATIVE_COCOA
#else
#error Unknown platform.
#endif
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

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
    RefCntAutoPtr<IEngineFactory> m_pEngineFactory;
    RefCntAutoPtr<IRenderDevice> m_pDevice;
    RefCntAutoPtr<IDeviceContext> m_pImmediateContext;
    std::vector<RefCntAutoPtr<IDeviceContext>> m_pDeferredContexts;
    RefCntAutoPtr<ISwapChain> m_pSwapChain;
    // ImGuiImplDiligent* m_pImGui = nullptr;

    /// Child
    RefCntAutoPtr<IPipelineState> m_pPSO;

    bool Initialize(const DiligentAppInitInfo& info)
    {
        m_pEngineFactory = info.pEngineFactory;
        m_pDevice = info.pDevice;
        m_pSwapChain = info.pSwapChain;
        m_pImmediateContext = info.ppContexts[0];
        m_pDeferredContexts.resize(info.NumDeferredCtx);
        for (Uint32 ctx = 0; ctx < info.NumDeferredCtx; ++ctx)
            m_pDeferredContexts[ctx] = info.ppContexts[1 + ctx];
        // m_pImGui = InitInfo.pImGui;

        /// Child app
        // Pipeline state object encompasses configuration of all GPU stages
        // Pipeline state object encompasses configuration of all GPU stages

        PipelineStateCreateInfo PSOCreateInfo;
        PipelineStateDesc& PSODesc = PSOCreateInfo.PSODesc;

        // Pipeline state name is used by the engine to report issues.
        // It is always a good idea to give objects descriptive names.
        PSODesc.Name = "Simple triangle PSO";

        // This is a graphics pipeline
        PSODesc.PipelineType = PIPELINE_TYPE_GRAPHICS;

        // clang-format off
        // This tutorial will render to a single render target
        PSODesc.GraphicsPipeline.NumRenderTargets             = 1;
        // Set render target format which is the format of the swap chain's color buffer
        PSODesc.GraphicsPipeline.RTVFormats[0]                = m_pSwapChain->GetDesc().ColorBufferFormat;
        // Use the depth buffer format from the swap chain
        PSODesc.GraphicsPipeline.DSVFormat                    = m_pSwapChain->GetDesc().DepthBufferFormat;
        // Primitive topology defines what kind of primitives will be rendered by this pipeline state
        PSODesc.GraphicsPipeline.PrimitiveTopology            = PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        // No back face culling for this tutorial
        PSODesc.GraphicsPipeline.RasterizerDesc.CullMode      = CULL_MODE_NONE;
        // Disable depth testing
        PSODesc.GraphicsPipeline.DepthStencilDesc.DepthEnable = False;
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
            m_pDevice->CreateShader(ShaderCI, &pVS);
        }

        // Create a pixel shader
        RefCntAutoPtr<IShader> pPS;
        {
            ShaderCI.Desc.ShaderType = SHADER_TYPE_PIXEL;
            ShaderCI.EntryPoint = "main";
            ShaderCI.Desc.Name = "Triangle pixel shader";
            ShaderCI.Source = PSSource;
            m_pDevice->CreateShader(ShaderCI, &pPS);
        }

        // Finally, create the pipeline state
        PSODesc.GraphicsPipeline.pVS = pVS;
        PSODesc.GraphicsPipeline.pPS = pPS;
        m_pDevice->CreatePipelineState(PSOCreateInfo, &m_pPSO);
        return true;
    }

    void Update() {}

    void Render()
    {
        {
            if (!m_pImmediateContext || !m_pSwapChain)
                return;

            ITextureView* pRTV = m_pSwapChain->GetCurrentBackBufferRTV();
            ITextureView* pDSV = m_pSwapChain->GetDepthBufferDSV();
            m_pImmediateContext->SetRenderTargets(1, &pRTV, pDSV, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
        }

        // Clear the back buffer
        const float ClearColor[] = { 0.350f, 0.350f, 0.350f, 1.0f };
        // Let the engine perform required state transitions
        auto* pRTV = m_pSwapChain->GetCurrentBackBufferRTV();
        auto* pDSV = m_pSwapChain->GetDepthBufferDSV();
        m_pImmediateContext->ClearRenderTarget(pRTV, ClearColor, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
        m_pImmediateContext->ClearDepthStencil(pDSV, CLEAR_DEPTH_FLAG, 1.f, 0, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

        // Set the pipeline state in the immediate context
        m_pImmediateContext->SetPipelineState(m_pPSO);

        // Typically we should now call CommitShaderResources(), however shaders in this example don't
        // use any resources.

        DrawAttribs drawAttrs;
        drawAttrs.NumVertices = 3; // We will render 3 vertices
        m_pImmediateContext->Draw(drawAttrs);
    }

    void Present()
    {
        if (!m_pSwapChain)
            return;
        bool m_bVSync = 1;
        m_pSwapChain->Present(m_bVSync ? 1 : 0);
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

#if GL_SUPPORTED
        case RENDER_DEVICE_TYPE_GL: {
            // Nothing to do
        } break;
#endif

#if GLES_SUPPORTED
        case RENDER_DEVICE_TYPE_GLES: {
            // Nothing to do
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

    NativeWindow GetNativeWindow()
    {
        NativeWindow nativeWindow {};
#if PLATFORM_WIN32
        nativeWindow.hWnd = glfwGetWin32Window(glfwWindow_);
#elif PLATFORM_LINUX
#error TODO
#elif PLATFORM_MACOS
#error TODO
#else
#error Unknown platform.
#endif
        return nativeWindow;
    }

    bool InitializeDiligentEngine()
    {
#if PLATFORM_MACOS
        // We need at least 3 buffers on Metal to avoid massive
        // peformance degradation in full screen mode.
        // https://github.com/KhronosGroup/MoltenVK/issues/808
        m_SwapChainInitDesc.BufferCount = 3;
#endif
        NativeWindow nativeWindow = GetNativeWindow();

        std::vector<IDeviceContext*> ppContexts;
        switch (m_DeviceType) {
#if D3D11_SUPPORTED
        case RENDER_DEVICE_TYPE_D3D11: {
            EngineD3D11CreateInfo EngineCI;

#ifdef DILIGENT_DEVELOPMENT
            EngineCI.DebugFlags |= D3D11_DEBUG_FLAG_CREATE_DEBUG_DEVICE | D3D11_DEBUG_FLAG_VERIFY_COMMITTED_SHADER_RESOURCES;
#endif
#ifdef DILIGENT_DEBUG
            EngineCI.DebugFlags |= D3D11_DEBUG_FLAG_VERIFY_COMMITTED_RESOURCE_RELEVANCE;
#endif

            if (m_ValidationLevel >= 1) {
                EngineCI.DebugFlags = D3D11_DEBUG_FLAG_CREATE_DEBUG_DEVICE | D3D11_DEBUG_FLAG_VERIFY_COMMITTED_SHADER_RESOURCES | D3D11_DEBUG_FLAG_VERIFY_COMMITTED_RESOURCE_RELEVANCE;
            } else if (m_ValidationLevel == 0) {
                EngineCI.DebugFlags = D3D11_DEBUG_FLAG_NONE;
            }

            GetEngineInitializationAttribs(m_DeviceType, EngineCI, m_SwapChainInitDesc);

#if ENGINE_DLL
            // Load the dll and import GetEngineFactoryD3D11() function
            auto GetEngineFactoryD3D11 = LoadGraphicsEngineD3D11();
#endif
            auto* pFactoryD3D11 = GetEngineFactoryD3D11();
            m_pEngineFactory = pFactoryD3D11;
            Uint32 NumAdapters = 0;
            pFactoryD3D11->EnumerateAdapters(EngineCI.MinimumFeatureLevel, NumAdapters, 0);
            std::vector<GraphicsAdapterInfo> Adapters(NumAdapters);
            if (NumAdapters > 0) {
                pFactoryD3D11->EnumerateAdapters(EngineCI.MinimumFeatureLevel, NumAdapters, Adapters.data());
            } else {
                LOG_ERROR_AND_THROW("Failed to find Direct3D11-compatible hardware adapters");
            }

            if (m_AdapterType == ADAPTER_TYPE_SOFTWARE) {
                for (Uint32 i = 0; i < Adapters.size(); ++i) {
                    if (Adapters[i].Type == m_AdapterType) {
                        m_AdapterId = i;
                        LOG_INFO_MESSAGE("Found software adapter '", Adapters[i].Description, "'");
                        break;
                    }
                }
            }

            m_AdapterAttribs = Adapters[m_AdapterId];
            if (m_AdapterType != ADAPTER_TYPE_SOFTWARE) {
                Uint32 NumDisplayModes = 0;
                pFactoryD3D11->EnumerateDisplayModes(EngineCI.MinimumFeatureLevel, m_AdapterId, 0, TEX_FORMAT_RGBA8_UNORM_SRGB, NumDisplayModes, nullptr);
                m_DisplayModes.resize(NumDisplayModes);
                pFactoryD3D11->EnumerateDisplayModes(EngineCI.MinimumFeatureLevel, m_AdapterId, 0, TEX_FORMAT_RGBA8_UNORM_SRGB, NumDisplayModes, m_DisplayModes.data());
            }

            EngineCI.AdapterId = m_AdapterId;
            ppContexts.resize(1 + EngineCI.NumDeferredContexts);
            pFactoryD3D11->CreateDeviceAndContextsD3D11(EngineCI, &m_pDevice, ppContexts.data());
            if (!m_pDevice) {
                LOG_ERROR_AND_THROW("Failed to create Direct3D11 render device and contexts.");
            }

            pFactoryD3D11->CreateSwapChainD3D11(m_pDevice, ppContexts[0], m_SwapChainInitDesc, FullScreenModeDesc {}, nativeWindow, &m_pSwapChain);
        } break;
#endif

#if D3D12_SUPPORTED
        case RENDER_DEVICE_TYPE_D3D12: {
            EngineD3D12CreateInfo EngineCI;

#ifdef DILIGENT_DEVELOPMENT
            EngineCI.EnableDebugLayer = true;
#endif
            if (m_ValidationLevel >= 1) {
                EngineCI.EnableDebugLayer = true;
                if (m_ValidationLevel >= 2)
                    EngineCI.EnableGPUBasedValidation = true;
            } else if (m_ValidationLevel == 0) {
                EngineCI.EnableDebugLayer = false;
            }

            GetEngineInitializationAttribs(m_DeviceType, EngineCI, m_SwapChainInitDesc);

#if ENGINE_DLL
            // Load the dll and import GetEngineFactoryD3D12() function
            auto GetEngineFactoryD3D12 = LoadGraphicsEngineD3D12();
#endif
            auto* pFactoryD3D12 = GetEngineFactoryD3D12();
            if (!pFactoryD3D12->LoadD3D12()) {
                LOG_ERROR_AND_THROW("Failed to load Direct3D12");
            }

            m_pEngineFactory = pFactoryD3D12;
            Uint32 NumAdapters = 0;
            pFactoryD3D12->EnumerateAdapters(EngineCI.MinimumFeatureLevel, NumAdapters, 0);
            std::vector<GraphicsAdapterInfo> Adapters(NumAdapters);
            if (NumAdapters > 0) {
                pFactoryD3D12->EnumerateAdapters(EngineCI.MinimumFeatureLevel, NumAdapters, Adapters.data());
            } else {
#if D3D11_SUPPORTED
                LOG_ERROR_MESSAGE("Failed to find Direct3D12-compatible hardware adapters. Attempting to initialize the engine in Direct3D11 mode.");
                m_DeviceType = RENDER_DEVICE_TYPE_D3D11;
                return InitializeDiligentEngine();
#else
                LOG_ERROR_AND_THROW("Failed to find Direct3D12-compatible hardware adapters.");
#endif
            }

            if (m_AdapterType == ADAPTER_TYPE_SOFTWARE) {
                for (Uint32 i = 0; i < Adapters.size(); ++i) {
                    if (Adapters[i].Type == m_AdapterType) {
                        m_AdapterId = i;
                        LOG_INFO_MESSAGE("Found software adapter '", Adapters[i].Description, "'");
                        break;
                    }
                }
            }

            m_AdapterAttribs = Adapters[m_AdapterId];
            if (m_AdapterType != ADAPTER_TYPE_SOFTWARE) {
                Uint32 NumDisplayModes = 0;
                pFactoryD3D12->EnumerateDisplayModes(EngineCI.MinimumFeatureLevel, m_AdapterId, 0, TEX_FORMAT_RGBA8_UNORM_SRGB, NumDisplayModes, nullptr);
                m_DisplayModes.resize(NumDisplayModes);
                pFactoryD3D12->EnumerateDisplayModes(EngineCI.MinimumFeatureLevel, m_AdapterId, 0, TEX_FORMAT_RGBA8_UNORM_SRGB, NumDisplayModes, m_DisplayModes.data());
            }

            EngineCI.AdapterId = m_AdapterId;
            ppContexts.resize(1 + EngineCI.NumDeferredContexts);
            pFactoryD3D12->CreateDeviceAndContextsD3D12(EngineCI, &m_pDevice, ppContexts.data());
            if (!m_pDevice) {
                LOG_ERROR_AND_THROW("Failed to create Direct3D12 render device and contexts.");
            }

            if (!m_pSwapChain)
                pFactoryD3D12->CreateSwapChainD3D12(m_pDevice, ppContexts[0], m_SwapChainInitDesc, FullScreenModeDesc {}, nativeWindow, &m_pSwapChain);
        } break;
#endif

#if GL_SUPPORTED || GLES_SUPPORTED
        case RENDER_DEVICE_TYPE_GL:
        case RENDER_DEVICE_TYPE_GLES: {
#if EXPLICITLY_LOAD_ENGINE_GL_DLL
            // Load the dll and import GetEngineFactoryOpenGL() function
            auto GetEngineFactoryOpenGL = LoadGraphicsEngineOpenGL();
#endif
            auto* pFactoryOpenGL = GetEngineFactoryOpenGL();
            m_pEngineFactory = pFactoryOpenGL;
            EngineGLCreateInfo EngineCI;
            EngineCI.Window = nativeWindow;

#ifdef DILIGENT_DEVELOPMENT
            EngineCI.CreateDebugContext = true;
#endif
            if (m_ValidationLevel >= 1) {
                EngineCI.CreateDebugContext = true;
            } else if (m_ValidationLevel == 0) {
                EngineCI.CreateDebugContext = false;
            }

            GetEngineInitializationAttribs(m_DeviceType, EngineCI, m_SwapChainInitDesc);
            if (EngineCI.NumDeferredContexts != 0) {
                LOG_ERROR_MESSAGE("Deferred contexts are not supported in OpenGL mode");
                EngineCI.NumDeferredContexts = 0;
            }
            ppContexts.resize(1 + EngineCI.NumDeferredContexts);
            pFactoryOpenGL->CreateDeviceAndSwapChainGL(
                EngineCI, &m_pDevice, ppContexts.data(), m_SwapChainInitDesc, &m_pSwapChain);
            if (!m_pDevice) {
                LOG_ERROR_AND_THROW("Failed to create GL render device and contexts.");
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
            if (m_ValidationLevel >= 1) {
                EngVkAttribs.EnableValidation = true;
            } else if (m_ValidationLevel == 0) {
                EngVkAttribs.EnableValidation = false;
            }

            GetEngineInitializationAttribs(m_DeviceType, EngVkAttribs, m_SwapChainInitDesc);
            ppContexts.resize(1 + EngVkAttribs.NumDeferredContexts);
            auto* pFactoryVk = GetEngineFactoryVk();
            m_pEngineFactory = pFactoryVk;
            pFactoryVk->CreateDeviceAndContextsVk(EngVkAttribs, &m_pDevice, ppContexts.data());
            if (!m_pDevice) {
                LOG_ERROR_AND_THROW("Failed to create Vulkan render device and contexts.");
            }

            if (!m_pSwapChain)
                pFactoryVk->CreateSwapChainVk(m_pDevice, ppContexts[0], m_SwapChainInitDesc, nativeWindow, &m_pSwapChain);
        } break;
#endif

#if METAL_SUPPORTED
        case RENDER_DEVICE_TYPE_METAL: {
            EngineMtlCreateInfo MtlAttribs;

            m_TheSample->GetEngineInitializationAttribs(m_DeviceType, MtlAttribs, m_SwapChainInitDesc);
            ppContexts.resize(1 + MtlAttribs.NumDeferredContexts);
            auto* pFactoryMtl = GetEngineFactoryMtl();
            pFactoryMtl->CreateDeviceAndContextsMtl(MtlAttribs, &m_pDevice, ppContexts.data());

            if (!m_pSwapChain)
                pFactoryMtl->CreateSwapChainMtl(m_pDevice, ppContexts[0], m_SwapChainInitDesc, nativeWindow, &m_pSwapChain);
        } break;
#endif

        default:
            LOG_ERROR_AND_THROW("Unknown device type");
            break;
        }

        switch (m_DeviceType) {
        // clang-format off
        case RENDER_DEVICE_TYPE_D3D11:  m_AppTitle.append(" (D3D11)");    break;
        case RENDER_DEVICE_TYPE_D3D12:  m_AppTitle.append(" (D3D12)");    break;
        case RENDER_DEVICE_TYPE_GL:     m_AppTitle.append(" (OpenGL)");   break;
        case RENDER_DEVICE_TYPE_GLES:   m_AppTitle.append(" (OpenGLES)"); break;
        case RENDER_DEVICE_TYPE_VULKAN: m_AppTitle.append(" (Vulkan)");   break;
        case RENDER_DEVICE_TYPE_METAL:  m_AppTitle.append(" (Metal)");    break;
        default: UNEXPECTED("Unknown/unsupported device type");
            // clang-format on
        }

        m_pImmediateContext.Attach(ppContexts[0]);
        auto NumDeferredCtx = ppContexts.size() - 1;
        m_pDeferredContexts.resize(NumDeferredCtx);
        for (Uint32 ctx = 0; ctx < NumDeferredCtx; ++ctx)
            m_pDeferredContexts[ctx].Attach(ppContexts[1 + ctx]);

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
        m_DeviceType = RENDER_DEVICE_TYPE_VULKAN;

        if (!InitializeDiligentEngine()) {
            LOONG_ERROR("Initialize diligent engine failed");
            return false;
        }

        glfwSetWindowSizeCallback(glfwWindow_, [](GLFWwindow* w, int width, int height) {
            DiligentTest* t = (DiligentTest*)glfwGetWindowUserPointer(w);
            t->m_pSwapChain->Resize(width, height);
            std::cout << "Window size" << std::endl;
        });
        glfwSetFramebufferSizeCallback(glfwWindow_, [](GLFWwindow* w, int width, int height) {
            DiligentTest* t = (DiligentTest*)glfwGetWindowUserPointer(w);
            t->m_pSwapChain->Resize(width, height);
            std::cout << "Framebuffer size" << std::endl;
        });

        // Initialize app

        const auto& SCDesc = m_pSwapChain->GetDesc();
        m_MaxFrameLatency = SCDesc.BufferCount;

        std::vector<IDeviceContext*> ppContexts(1 + m_pDeferredContexts.size());
        ppContexts[0] = m_pImmediateContext;
        Uint32 NumDeferredCtx = static_cast<Uint32>(m_pDeferredContexts.size());
        for (size_t ctx = 0; ctx < m_pDeferredContexts.size(); ++ctx)
            ppContexts[1 + ctx] = m_pDeferredContexts[ctx];

        DiligentAppInitInfo initInfo;
        initInfo.pEngineFactory = m_pEngineFactory;
        initInfo.pDevice = m_pDevice;
        initInfo.ppContexts = ppContexts.data();
        initInfo.NumDeferredCtx = NumDeferredCtx;
        initInfo.pSwapChain = m_pSwapChain;
        // initInfo.pImGui = m_pImGui.get();
        app.Initialize(initInfo);
        return true;
    }

    int Run()
    {
        glfwSetWindowTitle(glfwWindow_, m_AppTitle.c_str());

        while (!glfwWindowShouldClose(glfwWindow_)) {
            glfwPollEvents();

            app.Update();
            app.Render();
            app.Present();
        }

        return 0;
    }

    GLFWwindow* glfwWindow_;

    RENDER_DEVICE_TYPE m_DeviceType = RENDER_DEVICE_TYPE_UNDEFINED;
    RefCntAutoPtr<IEngineFactory> m_pEngineFactory;
    RefCntAutoPtr<IRenderDevice> m_pDevice;
    RefCntAutoPtr<IDeviceContext> m_pImmediateContext;
    std::vector<RefCntAutoPtr<IDeviceContext>> m_pDeferredContexts;
    RefCntAutoPtr<ISwapChain> m_pSwapChain;
    GraphicsAdapterInfo m_AdapterAttribs;
    std::vector<DisplayModeAttribs> m_DisplayModes;

    int m_InitialWindowWidth = 0;
    int m_InitialWindowHeight = 0;
    int m_ValidationLevel = -1;
    std::string m_AppTitle;
    Uint32 m_AdapterId = 0;
    ADAPTER_TYPE m_AdapterType = ADAPTER_TYPE_UNKNOWN;
    std::string m_AdapterDetailsString;
    int m_SelectedDisplayMode = 0;
    bool m_bVSync = false;
    bool m_bFullScreenMode = false;
    bool m_bShowAdaptersDialog = true;
    bool m_bShowUI = true;
    double m_CurrentTime = 0;
    Uint32 m_MaxFrameLatency = SwapChainDesc {}.BufferCount;

    // We will need this when we have to recreate the swap chain (on Android)
    SwapChainDesc m_SwapChainInitDesc;

    std::unique_ptr<ScreenCapture> m_pScreenCapture;

    // std::unique_ptr<ImGuiImplDiligent> m_pImGui;

    // GoldenImageMode m_GoldenImgMode = GoldenImageMode::None;
    int m_GoldenImgPixelTolerance = 0;
    int m_ExitCode = 0;
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