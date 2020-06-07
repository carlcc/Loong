#include "glad/glad.h"

#include "LoongApp/Driver.h"
#include "LoongApp/LoongApp.h"
#include "LoongFileSystem/LoongFileSystem.h"
#include "LoongFoundation/LoongClock.h"
#include "LoongFoundation/LoongFormat.h"
#include "LoongFoundation/LoongLogger.h"
#include "LoongFoundation/LoongSigslotHelper.h"
#include "LoongGui/LoongGuiButton.h"
#include "LoongGui/LoongGuiText.h"
#include "LoongGui/LoongGuiWindow.h"
#include "LoongResource/LoongResourceManager.h"

#include <imgui.h>
#include <iostream>

std::shared_ptr<Loong::App::LoongApp> gApp;

namespace Loong {

class MyApplication : public Foundation::LoongHasSlots {
public:
    MyApplication()
    {
        gApp->SubscribeUpdate(this, &MyApplication::OnUpdate);
        loongWindow_.ClearChildren();

        auto* button = loongWindow_.CreateChild<Gui::LoongGuiButton>("PushMe");
        button->SubscribeOnClick(this, &MyApplication::OnPressButton);

        for (int i = 0; i < 6; ++i) {
            int btn = int(App::LoongMouseButton::kButton1) + i;
            mouseTexts_[i] = loongWindow_.CreateChild<Gui::LoongGuiText>(Foundation::Format("Pressed button {}", btn));
        }

        for (int i = 0; i < 26; ++i) {
            int key = int(App::LoongKeyCode::kKeyA) + i;
            keyTexts_[i] = loongWindow_.CreateChild<Gui::LoongGuiText>(Foundation::Format("Pressed key {}", char(key)));
        }

        texture_ = Resource::LoongResourceManager::GetTexture("/Loong.jpg");
    }

    void OnUpdate()
    {
        clock_.Update();
        glClearColor(clearColor_[0], clearColor_[1], clearColor_[2], clearColor_[3]);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        auto& input = gApp->GetInputManager();

        for (int i = 0; i < 6; ++i) {
            int btn = int(App::LoongMouseButton::kButton1) + i;
            mouseTexts_[i]->SetVisible(input.IsMouseButtonPressed(App::LoongMouseButton(btn)));
        }

        for (int i = 0; i < 26; ++i) {
            int key = int(App::LoongKeyCode::kKeyA) + i;
            keyTexts_[i]->SetVisible(input.IsKeyPressed(App::LoongKeyCode(key)));
        }

        loongWindow_.Draw();
    }

    void OnPressButton(Gui::LoongGuiButton* button)
    {
        for (auto& c : clearColor_) {
            c = 1.0F * rand() / RAND_MAX;
        }
    }

    float clearColor_[4] { 0.3F, 0.4F, 0.5F, 1.0F };

    Foundation::LoongClock clock_;
    Gui::LoongGuiWindow loongWindow_ { "MainWindow" };

    Gui::LoongGuiText* mouseTexts_[6] { nullptr };
    Gui::LoongGuiText* keyTexts_[26] { nullptr };
    Gui::LoongGuiButton* button_ { nullptr };

    std::shared_ptr<Resource::LoongTexture> texture_ { nullptr };
};

}

#ifdef WIN32
const char kSeparator = '\\';
#else
const char kSeparator = '/';
#endif

const std::string GetDir(const std::string& path)
{
    auto index = path.rfind(kSeparator);
    assert(index != std::string::npos);

    return path.substr(0, index);
}

void StartApp()
{
    Loong::App::LoongApp::WindowConfig config {};
    config.title = "Play Ground";
    gApp = std::make_shared<Loong::App::LoongApp>(config);

    Loong::MyApplication myApp;

    gApp->Run();

    gApp = nullptr;
}

int main(int argc, char** argv)
{
    Loong::App::ScopedDriver appDriver;

    Loong::FS::ScopedDriver fsDriver(argv[0]);
    auto path = GetDir(__FILE__);
    Loong::FS::LoongFileSystem::MountSearchPath(path);

    Loong::Resource::ScopedDriver resourceDriver;

    auto listener = Loong::Foundation::Logger::Get().SubscribeLog([](const Loong::Foundation::LogItem& logItem) {
        std::cout << "[" << logItem.level << "][" << logItem.location << "]: " << logItem.message << std::endl;
    });

    StartApp();

    return 0;
}