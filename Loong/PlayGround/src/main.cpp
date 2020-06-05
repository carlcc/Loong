/*
 *  Copyright 2019-2020 Diligent Graphics LLC
 *  Copyright 2015-2019 Egor Yusov
 *  
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *  
 *      http://www.apache.org/licenses/LICENSE-2.0
 *  
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *  In no event and under no legal theory, whether in tort (including negligence), 
 *  contract, or otherwise, unless required by applicable law (such as deliberate 
 *  and grossly negligent acts) or agreed to in writing, shall any Contributor be
 *  liable for any damages, including any direct, indirect, special, incidental, 
 *  or consequential damages of any character arising as a result of this License or 
 *  out of the use or inability to use the software (including but not limited to damages 
 *  for loss of goodwill, work stoppage, computer failure or malfunction, or any and 
 *  all other commercial damages or losses), even if such Contributor has been advised 
 *  of the possibility of such damages.
 */

#include "LoongGui/LoongGuiWindow.h"
#include <LoongAppBase/LoongApplication.hpp>
#include <imgui.h>
#include <map>

namespace Loong {

class PlayGround final : public LoongApplication {
public:
    virtual void Initialize(const SampleInitInfo& InitInfo) override final;

    virtual void Render() override final;
    virtual void Update(double CurrTime, double ElapsedTime) override final;

    virtual const Diligent::Char* GetSampleName() const override final { return "Tutorial01: Hello Triangle"; }

private:
    Diligent::RefCntAutoPtr<Diligent::IPipelineState> m_pPSO;
    Gui::LoongGuiWindow loongWindow_;
};

LoongApplication* CreateLoongApplication()
{
    return new PlayGround();
}

// For this tutorial, we will use simple vertex shader
// that creates a procedural triangle

// Diligent Engine can use HLSL source on all supported platforms.
// It will convert HLSL to GLSL in OpenGL mode, while Vulkan backend will compile it directly to SPIRV.

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

void PlayGround::Initialize(const SampleInitInfo& InitInfo)
{
    LoongApplication::Initialize(InitInfo);

    // Pipeline state object encompasses configuration of all GPU stages

    Diligent::PipelineStateCreateInfo PSOCreateInfo;
    Diligent::PipelineStateDesc& PSODesc = PSOCreateInfo.PSODesc;

    // Pipeline state name is used by the engine to report issues.
    // It is always a good idea to give objects descriptive names.
    PSODesc.Name = "Simple triangle PSO";

    // This is a graphics pipeline
    PSODesc.IsComputePipeline = false;

    // clang-format off
    // This tutorial will render to a single render target
    PSODesc.GraphicsPipeline.NumRenderTargets             = 1;
    // Set render target format which is the format of the swap chain's color buffer
    PSODesc.GraphicsPipeline.RTVFormats[0]                = m_pSwapChain->GetDesc().ColorBufferFormat;
    // Use the depth buffer format from the swap chain
    PSODesc.GraphicsPipeline.DSVFormat                    = m_pSwapChain->GetDesc().DepthBufferFormat;
    // Primitive topology defines what kind of primitives will be rendered by this pipeline state
    PSODesc.GraphicsPipeline.PrimitiveTopology            = Diligent::PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    // No back face culling for this tutorial
    PSODesc.GraphicsPipeline.RasterizerDesc.CullMode      = Diligent::CULL_MODE_NONE;
    // Disable depth testing
    PSODesc.GraphicsPipeline.DepthStencilDesc.DepthEnable = Diligent::False;
    // clang-format on

    Diligent::ShaderCreateInfo ShaderCI;
    // Tell the system that the shader source code is in HLSL.
    // For OpenGL, the engine will convert this into GLSL under the hood.
    ShaderCI.SourceLanguage = Diligent::SHADER_SOURCE_LANGUAGE_HLSL;
    // OpenGL backend requires emulated combined HLSL texture samplers (g_Texture + g_Texture_sampler combination)
    ShaderCI.UseCombinedTextureSamplers = true;
    // Create a vertex shader
    Diligent::RefCntAutoPtr<Diligent::IShader> pVS;
    {
        ShaderCI.Desc.ShaderType = Diligent::SHADER_TYPE_VERTEX;
        ShaderCI.EntryPoint = "main";
        ShaderCI.Desc.Name = "Triangle vertex shader";
        ShaderCI.Source = VSSource;
        m_pDevice->CreateShader(ShaderCI, &pVS);
    }

    // Create a pixel shader
    Diligent::RefCntAutoPtr<Diligent::IShader> pPS;
    {
        ShaderCI.Desc.ShaderType = Diligent::SHADER_TYPE_PIXEL;
        ShaderCI.EntryPoint = "main";
        ShaderCI.Desc.Name = "Triangle pixel shader";
        ShaderCI.Source = PSSource;
        m_pDevice->CreateShader(ShaderCI, &pPS);
    }

    // Finally, create the pipeline state
    PSODesc.GraphicsPipeline.pVS = pVS;
    PSODesc.GraphicsPipeline.pPS = pPS;
    m_pDevice->CreatePipelineState(PSOCreateInfo, &m_pPSO);
}

// Render a frame
void PlayGround::Render()
{
    // Clear the back buffer
    const float ClearColor[] = { 0.350f, 0.350f, 0.350f, 1.0f };
    // Let the engine perform required state transitions
    auto* pRTV = m_pSwapChain->GetCurrentBackBufferRTV();
    auto* pDSV = m_pSwapChain->GetDepthBufferDSV();
    m_pImmediateContext->ClearRenderTarget(pRTV, ClearColor, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
    m_pImmediateContext->ClearDepthStencil(pDSV, Diligent::CLEAR_DEPTH_FLAG, 1.f, 0, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

    // Set the pipeline state in the immediate context
    m_pImmediateContext->SetPipelineState(m_pPSO);
    // Commit shader resources. Even though in this example we don't really
    // have any resources, this call also sets the shaders in OpenGL backend.
    m_pImmediateContext->CommitShaderResources(nullptr, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
    Diligent::DrawAttribs drawAttrs;
    drawAttrs.NumVertices = 3; // We will render 3 vertices
    m_pImmediateContext->Draw(drawAttrs);
}

void PlayGround::Update(double CurrTime, double ElapsedTime)
{
    LoongApplication::Update(CurrTime, ElapsedTime);

    loongWindow_.Draw();
    if (ImGui::Begin("Win")) {
        auto mouseX = GetInputManager().GetMouse().GetFloat(MouseButton::MouseAxisX);
        auto mouseY = GetInputManager().GetMouse().GetFloat(MouseButton::MouseAxisY);
        ImGui::Text("Mouse Pos: (%f, %f)", mouseX, mouseY);
        const std::map<MouseButton, const char*> kMouseNames = {
            { MouseButton::MouseButtonLeft, "MouseButtonLeft" },
            { MouseButton::MouseButtonMiddle, "MouseButtonMiddle" },
            { MouseButton::MouseButtonRight, "MouseButtonRight" },
            { MouseButton::MouseButtonWheelUp, "MouseButtonWheelUp" },
            { MouseButton::MouseButtonWheelDown, "MouseButtonWheelDown" },
        };
        for (int i = 0; i < 5; ++i) {
            int button = MouseButton::MouseButtonLeft + i;
            bool pressed = GetInputManager().GetMouse().GetBool(button);
            if (pressed) {
                auto it = kMouseNames.find(MouseButton(button));
                if (it != kMouseNames.end()) {
                    ImGui::Text("MouseButton %s pressed", it->second);
                } else {
                    ImGui::Text("MouseButton %d pressed", button);
                }
            }
        }
        for (int i = 0; i < 26; ++i) {
            auto key = Key::KeyA + i;
            if (GetInputManager().GetKeyboard().GetBool(Key(key))) {
                ImGui::Text("Key %c pressed", key);
            }
        }

        ImGui::End();
    }
}

} // namespace Diligent
