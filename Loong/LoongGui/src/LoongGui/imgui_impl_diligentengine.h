#pragma once
#include "LoongRHI/LoongRHIManager.h"
#include <cstdint>
#include <imgui.h>

namespace Loong::Gui {

class ImGuiImplDiligentEngine {
public:
    ImGuiImplDiligentEngine(RHI::IRenderDevice* device, RHI::TEXTURE_FORMAT backBufferFmt, RHI::TEXTURE_FORMAT depthBufferFmt);

    void NewFrame(uint32_t surfaceWidth, uint32_t surfaceHeight);

    void EndFrame();

    void RenderDrawData(RHI::IDeviceContext* context, ImDrawData* drawData);

    void InvalidateDeviceObjects();

    void CreateDeviceObjects();

    void CreateFontsTexture();

private:
    RHI::RefCntAutoPtr<RHI::IRenderDevice> device_;
    RHI::RefCntAutoPtr<RHI::IBuffer> vbo_;
    RHI::RefCntAutoPtr<RHI::IBuffer> ibo_;
    RHI::RefCntAutoPtr<RHI::IBuffer> vertexConstantBuffer_;
    RHI::RefCntAutoPtr<RHI::IPipelineState> pso_;
    RHI::RefCntAutoPtr<RHI::ITextureView> fontSRV_;
    RHI::RefCntAutoPtr<RHI::IShaderResourceBinding> srb_;
    RHI::IShaderResourceVariable* textureVar_;


    const RHI::TEXTURE_FORMAT backBufferFmt_;
    const RHI::TEXTURE_FORMAT depthBufferFmt_;
    uint32_t vertexBufferSize_ { 0 };
    uint32_t indexBufferSize_ { 0 };
    uint32_t renderSurfaceWidth_ { 0 };
    uint32_t renderSurfaceHeight_ { 0 };
};

}