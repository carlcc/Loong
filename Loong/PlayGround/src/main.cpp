#include "LoongFoundation/LoongClock.h"
#include "LoongFoundation/LoongLogger.h"
#include "LoongRHI/Driver.h"
#include "LoongRHI/LoongRHIManager.h"
#include "LoongWindow/Driver.h"
#include "LoongWindow/LoongApplication.h"
#include "LoongWindow/LoongWindow.h"
#include <GLFW/glfw3.h>
#include <GraphicsUtilities.h>
#include <LoongAsset/LoongMesh.h>
#include <LoongAsset/LoongModel.h>
#include <LoongFileSystem/Driver.h>
#include <LoongFileSystem/LoongFileSystem.h>
#include <LoongFoundation/Driver.h>
#include <LoongFoundation/LoongAssert.h>
#include <LoongFoundation/LoongPathUtils.h>
#include <LoongFoundation/LoongThreadPool.h>
#include <LoongResource/Driver.h>
#include <LoongResource/LoongGpuMesh.h>
#include <LoongResource/LoongGpuModel.h>
#include <LoongResource/LoongMaterial.h>
#include <LoongResource/LoongResourceManager.h>
#include <LoongResource/LoongTexture.h>
#include <LoongResource/loader/LoongMaterialLoader.h>
#include <cassert>
#include <iostream>

namespace Loong {

struct UniformConstants {
    RHI::float4x4 ub_MVP;
    RHI::float4x4 ub_Model;
    RHI::float4x4 ub_View;
    RHI::float4x4 ub_Projection;
    RHI::float3 ub_ViewPos;
    float ub_Time;
};

static const char* VSSource = R"(
cbuffer Constants
{
    float4x4 ub_MVP;
    float4x4 ub_Model;
    float4x4 ub_View;
    float4x4 ub_Projection;
    float3   ub_ViewPos;
    float    ub_Time;
};

struct VSInput
{
    float3 v_Pos    : ATTRIB0;
    float2 v_Uv     : ATTRIB1;
    float3 v_Normal : ATTRIB2;
    float3 v_Tan    : ATTRIB3;
    float3 v_BiTan  : ATTRIB4;
};

struct PSInput
{
    float4 Pos          : SV_POSITION;
    float2 Uv           : TEX_COORD;
    // float3 WorldNormal  : NORMAL;
    float4 WorldPos     : POSITIONT;
    //float3 CameraPos    ;
    //float3x3 TBN        ;
};

void main(in  VSInput VSIn,
          out PSInput PSIn)
{
    float4 pos = float4(VSIn.v_Pos,1.0);

    PSIn.Pos = mul(pos, ub_MVP);
    PSIn.Uv  = VSIn.v_Uv;
    PSIn.WorldPos = mul(pos, ub_Model);
    //PSIn.CameraPos = ub_ViewPos;

    //float3 T = normalize(float3(ub_Model * float4(VSIn.v_Tan,   0.0)));
    //float3 B = normalize(float3(ub_Model * float4(VSIn.v_BiTan, 0.0)));
    //float3 N = normalize(float3(ub_Model * float4(VSIn.v_Normal,0.0)));
    //PSIn.TBN = float3x3(T, B, N);
}
)";

// Pixel shader simply outputs interpolated vertex color
static const char* PSSource = R"(
Texture2D    g_Albedo;
SamplerState g_Albedo_sampler;

struct PSInput
{
    float4 Pos          : SV_POSITION;
    float2 Uv           : TEX_COORD;
    float3 WorldNormal  : NORMAL;
    float4 WorldPos     : POSITIONT;
    //float3 CameraPos    ;
    //float3x3 TBN        ;
};

struct PSOutput
{
    float4 Color : SV_TARGET;
};

void main(in  PSInput  PSIn,
          out PSOutput PSOut)
{
    PSOut.Color = g_Albedo.Sample(g_Albedo_sampler, PSIn.Uv);
}
)";

class LoongEditor : public Foundation::LoongHasSlots {
public:
    std::shared_ptr<Resource::LoongGpuModel> model_ { nullptr };
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
            RHI::LayoutElement { 2, 0, 3, RHI::VT_FLOAT32, false },
            RHI::LayoutElement { 3, 0, 3, RHI::VT_FLOAT32, false },
            RHI::LayoutElement { 4, 0, 3, RHI::VT_FLOAT32, false },
        };
        RHI::InputLayoutDesc inputLayout {};
        inputLayout.LayoutElements = layoutElements;
        inputLayout.NumElements = _countof(layoutElements);

        RHI::PipelineResourceLayoutDesc resourceLayout;
        resourceLayout.DefaultVariableType = RHI::SHADER_RESOURCE_VARIABLE_TYPE_STATIC;
        RHI::ShaderResourceVariableDesc shaderVariables[] {
            { RHI::SHADER_TYPE_PIXEL, "g_Albedo", RHI::SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE }
        };

        RHI::SamplerDesc samplerConfig {
            RHI::FILTER_TYPE_LINEAR, RHI::FILTER_TYPE_LINEAR, RHI::FILTER_TYPE_LINEAR,
            RHI::TEXTURE_ADDRESS_WRAP, RHI::TEXTURE_ADDRESS_WRAP, RHI::TEXTURE_ADDRESS_WRAP
        };
        RHI::StaticSamplerDesc shaderSamplers[] {
            { RHI::SHADER_TYPE_PIXEL, "g_Albedo", samplerConfig }
        };
        resourceLayout.Variables = shaderVariables;
        resourceLayout.NumVariables = _countof(shaderVariables);
        resourceLayout.StaticSamplers = shaderSamplers;
        resourceLayout.NumStaticSamplers = _countof(shaderSamplers);

        pso_ = RHI::LoongRHIManager::CreateGraphicsPSOForCurrentSwapChain(swapChain_, "TexturedCube", vs, ps, inputLayout, resourceLayout, true, Diligent::CULL_MODE_BACK);
        vsConstants_ = RHI::LoongRHIManager::CreateUniformBuffer("VS constants CB", sizeof(UniformConstants));
        pso_->GetStaticVariableByName(RHI::SHADER_TYPE_VERTEX, "Constants")->Set(vsConstants_);
        pso_->CreateShaderResourceBinding(&srb_, true);

        InitResources();
        return true;
    }

    void InitResources()
    {
        Foundation::LoongThreadPool::AddTask([&]() {
            LOONG_ASSERT(!Window::LoongApplication::IsInMainThread(), "");
            LOONG_INFO("Loading texture...");
            auto mtl = Resource::LoongResourceManager::GetMaterial("/Materials/Test.lgmtl");
            texture_ = mtl->GetAlbedoMap();

            Window::LoongApplication::RunInMainThread([&]() {
                LOONG_ASSERT(Window::LoongApplication::IsInMainThread(), "");
                textureSRV_ = texture_->GetTexture()->GetDefaultView(RHI::TEXTURE_VIEW_SHADER_RESOURCE);

                srb_->GetVariableByName(RHI::SHADER_TYPE_PIXEL, "g_Albedo")->Set(textureSRV_);

                model_ = Resource::LoongResourceManager::GetModel("/Models/DamagedHelmet.lgmdl");
            });
            return true;
        });
    }

    void OnUpdate();

    void OnRender()
    {
        LOONG_ASSERT(Window::LoongApplication::IsInMainThread(), "");
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

        if (texture_ == nullptr || model_ == nullptr) {
            return;
        }

        {
            // Map the buffer and write current world-view-projection matrix
            RHI::MapHelper<UniformConstants> uniforms(immediateContext, vsConstants_, RHI::MAP_WRITE, RHI::MAP_FLAG_DISCARD);
            uniforms->ub_View = uniforms_.ub_View.Transpose();
            uniforms->ub_Projection = uniforms_.ub_Projection.Transpose();
            uniforms->ub_Model = uniforms_.ub_Model.Transpose();
            uniforms->ub_MVP = uniforms_.ub_MVP.Transpose();
            uniforms->ub_ViewPos = uniforms_.ub_ViewPos;
            uniforms->ub_Time = uniforms_.ub_Time;
        }

        immediateContext->SetPipelineState(pso_);
        immediateContext->CommitShaderResources(srb_, RHI::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
        uint32_t offset = 0;
        for (auto* mesh : model_->GetMeshes()) {
            RHI::IBuffer* buffers[] = { mesh->GetVBO() };
            immediateContext->SetVertexBuffers(0, 1, buffers, &offset, RHI::RESOURCE_STATE_TRANSITION_MODE_TRANSITION, RHI::SET_VERTEX_BUFFERS_FLAG_RESET);
            immediateContext->SetIndexBuffer(mesh->GetIBO(), 0, RHI::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

            RHI::DrawIndexedAttribs drawAttrs;

            drawAttrs.IndexType = RHI::VT_UINT32;
            drawAttrs.NumIndices = mesh->GetIndexCount();
            drawAttrs.Flags = Diligent::DRAW_FLAG_VERIFY_ALL;
            immediateContext->DrawIndexed(drawAttrs);
        }
    }

    void OnPresent()
    {
        bool vsync = false;
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
        Window::LoongApplication::DestroyAllWindows();
    }

    Foundation::LoongClock clock_ {};
    RHI::RefCntAutoPtr<RHI::IPipelineState> pso_ { nullptr };
    RHI::RefCntAutoPtr<RHI::IBuffer> vsConstants_ { nullptr };
    RHI::RefCntAutoPtr<RHI::IShaderResourceBinding> srb_ { nullptr };
    std::shared_ptr<Resource::LoongTexture> texture_ { nullptr };
    RHI::RefCntAutoPtr<RHI::ITextureView> textureSRV_ { nullptr };
    RHI::float4x4 worldViewProjMatrix_ {};
    int frameBufferWidth_ { 0 };
    int frameBufferHeight_ { 0 };
    float frameBufferAspect_ { 1.0F };
    RHI::RefCntAutoPtr<RHI::ISwapChain> swapChain_ { nullptr };
    float clearColor_[4] { 0.350f, 0.350f, 0.350f, 1.0f };

    UniformConstants uniforms_ {};
};

class LoongEditor2 : public LoongEditor {
public:
    void OnClose1() override
    {
        Window::LoongApplication::DestroyWindow(window_);
    }
};

void LoongEditor::OnUpdate()
{
    LOONG_ASSERT(Window::LoongApplication::IsInMainThread(), "");
    if (window_->GetInputManager().IsKeyReleaseEvent(Window::LoongKeyCode::kKeyN)) {
        auto* ed = new LoongEditor2;
        auto* win = Window::LoongApplication::CreateWindow({}, [ed](auto* w) {
            delete ed;
        });
        ed->Initialize(win, RHI::LoongRHIManager::CreateSwapChain(win->GetGlfwWindow()));
        ed->clearColor_[0] = (1.0F + sin(clock_.ElapsedTime())) / 2.0F;
        ed->clearColor_[1] = (1.0F + sin(clock_.ElapsedTime() * 1.3f)) / 2.0F;
        ed->clearColor_[2] = (1.0F + sin(clock_.ElapsedTime() * 1.5f)) / 2.0F;
    }
    if (window_->GetInputManager().IsKeyReleaseEvent(Window::LoongKeyCode::kKeyM)) {
        auto* mat = new Resource::LoongMaterial;
        Resource::LoongMaterialLoader::Write("Materials/test.lgmtl", mat);
    }
    clock_.Update();
    using float4x4 = RHI::float4x4;

    uniforms_.ub_Model = float4x4::RotationY(clock_.ElapsedTime()) * float4x4::RotationX(-RHI::PI_F * 0.1f);
    // Camera is at (0, 0, -4) looking along the Z axis
    uniforms_.ub_View = float4x4::Translation(0.f, 0.0f, 4.0f);
    uniforms_.ub_Projection = float4x4::Projection(RHI::PI_F / 4.0f, frameBufferAspect_, 0.001f, 1000.f, false);
    uniforms_.ub_ViewPos = RHI::float3 { 0, 0, -4 };
    uniforms_.ub_MVP = uniforms_.ub_Model * uniforms_.ub_View * uniforms_.ub_Projection;
    uniforms_.ub_Time = clock_.ElapsedTime();
}
}

void StartApp(int argc, char** argv)
{
    Loong::Foundation::ScopedDriver foundationDriver;
    Loong::FS::ScopedDriver fsDriver(argv[0]);
    auto path = Loong::Foundation::LoongPathUtils::GetParent(argv[0]) + "/Resources";
    Loong::FS::LoongFileSystem::MountSearchPath(path);
    path = Loong::Foundation::LoongPathUtils::Normalize(argv[0]) + "/../../Resources";
    Loong::FS::LoongFileSystem::MountSearchPath(path);
    path = "/Users/chenchen02/gitrepo/Loong/Resources";
    Loong::FS::LoongFileSystem::SetWriteDir(path);

    Loong::Window::ScopedDriver appDriver;
    assert(bool(appDriver));

    Loong::Window::WindowConfig config {};
    config.title = "Play Ground";
    auto window = Loong::Window::LoongApplication::CreateWindow(config);

    Loong::RHI::ScopedDriver rhiDriver(window->GetGlfwWindow(), Loong::RHI::RENDER_DEVICE_TYPE_VULKAN);
    assert(bool(rhiDriver));

    Loong::Resource::ScopedDriver resourceDriver;
    assert(bool(resourceDriver));

    auto* ed = new Loong::LoongEditor;
    ed->Initialize(window, Loong::RHI::LoongRHIManager::GetPrimarySwapChain());
    Loong::Window::LoongApplication::SetDeleterForWindow(window, [ed](auto* w) {
        delete ed;
    });

    Loong::Window::LoongApplication::Run();

    Loong::RHI::LoongRHIManager::Uninitialize();
}

int main(int argc, char** argv)
{

    auto listener = Loong::Foundation::Logger::Get().SubscribeLog([](const Loong::Foundation::LogItem& logItem) {
        std::cout << "[" << logItem.level << "]: " << logItem.message << " (" << logItem.location << ")" << std::endl;
    });

    StartApp(argc, argv);

    return 0;
}
