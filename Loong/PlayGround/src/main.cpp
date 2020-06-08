#include "glad/glad.h"

#include "LoongApp/Driver.h"
#include "LoongApp/LoongApp.h"
#include "LoongFileSystem/LoongFileSystem.h"
#include "LoongFoundation/LoongClock.h"
#include "LoongFoundation/LoongFormat.h"
#include "LoongFoundation/LoongLogger.h"
#include "LoongFoundation/LoongSigslotHelper.h"
#include "LoongGui/LoongGuiButton.h"
#include "LoongGui/LoongGuiImage.h"
#include "LoongGui/LoongGuiText.h"
#include "LoongGui/LoongGuiWindow.h"
#include "LoongRenderer/LoongCamera.h"
#include "LoongRenderer/LoongRenderer.h"
#include "LoongResource/LoongGpuBuffer.h"
#include "LoongResource/LoongGpuModel.h"
#include "LoongResource/LoongResourceManager.h"
#include "LoongResource/LoongShader.h"
#include <imgui.h>
#include <iostream>

std::shared_ptr<Loong::App::LoongApp> gApp;

namespace Loong {

class MyApplication : public Foundation::LoongHasSlots {
public:
    struct UBO {
        Math::Matrix4 ub_Model;
        Math::Matrix4 ub_View;
        Math::Matrix4 ub_Projection;
        Math::Vector3 ub_ViewPos;
        float ub_Time;
    };

    MyApplication()
    {
        texture_ = Resource::LoongResourceManager::GetTexture("/Loong.jpg");
        unlitShader_ = Resource::LoongResourceManager::GetShader("/unlit.glsl");
        cubeModel_ = Resource::LoongResourceManager::GetModel("/cube.fbx");
        unlitShader_->Bind();
        UBO ubo;
        basicUniforms_.BufferData(&ubo, 1); // Note: Must allocate memory first
        basicUniforms_.SetBindingPoint(0, sizeof(UBO));
        auto index = unlitShader_->GetUniformBlockLocation("BasicUBO");
        unlitShader_->BindUniformBlock(index, 0);
        unlitShader_->Unbind();

        gApp->SubscribeUpdate(this, &MyApplication::OnUpdate);
        gApp->SubscribeRender(this, &MyApplication::OnRender);
        loongWindow_.ClearChildren();

        auto* button = loongWindow_.CreateChild<Gui::LoongGuiButton>("PushMe");
        button->SubscribeOnClick(this, &MyApplication::OnPressButton);

        auto* image = loongWindow_.CreateChild<Gui::LoongGuiImage>();
        image->SetTexture(texture_);

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

    void OnRender()
    {
        int width, height;
        {
            gApp->GetFramebufferSize(width, height);
            glEnable(GL_DEPTH_TEST);
            glViewport(0, 0, width, height);
            std::cout << "(" << width << ", " << height << ")" << std::endl;
            glClearColor(clearColor_[0], clearColor_[1], clearColor_[2], clearColor_[3]);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        }

        unlitShader_->Bind();
        UBO ubo;
        ubo.ub_ViewPos = { 0.0F, 1.0F, 3.0F };
        ubo.ub_Model = Math::Identity;
        ubo.ub_View = Math::LookAt(ubo.ub_ViewPos, Math::Zero, Math::kUp);
        ubo.ub_Projection = Math::Perspective(Math::DegreeToRad(45.0F), width, height, 0.01F, 1000.F);
        // camera_.UpdateMatrices(width, height, { 2.0F, 0.0F, 2.0F }, Math::Identity);
        for (auto* mesh : cubeModel_->GetMeshes()) {
            basicUniforms_.SetSubData(&ubo, 0);
            renderer_.Draw(*mesh);
        }
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
    std::shared_ptr<Resource::LoongShader> unlitShader_ { nullptr };
    std::shared_ptr<Resource::LoongGpuModel> cubeModel_ { nullptr };
    Renderer::Renderer renderer_;
    Renderer::LoongCamera camera_;
    Resource::LoongUniformBuffer basicUniforms_;
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