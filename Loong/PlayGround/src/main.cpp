#include "glad/glad.h"

#include "LoongApp/Driver.h"
#include "LoongApp/LoongApp.h"
#include "LoongFoundation/LoongClock.h"
#include "LoongFoundation/LoongFormat.h"
#include "LoongFoundation/LoongSigslotHelper.h"
#include "LoongGui/LoongGuiButton.h"
#include "LoongGui/LoongGuiText.h"
#include "LoongGui/LoongGuiWindow.h"

#include <imgui.h>

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

    Gui::LoongGuiText* mouseTexts_[6];
    Gui::LoongGuiText* keyTexts_[26];
    Gui::LoongGuiButton* button_;
};

}

int main()
{
    Loong::App::ScopedDriver appDriver;

    Loong::App::LoongApp::WindowConfig config {};
    config.title = "Play Ground";
    gApp = std::make_shared<Loong::App::LoongApp>(config);

    Loong::MyApplication myApp;

    gApp->Run();

    gApp = nullptr;
    return 0;
}