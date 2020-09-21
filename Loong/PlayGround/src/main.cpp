#include <GLFW/glfw3.h>

#include "LoongFoundation/LoongLogger.h"
#include "LoongRHI/Driver.h"
#include "LoongRHI/LoongRHIManager.h"
#include "LoongWindow/Driver.h"
#include "LoongWindow/LoongWindow.h"
#include <cassert>
#include <iostream>

std::shared_ptr<Loong::Window::LoongWindow> gWindow;

namespace Loong {

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

class LoongEditor : public Foundation::LoongHasSlots {
public:
    LoongEditor()
    {
        gWindow->SubscribeUpdate(this, &LoongEditor::OnUpdate);
        gWindow->SubscribeRender(this, &LoongEditor::OnRender);
        gWindow->SubscribePresent(this, &LoongEditor::OnPresent);

        RHI::ShaderCreateInfo vs;
        vs.SourceLanguage = RHI::SHADER_SOURCE_LANGUAGE_HLSL;
        // OpenGL backend requires emulated combined HLSL texture samplers (g_Texture + g_Texture_sampler combination)
        vs.UseCombinedTextureSamplers = true;
        vs.Desc.ShaderType = RHI::SHADER_TYPE_VERTEX;
        vs.EntryPoint = "main";
        vs.Desc.Name = "Triangle vertex shader";
        vs.Source = VSSource;

        RHI::ShaderCreateInfo ps;
        ps.SourceLanguage = RHI::SHADER_SOURCE_LANGUAGE_HLSL;
        ps.UseCombinedTextureSamplers = true;
        ps.Desc.ShaderType = RHI::SHADER_TYPE_PIXEL;
        ps.EntryPoint = "main";
        ps.Desc.Name = "Triangle pixel shader";
        ps.Source = PSSource;

        pso_ = RHI::LoongRHIManager::CreateGraphicsPSOForCurrentSwapChain("Triangle", vs, ps, false, Diligent::CULL_MODE_NONE);
    }

    void OnUpdate()
    {
    }

    void OnRender()
    {
        auto immediateContext = RHI::LoongRHIManager::GetImmediateContext();
        auto swapChain = RHI::LoongRHIManager::GetSwapChain();

        assert(immediateContext != nullptr);
        assert(swapChain != nullptr);
        {

            RHI::ITextureView* pRTV = swapChain->GetCurrentBackBufferRTV();
            RHI::ITextureView* pDSV = swapChain->GetDepthBufferDSV();
            immediateContext->SetRenderTargets(1, &pRTV, pDSV, RHI::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
        }

        // Clear the back buffer
        const float ClearColor[] = { 0.350f, 0.350f, 0.350f, 1.0f };
        // Let the engine perform required state transitions
        auto* pRTV = swapChain->GetCurrentBackBufferRTV();
        auto* pDSV = swapChain->GetDepthBufferDSV();
        immediateContext->ClearRenderTarget(pRTV, ClearColor, RHI::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
        immediateContext->ClearDepthStencil(pDSV, RHI::CLEAR_DEPTH_FLAG, 1.f, 0, RHI::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

        immediateContext->SetPipelineState(pso_);

        RHI::DrawAttribs drawAttrs;
        drawAttrs.NumVertices = 3; // We will render 3 vertices
        immediateContext->Draw(drawAttrs);
    }

    void OnPresent()
    {
        RHI::LoongRHIManager::Present(true);
    }

    RHI::RefCntAutoPtr<RHI::IPipelineState> pso_ { nullptr };
};

}

void StartApp()
{
    Loong::Window::ScopedDriver appDriver;
    assert(appDriver);

    Loong::Window::LoongWindow::WindowConfig config {};
    config.title = "Play Ground";
    gWindow = std::make_shared<Loong::Window::LoongWindow>(config);

    Loong::RHI::ScopedDriver rhiDriver(gWindow->GetGlfwWindow(), Loong::RHI::RENDER_DEVICE_TYPE_VULKAN);
    assert(rhiDriver);

    Loong::LoongEditor myApp;

    gWindow->Run();

    gWindow = nullptr;
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