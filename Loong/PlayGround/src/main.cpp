#include "glad/glad.h"

#include "LoongApp/Driver.h"
#include "LoongApp/LoongApp.h"
#include "LoongFoundation/LoongClock.h"
#include "LoongFoundation/LoongFormat.h"
#include "LoongFoundation/LoongSigslotHelper.h"
#include "LoongGui/LoongGuiButton.h"
#include "LoongGui/LoongGuiText.h"
#include "LoongGui/LoongGuiWindow.h"

std::shared_ptr<Loong::App::LoongApp> gApp;

namespace Loong {

class MyApplication : public Foundation::LoongHasSlots {
public:
    MyApplication()
    {
        gApp->SubscribeUpdate(this, &MyApplication::OnUpdate);
    }

    void OnUpdate()
    {
        clock_.Update();
        glClearColor(0.3F, 0.4F, 0.5F, 1.0F);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        auto& input = gApp->GetInputManager();
        loongWindow_.ClearChildren();

        for (int i = 0; i < 6; ++i) {
            int btn = int(App::LoongMouseButton::kButton1) + i;
            if (input.IsMouseButtonPressed(App::LoongMouseButton(btn))) {
                loongWindow_.CreateChild<Gui::LoongGuiText>(Foundation::Format("Pressed button {}", btn));
            }
        }

        for (int i = 0; i < 26; ++i) {
            int key = int(App::LoongKeyCode::kKeyA) + i;
            if (input.IsKeyPressed(Loong::App::LoongKeyCode(key))) {
                loongWindow_.CreateChild<Gui::LoongGuiText>(Foundation::Format("Pressed key {}", char(key)));
            }
        }

        loongWindow_.Draw();
    }

    Foundation::LoongClock clock_;
    Gui::LoongGuiWindow loongWindow_ { "MainWindow" };
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