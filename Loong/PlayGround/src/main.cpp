#include "LoongFoundation/LoongClock.h"
#include "LoongFoundation/LoongLogger.h"
#include "LoongRHI/Driver.h"
#include "LoongRHI/LoongRHIManager.h"
#include "LoongWindow/Driver.h"
#include "LoongWindow/LoongWindow.h"
#include "LoongWindow/LoongWindowManager.h"
#include <GLFW/glfw3.h>
#include <GraphicsUtilities.h>
#include <cassert>
#include <iostream>

namespace Loong {

static const char* VSSource = R"(
cbuffer Constants
{
    float4x4 g_WorldViewProj;
};

struct VSInput
{
    float3 Pos : ATTRIB0;
    float2 UV  : ATTRIB1;
};

struct PSInput
{
    float4 Pos : SV_POSITION;
    float2 UV  : TEX_COORD;
};

void main(in  VSInput VSIn,
          out PSInput PSIn)
{
    PSIn.Pos = mul( float4(VSIn.Pos,1.0), g_WorldViewProj);
    PSIn.UV  = VSIn.UV;
}
)";

// Pixel shader simply outputs interpolated vertex color
static const char* PSSource = R"(
Texture2D    g_Texture;
SamplerState g_Texture_sampler;

struct PSInput
{
    float4 Pos : SV_POSITION;
    float2 UV : TEX_COORD;
};

struct PSOutput
{
    float4 Color : SV_TARGET;
};

void main(in  PSInput  PSIn,
          out PSOutput PSOut)
{
    PSOut.Color = g_Texture.Sample(g_Texture_sampler, PSIn.UV);
}
)";

class LoongEditor : public Foundation::LoongHasSlots {
public:
    Window::LoongWindow* window_ { nullptr };
    bool Initialize(Window::LoongWindow* window, RHI::RefCntAutoPtr<RHI::ISwapChain> swapChain)
    {
        window_ = window;

        swapChain_ = swapChain;
        window->SubscribeUpdate(this, &LoongEditor::OnUpdate);
        window->SubscribeRender(this, &LoongEditor::OnRender);
        window->SubscribePresent(this, &LoongEditor::OnPresent);
        window->SubscribeFrameBufferResize(this, &LoongEditor::OnFrameBufferResize);
        window->SubscribeWindowClose(this, &LoongEditor::OnClose);
        window->GetFramebufferSize(frameBufferWidth_, frameBufferHeight_);
        OnFrameBufferResize(frameBufferWidth_, frameBufferHeight_);

        clock_.Reset();

        RHI::ShaderCreateInfo vs;
        vs.SourceLanguage = RHI::SHADER_SOURCE_LANGUAGE_HLSL;
        // OpenGL backend requires emulated combined HLSL texture samplers (g_Texture + g_Texture_sampler combination)
        vs.UseCombinedTextureSamplers = true;
        vs.Desc.ShaderType = RHI::SHADER_TYPE_VERTEX;
        vs.EntryPoint = "main";
        vs.Desc.Name = "Cube vertex shader";
        vs.Source = VSSource;

        RHI::ShaderCreateInfo ps;
        ps.SourceLanguage = RHI::SHADER_SOURCE_LANGUAGE_HLSL;
        ps.UseCombinedTextureSamplers = true;
        ps.Desc.ShaderType = RHI::SHADER_TYPE_PIXEL;
        ps.EntryPoint = "main";
        ps.Desc.Name = "Cube pixel shader";
        ps.Source = PSSource;

        RHI::LayoutElement layoutElements[] {
            RHI::LayoutElement { 0, 0, 3, RHI::VT_FLOAT32, false },
            RHI::LayoutElement { 1, 0, 2, RHI::VT_FLOAT32, false },
        };
        RHI::InputLayoutDesc inputLayout {};
        inputLayout.LayoutElements = layoutElements;
        inputLayout.NumElements = _countof(layoutElements);

        RHI::PipelineResourceLayoutDesc resourceLayout;
        resourceLayout.DefaultVariableType = RHI::SHADER_RESOURCE_VARIABLE_TYPE_STATIC;
        RHI::ShaderResourceVariableDesc shaderVariables[] {
            { RHI::SHADER_TYPE_PIXEL, "g_Texture", RHI::SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE }
        };

        RHI::SamplerDesc samplerConfig {
            RHI::FILTER_TYPE_LINEAR, RHI::FILTER_TYPE_LINEAR, RHI::FILTER_TYPE_LINEAR,
            RHI::TEXTURE_ADDRESS_CLAMP, RHI::TEXTURE_ADDRESS_CLAMP, RHI::TEXTURE_ADDRESS_CLAMP
        };
        RHI::StaticSamplerDesc shaderSamplers[] {
            { RHI::SHADER_TYPE_PIXEL, "g_Texture", samplerConfig }
        };
        resourceLayout.Variables = shaderVariables;
        resourceLayout.NumVariables = _countof(shaderVariables);
        resourceLayout.StaticSamplers = shaderSamplers;
        resourceLayout.NumStaticSamplers = _countof(shaderSamplers);

        pso_ = RHI::LoongRHIManager::CreateGraphicsPSOForCurrentSwapChain(swapChain_, "TexturedCube", vs, ps, inputLayout, resourceLayout, true, Diligent::CULL_MODE_NONE);
        vsConstants_ = RHI::LoongRHIManager::CreateUniformBuffer("VS constants CB", sizeof(RHI::float4x4));
        pso_->GetStaticVariableByName(RHI::SHADER_TYPE_VERTEX, "Constants")->Set(vsConstants_);
        pso_->CreateShaderResourceBinding(&srb_, true);

        InitResources();

        return true;
    }

    void InitResources()
    {
        using float3 = RHI::float3;
        using float2 = RHI::float2;
        struct Vertex {
            float3 pos;
            float2 uv;
        };

        // Cube vertices
        //      (-1,+1,+1)________________(+1,+1,+1)
        //               /|              /|
        //              / |             / |
        //             /  |            /  |
        //            /   |           /   |
        //(-1,-1,+1) /____|__________/(+1,-1,+1)
        //           |    |__________|____|
        //           |   /(-1,+1,-1) |    /(+1,+1,-1)
        //           |  /            |   /
        //           | /             |  /
        //           |/              | /
        //           /_______________|/
        //        (-1,-1,-1)       (+1,-1,-1)
        //
        // This time we have to duplicate verices because texture coordinates cannot
        // be shared
        Vertex kCubeVerts[] = {
            { float3(-1, -1, -1), float2(0, 1) },
            { float3(-1, +1, -1), float2(0, 0) },
            { float3(+1, +1, -1), float2(1, 0) },
            { float3(+1, -1, -1), float2(1, 1) },

            { float3(-1, -1, -1), float2(0, 1) },
            { float3(-1, -1, +1), float2(0, 0) },
            { float3(+1, -1, +1), float2(1, 0) },
            { float3(+1, -1, -1), float2(1, 1) },

            { float3(+1, -1, -1), float2(0, 1) },
            { float3(+1, -1, +1), float2(1, 1) },
            { float3(+1, +1, +1), float2(1, 0) },
            { float3(+1, +1, -1), float2(0, 0) },

            { float3(+1, +1, -1), float2(0, 1) },
            { float3(+1, +1, +1), float2(0, 0) },
            { float3(-1, +1, +1), float2(1, 0) },
            { float3(-1, +1, -1), float2(1, 1) },

            { float3(-1, +1, -1), float2(1, 0) },
            { float3(-1, +1, +1), float2(0, 0) },
            { float3(-1, -1, +1), float2(0, 1) },
            { float3(-1, -1, -1), float2(1, 1) },

            { float3(-1, -1, +1), float2(1, 1) },
            { float3(+1, -1, +1), float2(0, 1) },
            { float3(+1, +1, +1), float2(0, 0) },
            { float3(-1, +1, +1), float2(1, 0) }
        };

        uint32_t kIndices[] = {
            2, 0, 1, 2, 3, 0,
            4, 6, 5, 4, 7, 6,
            8, 10, 9, 8, 11, 10,
            12, 14, 13, 12, 15, 14,
            16, 18, 17, 16, 19, 18,
            20, 21, 22, 20, 22, 23
        };

        cubeVertexBuffer_ = RHI::LoongRHIManager::CreateVertexBuffer("Cube vertex buffer", sizeof(kCubeVerts), kCubeVerts);
        cubeIndexBuffer_ = RHI::LoongRHIManager::CreateIndexBuffer("Cube index buffer", sizeof(kIndices), kIndices);
        texture_ = RHI::LoongRHIManager::CreateTextureFromFile("Resources/Textures/Loong.jpg", true);
        textureSRV_ = texture_->GetDefaultView(RHI::TEXTURE_VIEW_SHADER_RESOURCE);

        srb_->GetVariableByName(RHI::SHADER_TYPE_PIXEL, "g_Texture")->Set(textureSRV_);
    }

    void OnUpdate();

    void OnRender()
    {
        auto immediateContext = RHI::LoongRHIManager::GetImmediateContext();
        auto swapChain = swapChain_;

        assert(immediateContext != nullptr);
        assert(swapChain != nullptr);

        RHI::ITextureView* pRTV = swapChain->GetCurrentBackBufferRTV();
        RHI::ITextureView* pDSV = swapChain->GetDepthBufferDSV();
        immediateContext->SetRenderTargets(1, &pRTV, pDSV, RHI::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

        // Let the engine perform required state transitions
        immediateContext->ClearRenderTarget(pRTV, clearColor_, RHI::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
        immediateContext->ClearDepthStencil(pDSV, RHI::CLEAR_DEPTH_FLAG, 1.f, 0, RHI::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

        {
            // Map the buffer and write current world-view-projection matrix
            RHI::MapHelper<RHI::float4x4> cbConstants(immediateContext, vsConstants_, RHI::MAP_WRITE, RHI::MAP_FLAG_DISCARD);
            *cbConstants = worldViewProjMatrix_.Transpose();
        }

        uint32_t offset = 0;
        RHI::IBuffer* buffers[] = { cubeVertexBuffer_ };
        immediateContext->SetVertexBuffers(0, 1, buffers, &offset, RHI::RESOURCE_STATE_TRANSITION_MODE_TRANSITION, RHI::SET_VERTEX_BUFFERS_FLAG_RESET);
        immediateContext->SetIndexBuffer(cubeIndexBuffer_, 0, RHI::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

        immediateContext->SetPipelineState(pso_);
        immediateContext->CommitShaderResources(srb_, RHI::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

        RHI::DrawIndexedAttribs drawAttrs;

        drawAttrs.IndexType = RHI::VT_UINT32;
        drawAttrs.NumIndices = 36;
        drawAttrs.Flags = Diligent::DRAW_FLAG_VERIFY_ALL;
        immediateContext->DrawIndexed(drawAttrs);
    }

    void OnPresent()
    {
        bool vsync = true;
        swapChain_->Present(vsync ? 1 : 0);
    }

    void OnFrameBufferResize(int w, int h)
    {
        frameBufferWidth_ = w;
        frameBufferHeight_ = h;
        frameBufferAspect_ = (float)w / (float)h;
        swapChain_->Resize(w, h);
    }

    void OnClose()
    {
        OnClose1();
    }

    virtual void OnClose1()
    {
        Window::LoongWindowManager::DestroyAllWindows();
    }

    Foundation::LoongClock clock_ {};
    RHI::RefCntAutoPtr<RHI::IPipelineState> pso_ { nullptr };
    RHI::RefCntAutoPtr<RHI::IBuffer> vsConstants_ { nullptr };
    RHI::RefCntAutoPtr<RHI::IBuffer> cubeVertexBuffer_ { nullptr };
    RHI::RefCntAutoPtr<RHI::IBuffer> cubeIndexBuffer_ { nullptr };
    RHI::RefCntAutoPtr<RHI::IShaderResourceBinding> srb_ { nullptr };
    RHI::RefCntAutoPtr<RHI::ITexture> texture_ { nullptr };
    RHI::RefCntAutoPtr<RHI::ITextureView> textureSRV_ { nullptr };
    RHI::float4x4 worldViewProjMatrix_ {};
    int frameBufferWidth_ { 0 };
    int frameBufferHeight_ { 0 };
    float frameBufferAspect_ { 1.0F };
    RHI::RefCntAutoPtr<RHI::ISwapChain> swapChain_ { nullptr };
    float clearColor_[4] { 0.350f, 0.350f, 0.350f, 1.0f };
};

class LoongEditor2 : public LoongEditor {
public:
    void OnClose1() override
    {
        Window::LoongWindowManager::DestroyWindow(window_);
    }
};

void LoongEditor::OnUpdate()
{
    if (window_->GetInputManager().IsKeyReleaseEvent(Window::LoongKeyCode::kKeyN)) {
        auto* ed = new LoongEditor2;
        auto* win = Window::LoongWindowManager::CreateWindow({}, [ed](auto* w) {
            delete ed;
        });
        ed->Initialize(win, RHI::LoongRHIManager::CreateSwapChain(win->GetGlfwWindow()));
        ed->clearColor_[0] = (1.0F + sin(clock_.ElapsedTime())) / 2.0F;
        ed->clearColor_[1] = (1.0F + sin(clock_.ElapsedTime() * 1.3f)) / 2.0F;
        ed->clearColor_[2] = (1.0F + sin(clock_.ElapsedTime() * 1.5f)) / 2.0F;
    }
    clock_.Update();
    using float4x4 = RHI::float4x4;

    float4x4 cubeModelTransform = float4x4::RotationY(clock_.ElapsedTime()) * float4x4::RotationX(-RHI::PI_F * 0.1f);
    // Camera is at (0, 0, -5) looking along the Z axis
    float4x4 view = float4x4::Translation(0.f, 0.0f, 5.0f);
    float4x4 proj = float4x4::Projection(RHI::PI_F / 4.0f, frameBufferAspect_, 0.001f, 1000.f, false);
    worldViewProjMatrix_ = cubeModelTransform * view * proj;
}

}

void StartApp()
{
    Loong::Window::ScopedDriver appDriver;
    assert(appDriver);

    Loong::Window::WindowConfig config {};
    config.title = "Play Ground";
    auto window = Loong::Window::LoongWindowManager::CreateWindow(config);

    Loong::RHI::ScopedDriver rhiDriver(window->GetGlfwWindow(), Loong::RHI::RENDER_DEVICE_TYPE_VULKAN);
    assert(rhiDriver);

    auto* ed = new Loong::LoongEditor;
    ed->Initialize(window, Loong::RHI::LoongRHIManager::GetPrimarySwapChain());
    Loong::Window::LoongWindowManager::SetDeleterForWindow(window, [ed](auto* w) {
        delete ed;
    });

    Loong::Window::LoongWindowManager::Run();

    Loong::RHI::LoongRHIManager::Uninitialize();
}

int main(int argc, char** argv)
{

    auto listener = Loong::Foundation::Logger::Get().SubscribeLog([](const Loong::Foundation::LogItem& logItem) {
        std::cout << "[" << logItem.level << "][" << logItem.location << "]: " << logItem.message << std::endl;
    });

    StartApp();

    return 0;
}