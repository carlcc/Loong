#include "LoongGui/imgui_impl_diligentengine.h"

namespace Loong::Gui {

    
ImGuiDiligentEngineIntergration::ImGuiDiligentEngineIntergration(
    RHI::IRenderDevice* device, RHI::TEXTURE_FORMAT backBufferFmt, RHI::TEXTURE_FORMAT depthBufferFmt)
    : device_(device)
    , backBufferFmt_(backBufferFmt)
    , depthBufferFmt_(depthBufferFmt)
{
}

void ImGuiDiligentEngineIntergration::NewFrame(uint32_t surfaceWidth, uint32_t surfaceHeight)
{
    if (pso_ == nullptr) {
        CreateDeviceObjects();
    }
    renderSurfaceWidth_ = surfaceWidth;
    renderSurfaceHeight_ = surfaceHeight;
}

void ImGuiDiligentEngineIntergration::EndFrame()
{
}

void ImGuiDiligentEngineIntergration::RenderDrawData(RHI::IDeviceContext* context, ImDrawData* drawData)
{
    // Avoid rendering when minimized
    if (drawData == nullptr || drawData->DisplaySize.x <= 0.0f || drawData->DisplaySize.y <= 0.0f)
        return;

    // Create and grow vertex/index buffers if needed
    if (!vbo_ || static_cast<int>(vertexBufferSize_) < drawData->TotalVtxCount) {
        vbo_.Release();
        while (static_cast<int>(vertexBufferSize_) < drawData->TotalVtxCount)
            vertexBufferSize_ = vertexBufferSize_ == 0 ? 128 : vertexBufferSize_ * 2;

        RHI::BufferDesc VBDesc;
        VBDesc.Name = "Imgui vertex buffer";
        VBDesc.BindFlags = RHI::BIND_VERTEX_BUFFER;
        VBDesc.uiSizeInBytes = vertexBufferSize_ * sizeof(ImDrawVert);
        VBDesc.Usage = RHI::USAGE_DYNAMIC;
        VBDesc.CPUAccessFlags = RHI::CPU_ACCESS_WRITE;
        device_->CreateBuffer(VBDesc, nullptr, &vbo_);
    }

    if (!ibo_ || static_cast<int>(indexBufferSize_) < drawData->TotalIdxCount) {
        ibo_.Release();
        while (static_cast<int>(indexBufferSize_) < drawData->TotalIdxCount)
            indexBufferSize_ = indexBufferSize_ == 0 ? 128 : indexBufferSize_ * 2;

        RHI::BufferDesc IBDesc;
        IBDesc.Name = "Imgui index buffer";
        IBDesc.BindFlags = RHI::BIND_INDEX_BUFFER;
        IBDesc.uiSizeInBytes = indexBufferSize_ * sizeof(ImDrawIdx);
        IBDesc.Usage = RHI::USAGE_DYNAMIC;
        IBDesc.CPUAccessFlags = RHI::CPU_ACCESS_WRITE;
        device_->CreateBuffer(IBDesc, nullptr, &ibo_);
    }

    {
        RHI::MapHelper<ImDrawVert> Verices(context, vbo_, RHI::MAP_WRITE, RHI::MAP_FLAG_DISCARD);
        RHI::MapHelper<ImDrawIdx> Indices(context, ibo_, RHI::MAP_WRITE, RHI::MAP_FLAG_DISCARD);

        ImDrawVert* vtx_dst = Verices;
        ImDrawIdx* idx_dst = Indices;
        for (int n = 0; n < drawData->CmdListsCount; n++) {
            const ImDrawList* cmd_list = drawData->CmdLists[n];
            memcpy(vtx_dst, cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Size * sizeof(ImDrawVert));
            memcpy(idx_dst, cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx));
            vtx_dst += cmd_list->VtxBuffer.Size;
            idx_dst += cmd_list->IdxBuffer.Size;
        }
    }

    // Setup orthographic projection matrix into our constant buffer
    // Our visible imgui space lies from drawData->DisplayPos (top left) to drawData->DisplayPos+data_data->DisplaySize (bottom right).
    // DisplayPos is (0,0) for single viewport apps.
    {
        // DisplaySize always refers to the logical dimensions that account for pre-transform, hence
        // the aspect ratio will be correct after applying appropriate rotation.
        float L = drawData->DisplayPos.x;
        float R = drawData->DisplayPos.x + drawData->DisplaySize.x;
        float T = drawData->DisplayPos.y;
        float B = drawData->DisplayPos.y + drawData->DisplaySize.y;

        // clang-format off
        RHI::float4x4 Projection
        {
            2.0f / (R - L),                  0.0f,   0.0f,   0.0f,
            0.0f,                  2.0f / (T - B),   0.0f,   0.0f,
            0.0f,                            0.0f,   0.5f,   0.0f,
            (R + L) / (L - R),  (T + B) / (B - T),   0.5f,   1.0f
        };
        // clang-format on

        RHI::MapHelper<RHI::float4x4> CBData(context, vertexConstantBuffer_, RHI::MAP_WRITE, RHI::MAP_FLAG_DISCARD);
        *CBData = Projection;
    }

    auto SetupRenderState = [&]() //
    {
        // Setup shader and vertex buffers
        uint32_t Offsets[] = { 0 };
        RHI::IBuffer* pVBs[] = { vbo_ };
        context->SetVertexBuffers(0, 1, pVBs, Offsets, RHI::RESOURCE_STATE_TRANSITION_MODE_TRANSITION, RHI::SET_VERTEX_BUFFERS_FLAG_RESET);
        context->SetIndexBuffer(ibo_, 0, RHI::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
        context->SetPipelineState(pso_);

        const float blend_factor[4] = { 0.f, 0.f, 0.f, 0.f };
        context->SetBlendFactors(blend_factor);

        RHI::Viewport vp;
        vp.Width = static_cast<float>(renderSurfaceWidth_) * drawData->FramebufferScale.x;
        vp.Height = static_cast<float>(renderSurfaceHeight_) * drawData->FramebufferScale.y;
        vp.MinDepth = 0.0f;
        vp.MaxDepth = 1.0f;
        vp.TopLeftX = vp.TopLeftY = 0;
        context->SetViewports(1,
            &vp,
            static_cast<uint32_t>(renderSurfaceWidth_ * drawData->FramebufferScale.x),
            static_cast<uint32_t>(renderSurfaceHeight_* drawData->FramebufferScale.y));
    };

    SetupRenderState();

    // Render command lists
    // (Because we merged all buffers into a single one, we maintain our own offset into them)
    int global_idx_offset = 0;
    int global_vtx_offset = 0;

    RHI::ITextureView* pLastTextureView = nullptr;
    for (int n = 0; n < drawData->CmdListsCount; n++) {
        const ImDrawList* cmd_list = drawData->CmdLists[n];
        for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++) {
            const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
            if (pcmd->UserCallback != NULL) {
                // User callback, registered via ImDrawList::AddCallback()
                // (ImDrawCallback_ResetRenderState is a special callback value used by the user to request the renderer to reset render state.)
                if (pcmd->UserCallback == ImDrawCallback_ResetRenderState)
                    SetupRenderState();
                else
                    pcmd->UserCallback(cmd_list, pcmd);
            } else {
                // Apply scissor/clipping rectangle
                RHI::float4 ClipRect {
                    (pcmd->ClipRect.x - drawData->DisplayPos.x) * drawData->FramebufferScale.x,
                    (pcmd->ClipRect.y - drawData->DisplayPos.y) * drawData->FramebufferScale.y,
                    (pcmd->ClipRect.z - drawData->DisplayPos.x) * drawData->FramebufferScale.x,
                    (pcmd->ClipRect.w - drawData->DisplayPos.y) * drawData->FramebufferScale.y,
                };

                RHI::Rect r {
                    static_cast<int32_t>(ClipRect.x),
                    static_cast<int32_t>(ClipRect.y),
                    static_cast<int32_t>(ClipRect.z),
                    static_cast<int32_t>(ClipRect.w),
                };
                context->SetScissorRects(1,
                    &r,
                    static_cast<uint32_t>(renderSurfaceWidth_ * drawData->FramebufferScale.x),
                    static_cast<uint32_t>(renderSurfaceHeight_ * drawData->FramebufferScale.y));

                // Bind texture
                auto* pTextureView = reinterpret_cast<RHI::ITextureView*>(pcmd->TextureId);
                VERIFY_EXPR(pTextureView);
                if (pTextureView != pLastTextureView) {
                    pLastTextureView = pTextureView;
                    textureVar_->Set(pTextureView);
                    context->CommitShaderResources(srb_, RHI::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
                }

                // Draw
                RHI::DrawIndexedAttribs DrawAttrs(pcmd->ElemCount, sizeof(ImDrawIdx) == 2 ? RHI::VT_UINT16 : RHI::VT_UINT32, RHI::DRAW_FLAG_VERIFY_STATES);
                DrawAttrs.FirstIndexLocation = pcmd->IdxOffset + global_idx_offset;
                DrawAttrs.BaseVertex = pcmd->VtxOffset + global_vtx_offset;
                context->DrawIndexed(DrawAttrs);
            }
        }
        global_idx_offset += cmd_list->IdxBuffer.Size;
        global_vtx_offset += cmd_list->VtxBuffer.Size;
    }
}

void ImGuiDiligentEngineIntergration::InvalidateDeviceObjects()
{
    vbo_.Release();
    ibo_.Release();
    vertexConstantBuffer_.Release();
    pso_.Release();
    fontSRV_.Release();
    srb_.Release();
}

static const char* kVertexShaderHLSL = R"(
cbuffer Constants
{
    float4x4 ProjectionMatrix;
}

struct VSInput
{
    float2 pos : ATTRIB0;
    float2 uv  : ATTRIB1;
    float4 col : ATTRIB2;
};

struct PSInput
{
    float4 pos : SV_POSITION;
    float4 col : COLOR;
    float2 uv  : TEXCOORD;
};

void main(in VSInput VSIn, out PSInput PSIn)
{
    PSIn.pos = mul(ProjectionMatrix, float4(VSIn.pos.xy, 0.0, 1.0));
    PSIn.col = VSIn.col;
    PSIn.uv  = VSIn.uv;
}
)";
static const char* kPixelShaderHLSL = R"(
struct PSInput
{
    float4 pos : SV_POSITION;
    float4 col : COLOR;
    float2 uv  : TEXCOORD;
};

Texture2D    Texture;
SamplerState Texture_sampler;

float4 main(in PSInput PSIn) : SV_Target
{
    return PSIn.col * Texture.Sample(Texture_sampler, PSIn.uv);
}
)";

void ImGuiDiligentEngineIntergration::CreateDeviceObjects()
{
    InvalidateDeviceObjects();

    RHI::ShaderCreateInfo shaderCI;
    shaderCI.UseCombinedTextureSamplers = true;
    shaderCI.SourceLanguage = RHI::SHADER_SOURCE_LANGUAGE_HLSL;

    RHI::RefCntAutoPtr<RHI::IShader> pVS;
    {
        shaderCI.Desc.ShaderType = RHI::SHADER_TYPE_VERTEX;
        shaderCI.Desc.Name = "Imgui VS";
        shaderCI.Source = kVertexShaderHLSL;
        device_->CreateShader(shaderCI, &pVS);
    }
    RHI::RefCntAutoPtr<RHI::IShader> pPS;
    {
        shaderCI.Desc.ShaderType = RHI::SHADER_TYPE_PIXEL;
        shaderCI.Desc.Name = "Imgui PS";
        shaderCI.Source = kPixelShaderHLSL;
        device_->CreateShader(shaderCI, &pPS);
    }

    RHI::PipelineStateCreateInfo psoCreateInfo {};
    RHI::PipelineStateDesc& psoDesc = psoCreateInfo.PSODesc;

    psoDesc.Name = "ImGUI PSO";
    auto& graphicsPipeline = psoDesc.GraphicsPipeline;

    graphicsPipeline.NumRenderTargets = 1;
    graphicsPipeline.RTVFormats[0] = backBufferFmt_;
    graphicsPipeline.DSVFormat = depthBufferFmt_;
    graphicsPipeline.PrimitiveTopology = RHI::PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    graphicsPipeline.pVS = pVS;
    graphicsPipeline.pPS = pPS;

    graphicsPipeline.RasterizerDesc.CullMode = RHI::CULL_MODE_NONE;
    graphicsPipeline.RasterizerDesc.ScissorEnable = true;
    graphicsPipeline.DepthStencilDesc.DepthEnable = false;

    auto& rt0 = graphicsPipeline.BlendDesc.RenderTargets[0];
    rt0.BlendEnable = true;
    rt0.SrcBlend = RHI::BLEND_FACTOR_SRC_ALPHA;
    rt0.DestBlend = RHI::BLEND_FACTOR_INV_SRC_ALPHA;
    rt0.BlendOp = RHI::BLEND_OPERATION_ADD;
    rt0.SrcBlendAlpha = RHI::BLEND_FACTOR_INV_SRC_ALPHA;
    rt0.DestBlendAlpha = RHI::BLEND_FACTOR_ZERO;
    rt0.BlendOpAlpha = RHI::BLEND_OPERATION_ADD;
    rt0.RenderTargetWriteMask = RHI::COLOR_MASK_ALL;

    RHI::LayoutElement vsInputs[] {
        { 0, 0, 2, RHI::VT_FLOAT32 },       // pos
        { 1, 0, 2, RHI::VT_FLOAT32 },       // uv
        { 2, 0, 4, RHI::VT_UINT8, true },   // color
    };
    graphicsPipeline.InputLayout.NumElements = _countof(vsInputs);
    graphicsPipeline.InputLayout.LayoutElements = vsInputs;

    RHI::ShaderResourceVariableDesc variables[] {
        { RHI::SHADER_TYPE_PIXEL, "Texture", RHI::SHADER_RESOURCE_VARIABLE_TYPE_DYNAMIC },
    };

    psoDesc.ResourceLayout.Variables = variables;
    psoDesc.ResourceLayout.NumVariables = _countof(variables);

    RHI::SamplerDesc samLinearWrap {};
    samLinearWrap.AddressU = RHI::TEXTURE_ADDRESS_WRAP;
    samLinearWrap.AddressV = RHI::TEXTURE_ADDRESS_WRAP;
    samLinearWrap.AddressW = RHI::TEXTURE_ADDRESS_WRAP;

    RHI::StaticSamplerDesc staticSamplers[] {
        { RHI::SHADER_TYPE_PIXEL, "Texture", samLinearWrap },
    };
    psoDesc.ResourceLayout.StaticSamplers = staticSamplers;
    psoDesc.ResourceLayout.NumStaticSamplers = _countof(staticSamplers);

    device_->CreatePipelineState(psoCreateInfo, &pso_);

     {
        RHI::BufferDesc buffDesc;
        buffDesc.uiSizeInBytes = sizeof(RHI::float4x4);
        buffDesc.Usage = RHI::USAGE_DYNAMIC;
        buffDesc.BindFlags = RHI::BIND_UNIFORM_BUFFER;
        buffDesc.CPUAccessFlags = RHI::CPU_ACCESS_WRITE;
        device_->CreateBuffer(buffDesc, nullptr, &vertexConstantBuffer_);
    }
     pso_->GetStaticVariableByName(RHI::SHADER_TYPE_VERTEX, "Constants")->Set(vertexConstantBuffer_);

    CreateFontsTexture();
}

void ImGuiDiligentEngineIntergration::CreateFontsTexture()
{
    // Build texture atlas
    ImGuiIO& io = ImGui::GetIO();
    unsigned char* pixels = nullptr;
    int width = 0, height = 0;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

    RHI::TextureDesc fontTexDesc;
    fontTexDesc.Name = "Imgui font texture";
    fontTexDesc.Type = RHI::RESOURCE_DIM_TEX_2D;
    fontTexDesc.Width = static_cast<uint32_t>(width);
    fontTexDesc.Height = static_cast<uint32_t>(height);
    fontTexDesc.Format = RHI::TEX_FORMAT_RGBA8_UNORM;
    fontTexDesc.BindFlags = RHI::BIND_SHADER_RESOURCE;
    fontTexDesc.Usage = RHI::USAGE_STATIC;

    RHI::TextureSubResData Mip0Data[] = { { pixels, fontTexDesc.Width * 4 } };
    RHI::TextureData InitData(Mip0Data, _countof(Mip0Data));

    RHI::RefCntAutoPtr<RHI::ITexture> pFontTex;
    device_->CreateTexture(fontTexDesc, &InitData, &pFontTex);
    fontSRV_ = pFontTex->GetDefaultView(RHI::TEXTURE_VIEW_SHADER_RESOURCE);

    srb_.Release();
    pso_->CreateShaderResourceBinding(&srb_, true);
    textureVar_ = srb_->GetVariableByName(RHI::SHADER_TYPE_PIXEL, "Texture");
    VERIFY_EXPR(textureVar_ != nullptr);

    // Store our identifier
    io.Fonts->TexID = (ImTextureID)fontSRV_;
}

}