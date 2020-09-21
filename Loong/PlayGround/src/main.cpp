#include <GLFW/glfw3.h>

#include "LoongWindow/Driver.h"
#include "LoongWindow/LoongWindow.h"
#include "LoongFoundation/LoongLogger.h"
#include "LoongRHI/Driver.h"
#include "LoongRHI/LoongRHIManager.h"
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

        auto swapChain = RHI::LoongRHIManager::GetSwapChain();
        auto device = RHI::LoongRHIManager::GetDevice();

        RHI::PipelineStateCreateInfo psoCreateInfo;
        RHI::PipelineStateDesc& psoDesc = psoCreateInfo.PSODesc;

        // Pipeline state name is used by the engine to report issues.
        // It is always a good idea to give objects descriptive names.
        psoDesc.Name = "Simple triangle PSO";

        // This is a graphics pipeline
        psoDesc.PipelineType = RHI::PIPELINE_TYPE_GRAPHICS;

        // clang-format off
        // This tutorial will render to a single render target
        psoDesc.GraphicsPipeline.NumRenderTargets             = 1;
        // Set render target format which is the format of the swap chain's color buffer
        psoDesc.GraphicsPipeline.RTVFormats[0]                = swapChain->GetDesc().ColorBufferFormat;
        // Use the depth buffer format from the swap chain
        psoDesc.GraphicsPipeline.DSVFormat                    = swapChain->GetDesc().DepthBufferFormat;
        // Primitive topology defines what kind of primitives will be rendered by this pipeline state
        psoDesc.GraphicsPipeline.PrimitiveTopology            = RHI::PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        // No back face culling for this tutorial
        psoDesc.GraphicsPipeline.RasterizerDesc.CullMode      = RHI::CULL_MODE_NONE;
        // Disable depth testing
        psoDesc.GraphicsPipeline.DepthStencilDesc.DepthEnable = false;
        // clang-format on

        RHI::ShaderCreateInfo ShaderCI;
        // Tell the system that the shader source code is in HLSL.
        // For OpenGL, the engine will convert this into GLSL under the hood.
        ShaderCI.SourceLanguage = RHI::SHADER_SOURCE_LANGUAGE_HLSL;
        // OpenGL backend requires emulated combined HLSL texture samplers (g_Texture + g_Texture_sampler combination)
        ShaderCI.UseCombinedTextureSamplers = true;
        // Create a vertex shader
        RHI::RefCntAutoPtr<RHI::IShader> pVS;
        {
            ShaderCI.Desc.ShaderType = RHI::SHADER_TYPE_VERTEX;
            ShaderCI.EntryPoint = "main";
            ShaderCI.Desc.Name = "Triangle vertex shader";
            ShaderCI.Source = VSSource;
            device->CreateShader(ShaderCI, &pVS);
        }

        // Create a pixel shader
        RHI::RefCntAutoPtr<RHI::IShader> pPS;
        {
            ShaderCI.Desc.ShaderType = RHI::SHADER_TYPE_PIXEL;
            ShaderCI.EntryPoint = "main";
            ShaderCI.Desc.Name = "Triangle pixel shader";
            ShaderCI.Source = PSSource;
            device->CreateShader(ShaderCI, &pPS);
        }

        // Finally, create the pipeline state
        psoDesc.GraphicsPipeline.pVS = pVS;
        psoDesc.GraphicsPipeline.pPS = pPS;
        device->CreatePipelineState(psoCreateInfo, &pso_);
    }

    void OnUpdate()
    {
    }

    void OnRender()
    {
        auto immediateContext = RHI::LoongRHIManager::GetImmediateContext();
        auto swapChain = RHI::LoongRHIManager::GetSwapChain();
        {
            if (!immediateContext || !swapChain)
                return;

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

        // Set the pipeline state in the immediate context
        immediateContext->SetPipelineState(pso_);

        // Typically we should now call CommitShaderResources(), however shaders in this example don't
        // use any resources.

        RHI::DrawAttribs drawAttrs;
        drawAttrs.NumVertices = 3; // We will render 3 vertices
        immediateContext->Draw(drawAttrs);
    }

    void OnPresent() {
        auto swapChain = RHI::LoongRHIManager::GetSwapChain();
        if (!swapChain)
            return;
        bool vsync = 1;
        swapChain->Present(vsync ? 1 : 0);
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