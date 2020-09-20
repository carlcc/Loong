#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include "LoongApp/Driver.h"
#include "LoongApp/LoongApp.h"
#include "LoongFoundation/LoongLogger.h"
#include "LoongRHI/LoongRHIManager.h"
#include <NativeWindow.h>
#include <cassert>
#include <imgui.h>
#include <iostream>

std::shared_ptr<Loong::App::LoongApp> gApp;

namespace Loong {

class LoongEditor : public Foundation::LoongHasSlots {
public:
    LoongEditor()
    {
        gApp->SubscribeUpdate(this, &LoongEditor::OnUpdate);
        gApp->SubscribeRender(this, &LoongEditor::OnRender);
        RHI::NativeWindow nw { glfwGetWin32Window(gApp->GetGlfwWindow()) };
        assert(RHI::LoongRHIManager::Initialize(nw, RHI::RENDER_DEVICE_TYPE_VULKAN));
    }

    void OnUpdate()
    {
    }

    void OnRender()
    {
    }
};

}

void StartApp()
{
    Loong::App::LoongApp::WindowConfig config {};
    config.title = "Play Ground";
    gApp = std::make_shared<Loong::App::LoongApp>(config);

    Loong::LoongEditor myApp;

    gApp->Run();

    gApp = nullptr;
}

int main(int argc, char** argv)
{
    Loong::App::ScopedDriver appDriver;

    auto listener = Loong::Foundation::Logger::Get().SubscribeLog([](const Loong::Foundation::LogItem& logItem) {
        std::cout << "[" << logItem.level << "][" << logItem.location << "]: " << logItem.message << std::endl;
    });

    StartApp();

    return 0;
}