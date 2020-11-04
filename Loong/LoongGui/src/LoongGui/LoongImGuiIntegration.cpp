//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//
#include "LoongGui/LoongImGuiIntegration.h"
#include "imgui_impl_diligentengine.h"
#include "imgui_impl_glfw.h"
#include <GLFW/glfw3.h>
#include <imgui.h>

namespace Loong::Gui {

LoongImGuiIntegration::LoongImGuiIntegration(GLFWwindow* glfwWindow, RHI::IRenderDevice* device, RHI::ISwapChain* swapchain)
    : imgui_ { nullptr }
    , window_ { glfwWindow }
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;

    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable; // Enable Docking
    // io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable; // Enable Multi-Viewport / Platform Windows
    //io.ConfigViewportsNoAutoMerge = true;
    //io.ConfigViewportsNoTaskBarIcon = true;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsClassic();

    // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    auto swapchainDesc = swapchain->GetDesc();
    // Setup Platform/Renderer bindings
    // const char* glsl_version = "#version 150";
    ImGui_ImplGlfw_InitForDiligentEngine(glfwWindow, true);
    imgui_ = new Gui::ImGuiImplDiligentEngine(device, swapchainDesc.ColorBufferFormat, swapchainDesc.DepthBufferFormat);

    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
    // - If the file cannot be loaded, the function will return NULL. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
    // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
    // - Read 'docs/FONTS.txt' for more instructions and details.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
    //io.Fonts->AddFontDefault();
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/ProggyTiny.ttf", 10.0f);
    //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
    // io.Fonts->AddFontFromFileTTF("resource/fonts/wqymicroheimono.ttf", 16.0f);
    //IM_ASSERT(font != NULL);
}

LoongImGuiIntegration::~LoongImGuiIntegration()
{
    delete imgui_;
    imgui_ = nullptr;
    ImGui_ImplGlfw_Shutdown();
}

void LoongImGuiIntegration::NewFrame()
{
    int w, h;
    glfwGetFramebufferSize(window_, &w, &h);
    imgui_->NewFrame(w, h);
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void LoongImGuiIntegration::EndFrame()
{
    ImGui::EndFrame();
}

void LoongImGuiIntegration::Render(RHI::IDeviceContext* context)
{
    ImGui::Render();
    imgui_->RenderDrawData(context, ImGui::GetDrawData());
}

}