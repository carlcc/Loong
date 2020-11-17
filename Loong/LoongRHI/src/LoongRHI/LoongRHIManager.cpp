//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//
#include "LoongRHI/LoongRHIManager.h"
#include "GetNativeWindow.h"
#include "LoongFileSystem/LoongFileSystem.h"
#include "LoongFoundation/LoongAssert.h"
#include "LoongFoundation/LoongLogger.h"
#include "LoongFoundation/LoongStringUtils.h"
#include <GraphicsUtilities.h>
#include <TextureUtilities.h>
#include <cassert>
#include <cstring>
#include <pugixml.hpp>
#include <unordered_map>
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

    RHI::GraphicsPipelineStateCreateInfo psoCreateInfo;
    RHI::PipelineStateDesc& psoDesc = psoCreateInfo.PSODesc;

    // Pipeline state name is used by the engine to report issues.
    // It is always a good idea to give objects descriptive names.
    psoDesc.Name = pipelineName;

    // This is a graphics pipeline
    psoDesc.PipelineType = RHI::PIPELINE_TYPE_GRAPHICS;

    // This tutorial will render to a single render target
    psoCreateInfo.GraphicsPipeline.NumRenderTargets = 1;
    // Set render target format which is the format of the swap chain's color buffer
    psoCreateInfo.GraphicsPipeline.RTVFormats[0] = swapChain->GetDesc().ColorBufferFormat;
    // Use the depth buffer format from the swap chain
    psoCreateInfo.GraphicsPipeline.DSVFormat = swapChain->GetDesc().DepthBufferFormat;
    // Primitive topology defines what kind of primitives will be rendered by this pipeline state
    psoCreateInfo.GraphicsPipeline.PrimitiveTopology = topology;
    // No back face culling for this tutorial
    psoCreateInfo.GraphicsPipeline.RasterizerDesc.CullMode = cullMode;
    // Disable depth testing
    psoCreateInfo.GraphicsPipeline.DepthStencilDesc.DepthEnable = depthEnabled;

    // Create a vertex shader
    RHI::RefCntAutoPtr<RHI::IShader> pVS;
    device->CreateShader(vs, &pVS);

    // Create a pixel shader
    RHI::RefCntAutoPtr<RHI::IShader> pPS;
    device->CreateShader(ps, &pPS);

    // Finally, create the pipeline state
    psoCreateInfo.pVS = pVS;
    psoCreateInfo.pPS = pPS;

    psoCreateInfo.GraphicsPipeline.InputLayout = inputLayout;
    psoDesc.ResourceLayout = resourceLayout;

    if (pPS == nullptr || pVS == nullptr) {
        return {};
    }
    RefCntAutoPtr<IPipelineState> pso;
    device->CreateGraphicsPipelineState(psoCreateInfo, &pso);

    return pso;
}

// A helper class to map strings to enum values
class PsoEnumMap {
private:
    PsoEnumMap() = default;

    std::unordered_map<std::string, RHI::PRIMITIVE_TOPOLOGY> topology_ {
        { "TRIANGLE_LIST", PRIMITIVE_TOPOLOGY_TRIANGLE_LIST },
        { "TRIANGLE_STRIP", PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP },
        { "LINE_LIST", PRIMITIVE_TOPOLOGY_LINE_LIST },
        { "LINE_STRIP", PRIMITIVE_TOPOLOGY_LINE_STRIP },
        { "POINT_LIST", PRIMITIVE_TOPOLOGY_POINT_LIST },
    };
    std::unordered_map<std::string, RHI::CULL_MODE> cullMode_ {
        { "BACK", CULL_MODE_BACK },
        { "FRONT", CULL_MODE_FRONT },
        { "NONE", CULL_MODE_NONE },
    };
    std::unordered_map<std::string, RHI::TEXTURE_FORMAT> textureFormat_ {
        { "BGRA8_UNORM_SRGB", TEX_FORMAT_BGRA8_UNORM_SRGB },
        { "RGBA8_UNORM_SRGB", TEX_FORMAT_RGBA8_UNORM_SRGB },
        { "D32_FLOAT", TEX_FORMAT_D32_FLOAT },
        { "D24_UNORM_S8_UINT", TEX_FORMAT_D24_UNORM_S8_UINT },
    };
    std::unordered_map<std::string, RHI::COMPARISON_FUNCTION> comparisonFunc_ {
        { "LESS", COMPARISON_FUNC_LESS },
        { "EQUAL", COMPARISON_FUNC_EQUAL },
        { "LESS_EQUAL", COMPARISON_FUNC_LESS_EQUAL },
        { "GREATER", COMPARISON_FUNC_GREATER },
        { "GREATER_EQUAL", COMPARISON_FUNC_GREATER_EQUAL },
        { "NOT_EQUAL", COMPARISON_FUNC_NOT_EQUAL },
        { "NEVER", COMPARISON_FUNC_NEVER },
        { "ALWAYS", COMPARISON_FUNC_ALWAYS },
    };
    std::unordered_map<std::string, RHI::VALUE_TYPE> valueType_ {
        { "INT8", VT_INT8 },
        { "INT16", VT_INT16 },
        { "INT32", VT_INT32 },
        { "UINT8", VT_UINT8 },
        { "UINT16", VT_UINT16 },
        { "UINT32", VT_UINT32 },
        { "FLOAT16", VT_FLOAT16 },
        { "FLOAT32", VT_FLOAT32 },
    };
    std::unordered_map<std::string, RHI::SHADER_RESOURCE_VARIABLE_TYPE> shaderResourceVarType_ {
        { "MUTABLE", SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE },
        { "STATIC", SHADER_RESOURCE_VARIABLE_TYPE_STATIC },
        { "DYNAMIC", SHADER_RESOURCE_VARIABLE_TYPE_DYNAMIC },
    };
    std::unordered_map<std::string, RHI::FILTER_TYPE> filterType_ {
        { "POINT", FILTER_TYPE_POINT },
        { "LINEAR", FILTER_TYPE_LINEAR },
        { "ANISOTROPIC", FILTER_TYPE_ANISOTROPIC },
        { "COMPARISON_POINT", FILTER_TYPE_COMPARISON_POINT },
        { "COMPARISON_LINEAR", FILTER_TYPE_COMPARISON_LINEAR },
        { "COMPARISON_ANISOTROPIC", FILTER_TYPE_COMPARISON_ANISOTROPIC },
    };
    std::unordered_map<std::string, RHI::TEXTURE_ADDRESS_MODE> addressMode_ {
        { "WRAP", TEXTURE_ADDRESS_WRAP },
        { "BORDER", TEXTURE_ADDRESS_BORDER },
        { "CLAMP", TEXTURE_ADDRESS_CLAMP },
        { "MIRROR", TEXTURE_ADDRESS_MIRROR },
        { "MIRROR_ONCE", TEXTURE_ADDRESS_MIRROR_ONCE },
    };
    std::unordered_map<std::string, RHI::SHADER_TYPE> shaderType_ {
        { "VERTEX", SHADER_TYPE_VERTEX },
        { "PIXEL", SHADER_TYPE_PIXEL },
        { "GEOMETRY", SHADER_TYPE_GEOMETRY },
        { "HULL", SHADER_TYPE_HULL },
        { "DOMAIN", SHADER_TYPE_DOMAIN },
        { "COMPUTE", SHADER_TYPE_COMPUTE },
        { "AMPLIFICATION", SHADER_TYPE_AMPLIFICATION },
        { "MESH", SHADER_TYPE_MESH },
    };

public:
    static const PsoEnumMap& Get()
    {
        static PsoEnumMap m;
        return m;
    }

#define DEFINE_GET_FUNC(funcName, mapVar, undefinedVar) \
    static auto funcName(const std::string& name)       \
    {                                                   \
        auto it = Get().mapVar.find(name);              \
        if (it == Get().mapVar.end()) {                 \
            return undefinedVar;                        \
        }                                               \
        return it->second;                              \
    }

    DEFINE_GET_FUNC(GetTopology, topology_, PRIMITIVE_TOPOLOGY_UNDEFINED)
    DEFINE_GET_FUNC(GetCullMode, cullMode_, CULL_MODE_UNDEFINED)
    DEFINE_GET_FUNC(GetTextureFormat, textureFormat_, TEX_FORMAT_UNKNOWN)
    DEFINE_GET_FUNC(GetComparisonFunc, comparisonFunc_, COMPARISON_FUNC_UNKNOWN)
    DEFINE_GET_FUNC(GetValueType, valueType_, VT_UNDEFINED)
    DEFINE_GET_FUNC(GetShaderResourceVarType, shaderResourceVarType_, SHADER_RESOURCE_VARIABLE_TYPE_NUM_TYPES)
    DEFINE_GET_FUNC(GetFilterType, filterType_, FILTER_TYPE_UNKNOWN)
    DEFINE_GET_FUNC(GetAddressMode, addressMode_, TEXTURE_ADDRESS_UNKNOWN)
    DEFINE_GET_FUNC(GetShaderType, shaderType_, SHADER_TYPE_UNKNOWN)
};

#define CHECK_ENUM_VALUE(var, node, getFunc, attrName, invalidValue)                                                                   \
    auto var = invalidValue;                                                                                                           \
    {                                                                                                                                  \
        const auto* val = node.attribute(attrName).value();                                                                            \
        var = PsoEnumMap::getFunc(val);                                                                                                \
        if (var == invalidValue) {                                                                                                     \
            LOONG_ERROR("Load PSO '{}' failed: Unknown '" attrName "' value '{}' for node '{}'", vfsPath, attrName, val, node.path()); \
            return {};                                                                                                                 \
        }                                                                                                                              \
    }                                                                                                                                  \
    do {                                                                                                                               \
    } while (false)

#define CHECK_TRUE_FALSE(var, node, attrName)                                                                                         \
    bool var = false;                                                                                                                 \
    {                                                                                                                                 \
        const char* val = node.attribute(attrName).value();                                                                           \
        var = strcasecmp(val, "true") == 0;                                                                                           \
        if (!var && strcasecmp(val, "false") != 0) {                                                                                  \
            LOONG_ERROR("Node {}'s attribute {}'s value is neither 'true' nor 'false', use false by default", node.path(), attrName); \
        }                                                                                                                             \
    }                                                                                                                                 \
    do {                                                                                                                              \
    } while (false)

#define CHECK_INT_VALUE(var, node, attrName)                                                                                                                  \
    int var = 0;                                                                                                                                              \
    {                                                                                                                                                         \
        const char* val = node.attribute(attrName).value();                                                                                                   \
        char* endPtr = nullptr;                                                                                                                               \
        var = strtol(val, &endPtr, 10);                                                                                                                       \
        if (endPtr[0] != '\0') {                                                                                                                              \
            LOONG_ERROR("Load PSO '{}' failed: attribute '" attrName "' value '{}' for node '{}' should be an integer", vfsPath, attrName, val, node.path()); \
            return {};                                                                                                                                        \
        }                                                                                                                                                     \
    }

static RHI::RefCntAutoPtr<RHI::IPipelineState> CreateGraphicsPipeline(const std::string& vfsPath, pugi::xml_node node, RHI::IRenderDevice& device)
{
    RHI::GraphicsPipelineStateCreateInfo psoCreateInfo;
    RHI::PipelineStateDesc& psoDesc = psoCreateInfo.PSODesc;

    psoDesc.Name = node.attribute("name").value();
    psoDesc.PipelineType = RHI::PIPELINE_TYPE_GRAPHICS;

    CHECK_ENUM_VALUE(primitiveTopology, node, GetTopology, "primitiveTopology", PRIMITIVE_TOPOLOGY_UNDEFINED);
    psoCreateInfo.GraphicsPipeline.PrimitiveTopology = primitiveTopology;

    CHECK_ENUM_VALUE(cullMode, node, GetCullMode, "cullMode", CULL_MODE_UNDEFINED);
    psoCreateInfo.GraphicsPipeline.RasterizerDesc.CullMode = cullMode;

    // <RenderTarget> tags
    if (auto renderTargets = node.children("RenderTarget"); true) {
        int numRenderTarget = 0;
        for (auto& renderTarget : renderTargets) {
            CHECK_ENUM_VALUE(textureFormat, renderTarget, GetTextureFormat, "format", TEX_FORMAT_UNKNOWN);
            psoCreateInfo.GraphicsPipeline.RTVFormats[numRenderTarget] = textureFormat;
            ++numRenderTarget;
        }
        if (numRenderTarget > 8) {
            LOONG_ERROR("Load PSO '{}' failed: too many 'RenderTarget's", vfsPath);
            return {};
        }
        psoCreateInfo.GraphicsPipeline.NumRenderTargets = numRenderTarget;
    }

    // <DepthStencil> tag
    if (auto depthStencil = node.child("DepthStencil"); depthStencil.empty()) {
        LOONG_ERROR("Load PSO '{}' failed: exactly one 'DepthStencil' tag is needed under 'GraphicsPipeline'", vfsPath);
        return {};
    } else {
        CHECK_ENUM_VALUE(depthStencilFormat, depthStencil, GetTextureFormat, "format", TEX_FORMAT_UNKNOWN);
        psoCreateInfo.GraphicsPipeline.DSVFormat = depthStencilFormat;

        // <DepthTest> tag
        auto depthTest = depthStencil.child("DepthTest");
        if (!depthTest.empty()) {
            CHECK_TRUE_FALSE(depthTestEnabled, depthTest, "enabled");
            CHECK_TRUE_FALSE(deptWriteEnabled, depthTest, "write");
            CHECK_ENUM_VALUE(cmpFunc, depthTest, GetComparisonFunc, "func", COMPARISON_FUNC_UNKNOWN);
            psoCreateInfo.GraphicsPipeline.DepthStencilDesc.DepthEnable = depthTestEnabled;
            psoCreateInfo.GraphicsPipeline.DepthStencilDesc.DepthWriteEnable = deptWriteEnabled;
            psoCreateInfo.GraphicsPipeline.DepthStencilDesc.DepthFunc = cmpFunc;
        }
        // <StencilTest> tag
        auto stencilTest = depthStencil.child("StencilTest");
        if (!stencilTest.empty()) {
            CHECK_TRUE_FALSE(enabled, stencilTest, "enabled");
            CHECK_INT_VALUE(readMask, stencilTest, "readMask");
            CHECK_INT_VALUE(writeMask, stencilTest, "writeMask");
            psoCreateInfo.GraphicsPipeline.DepthStencilDesc.StencilEnable = enabled;
            psoCreateInfo.GraphicsPipeline.DepthStencilDesc.StencilReadMask = readMask;
            psoCreateInfo.GraphicsPipeline.DepthStencilDesc.StencilWriteMask = writeMask;
        }
    }

    // <Shaders> tag
    std::vector<RHI::RefCntAutoPtr<RHI::IShader>> shaderObjects; // We need to keep them valid before pso created
    if (auto shaders = node.child("Shaders"); true) {
        struct ShaderSource {
            RHI::ShaderMacroHelper macros;
            std::string source;
        };
        auto CreateShaderDesc = [&vfsPath](pugi::xml_node shaderNode, RHI::ShaderCreateInfo& sci, ShaderSource& shaderSource) -> bool {
            if (auto attr = shaderNode.attribute("entry"); attr.empty() || strcmp(attr.value(), "") == 0) {
                LOONG_ERROR("Load PSO '{}' failed: missing 'entry' attribute for '{}'", vfsPath, shaderNode.path());
                return false;
            } else {
                sci.EntryPoint = attr.value();
            }

            sci.SourceLanguage = RHI::SHADER_SOURCE_LANGUAGE_HLSL;
            sci.UseCombinedTextureSamplers = true;
            sci.Desc.Name = shaderNode.attribute("name").value();

            if (auto srcFile = shaderNode.attribute("sourceFile"); !srcFile.empty() && strcmp(srcFile.value(), "") != 0) {
                bool suc = false;
                shaderSource.source = FS::LoongFileSystem::LoadFileContentAsString(srcFile.value(), suc);
                if (!suc) {
                    LOONG_ERROR("Load PSO '{}' failed: load source file '{}' failed", vfsPath, srcFile.value());
                    return false;
                }
                sci.Source = shaderSource.source.c_str();
            } else {
                // TODO: Try to get source from other attribute
                LOONG_ERROR("Load PSO '{}' failed: missing shader source ('sourceFile' attribute) for '{}'", vfsPath, shaderNode.path());
                return false;
            }

            auto definitions = shaderNode.children("Definition");
            if (definitions.begin() != definitions.end()) {
                RHI::ShaderMacroHelper& psMacros = shaderSource.macros;
                for (auto def : definitions) {
                    const char* name = def.attribute("name").value();
                    if (strcmp(name, "") == 0) {
                        LOONG_ERROR("Load PSO '{}' failed: empty definition 'name' for '{}'", vfsPath, shaderNode.path());
                        return false;
                    }
                    const char* value = def.attribute("value").value();
                    if (strcmp(value, "") == 0) {
                        LOONG_ERROR("Load PSO '{}' failed: empty definition 'value' for '{}'", vfsPath, shaderNode.path());
                        return false;
                    }
                    psMacros.AddShaderMacro(name, value);
                }
                sci.Macros = psMacros;
            }
            return true;
        };

        for (auto shaderNode : shaders) {
            RHI::ShaderCreateInfo sci {};
            ShaderSource shaderSource; // we need to keep this object alive before shader is created
            if (!CreateShaderDesc(shaderNode, sci, shaderSource)) {
                return {};
            }
            if (strcmp(shaderNode.name(), "PS") == 0) {
                // Pixel shader
                sci.Desc.ShaderType = RHI::SHADER_TYPE_PIXEL;
            } else if (strcmp(shaderNode.name(), "VS") == 0) {
                // Vertex shader
                sci.Desc.ShaderType = RHI::SHADER_TYPE_VERTEX;
            } else {
                LOONG_WARNING("Shader node '{}' of type '{}' is not supported yet", shaderNode.path(), shaderNode.name());
                continue;
            }

            RHI::RefCntAutoPtr<RHI::IShader> pShader;
            device.CreateShader(sci, &pShader);
            if (pShader == nullptr) {
                LOONG_ERROR("Load PSO '{}' failed: create shader for '{}' failed", vfsPath, shaderNode.path());
                return {};
            }
            shaderObjects.push_back(pShader);

            if (strcmp(shaderNode.name(), "PS") == 0) {
                // Pixel shader
                psoCreateInfo.pPS = pShader;
            } else if (strcmp(shaderNode.name(), "VS") == 0) {
                // Vertex shader
                psoCreateInfo.pVS = pShader;
            } else {
                LOONG_ASSERT(false, "Impossible");
            }
        }
    }

    // <InputLayout> tag
    RHI::InputLayoutDesc& inputLayout = psoCreateInfo.GraphicsPipeline.InputLayout;
    std::vector<RHI::LayoutElement> inputLayoutElements;
    if (auto elementNodes = node.child("InputLayout").children("Element"); elementNodes.begin() == elementNodes.end()) {
        LOONG_ERROR("Load PSO '{}' failed: empty input layout", vfsPath);
        return {};
    } else {
        for (auto eleNode : elementNodes) {
            CHECK_INT_VALUE(index, eleNode, "index");
            CHECK_INT_VALUE(slot, eleNode, "slot");
            CHECK_INT_VALUE(numComp, eleNode, "numComp");
            CHECK_ENUM_VALUE(valueType, eleNode, GetValueType, "valueType", VT_UNDEFINED);
            CHECK_TRUE_FALSE(normalized, eleNode, "normalized");
            inputLayoutElements.emplace_back((uint32_t)index, (uint32_t)slot, (uint32_t)numComp, valueType, normalized);
        }
        inputLayout.NumElements = (uint32_t)inputLayoutElements.size();
        inputLayout.LayoutElements = inputLayoutElements.data();
    }

    // <ResourceLayout> tag
    RHI::PipelineResourceLayoutDesc& resourceLayout = psoDesc.ResourceLayout;
    resourceLayout.DefaultVariableType = RHI::SHADER_RESOURCE_VARIABLE_TYPE_STATIC;
    std::vector<RHI::ShaderResourceVariableDesc> shaderVariables;
    std::vector<RHI::ImmutableSamplerDesc> shaderSamplers;
    if (auto elementNodes = node.child("ResourceLayout"); true) {
        for (auto eleNode : elementNodes) {
            CHECK_ENUM_VALUE(shaderType, eleNode, GetShaderType, "shaderType", SHADER_TYPE_UNKNOWN);
            CHECK_ENUM_VALUE(type, eleNode, GetShaderResourceVarType, "type", SHADER_RESOURCE_VARIABLE_TYPE_NUM_TYPES);
            CHECK_ENUM_VALUE(minFilter, eleNode, GetFilterType, "minFilter", FILTER_TYPE_UNKNOWN);
            CHECK_ENUM_VALUE(magFilter, eleNode, GetFilterType, "magFilter", FILTER_TYPE_UNKNOWN);
            CHECK_ENUM_VALUE(mipFilter, eleNode, GetFilterType, "mipFilter", FILTER_TYPE_UNKNOWN);
            CHECK_ENUM_VALUE(addrU, eleNode, GetAddressMode, "addrU", TEXTURE_ADDRESS_UNKNOWN);
            CHECK_ENUM_VALUE(addrV, eleNode, GetAddressMode, "addrW", TEXTURE_ADDRESS_UNKNOWN);
            CHECK_ENUM_VALUE(addrW, eleNode, GetAddressMode, "addrW", TEXTURE_ADDRESS_UNKNOWN);

            const char* name = eleNode.attribute("name").value();
            if (strcmp(name, "") == 0) {
                LOONG_ERROR("Load PSO '{}' failed: empty shader resource name for node '{}'", vfsPath, eleNode.path());
                return {};
            }
            shaderVariables.emplace_back(shaderType, name, type);
            shaderSamplers.push_back({ shaderType, name, { minFilter, magFilter, mipFilter, addrU, addrV, addrW } });
        }
    }
    resourceLayout.Variables = shaderVariables.data();
    resourceLayout.NumVariables = uint32_t(shaderVariables.size());
    resourceLayout.ImmutableSamplers = shaderSamplers.data();
    resourceLayout.NumImmutableSamplers = uint32_t(shaderSamplers.size());

    RHI::RefCntAutoPtr<RHI::IPipelineState> pso;
    device.CreateGraphicsPipelineState(psoCreateInfo, &pso);
    if (pso == nullptr) {
        LOONG_ERROR("Load PSO '{}' failed: create failed", vfsPath);
    }
    return pso;
}

RHI::RefCntAutoPtr<RHI::IPipelineState> LoongRHIManager::LoadPSO(const std::string& vfsPath)
{
    bool succeed;
    auto buffer = FS::LoongFileSystem::LoadFileContent(vfsPath, succeed);
    if (!succeed) {
        LOONG_ERROR("Load PSO '{}' failed: load file failed", vfsPath);
        return {};
    }

    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_buffer(buffer.data(), buffer.size());
    if (!result) {
        LOONG_ERROR("Load PSO failed: '{}' failed: parse xml failed", vfsPath);
        return {};
    }

    auto node = doc.root().first_child();
    if (node.empty()) {
        LOONG_ERROR("Load PSO failed: '{}' failed: empty PSO file", vfsPath);
        return {};
    }
    constexpr const char* kGraphicsPipeline = "GraphicsPipeline";
    if (strcmp(node.name(), kGraphicsPipeline) == 0) {
        auto& device = *gRhiImpl.device_;
        return CreateGraphicsPipeline(vfsPath, node, device);
    }
    LOONG_ERROR("Load PSO failed: '{}' failed: pipeline object of which the type is not '{}' is not supported yet", vfsPath, kGraphicsPipeline);
    return {};
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