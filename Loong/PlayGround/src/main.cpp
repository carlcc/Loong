#include <GLFW/glfw3.h>

#include "LoongApp/Driver.h"
#include "LoongApp/LoongWindow.h"
#include "LoongFoundation/LoongLogger.h"
#include "LoongRHI/Driver.h"
#include "LoongRHI/LoongRHIManager.h"
#include <cassert>
#include <iostream>

std::shared_ptr<Loong::App::LoongWindow> gApp;

namespace Loong {

class LoongEditor : public Foundation::LoongHasSlots {
public:
    LoongEditor()
    {
        gApp->SubscribeUpdate(this, &LoongEditor::OnUpdate);
        gApp->SubscribeRender(this, &LoongEditor::OnRender);
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
    Loong::App::ScopedDriver appDriver;
    assert(appDriver);

    Loong::App::LoongWindow::WindowConfig config {};
    config.title = "Play Ground";
    gApp = std::make_shared<Loong::App::LoongWindow>(config);

    Loong::RHI::ScopedDriver rhiDriver(gApp->GetGlfwWindow(), Loong::RHI::RENDER_DEVICE_TYPE_VULKAN);
    assert(rhiDriver);

    Loong::LoongEditor myApp;

    gApp->Run();

    gApp = nullptr;
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