//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#pragma once
#include "LoongRHI/LoongRHIManager.h"
#include <cstdint>

struct GLFWwindow;

namespace Loong::Gui {

class ImGuiImplDiligentEngine;

class LoongImGuiIntegration {
public:
    LoongImGuiIntegration(GLFWwindow* glfwWindow, RHI::IRenderDevice* device, RHI::ISwapChain* swapchain);

    LoongImGuiIntegration(const LoongImGuiIntegration&) = delete;
    LoongImGuiIntegration(LoongImGuiIntegration&&) = delete;

    ~LoongImGuiIntegration();

    LoongImGuiIntegration& operator=(const LoongImGuiIntegration&) = delete;
    LoongImGuiIntegration& operator=(LoongImGuiIntegration&&) = delete;

    void NewFrame();

    void EndFrame();

    void Render(RHI::IDeviceContext* context);

private:
    ImGuiImplDiligentEngine* imgui_ { nullptr };
    void* imguiContext_ { nullptr };
    GLFWwindow* window_ { nullptr };
};

}