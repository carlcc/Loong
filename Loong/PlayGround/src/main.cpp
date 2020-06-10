#include "glad/glad.h"

#include "LoongApp/Driver.h"
#include "LoongApp/LoongApp.h"
#include "LoongCore/scene/LoongActor.h"
#include "LoongCore/scene/LoongScene.h"
#include "LoongCore/scene/components/LoongCCamera.h"
#include "LoongCore/scene/components/LoongCModelRenderer.h"
#include "LoongFileSystem/Driver.h"
#include "LoongFileSystem/LoongFileSystem.h"
#include "LoongFoundation/LoongClock.h"
#include "LoongFoundation/LoongFormat.h"
#include "LoongFoundation/LoongLogger.h"
#include "LoongFoundation/LoongPathUtils.h"
#include "LoongFoundation/LoongSigslotHelper.h"
#include "LoongGui/LoongGuiButton.h"
#include "LoongGui/LoongGuiImage.h"
#include "LoongGui/LoongGuiText.h"
#include "LoongGui/LoongGuiWindow.h"
#include "LoongRenderer/LoongCamera.h"
#include "LoongRenderer/LoongRenderer.h"
#include "LoongResource/Driver.h"
#include "LoongResource/LoongGpuBuffer.h"
#include "LoongResource/LoongGpuModel.h"
#include "LoongResource/LoongMaterial.h"
#include "LoongResource/LoongResourceManager.h"
#include "LoongResource/LoongShader.h"
#include "LoongResource/LoongTexture.h"
#include <imgui.h>

std::shared_ptr<Loong::App::LoongApp> gApp;

namespace Loong {

class LoongEditor : public Foundation::LoongHasSlots {
public:
    struct UBO {
        Math::Matrix4 ub_Model;
        Math::Matrix4 ub_View;
        Math::Matrix4 ub_Projection;
        Math::Vector3 ub_ViewPos;
        float ub_Time;
    };

    LoongEditor()
    {
        texture_ = Resource::LoongResourceManager::GetTexture("Textures/Loong.jpg");
        scene_ = std::make_shared<Core::LoongScene>(0, "SceneRoot", "");

        auto fireTexture = Resource::LoongResourceManager::GetTexture("Textures/fire.jpg");
        auto material = std::make_shared<Resource::LoongMaterial>();
        material->SetShaderByFile("Shaders/unlit.glsl");
        material->GetUniformsData()["u_DiffuseMap"] = fireTexture;

        auto cubeModel = Resource::LoongResourceManager::GetModel("Models/cube.fbx");
        auto* actor = new Core::LoongActor(1, "ActorCube", "");
        auto* modelRenderer = actor->AddComponent<Core::LoongCModelRenderer>();
        modelRenderer->SetModel(cubeModel);
        modelRenderer->SetMaterial(0, material);

        actor->SetParent(scene_.get());

        auto* cameraActor = new Core::LoongActor(2, "ActorCamera", "");
        cameraComponent_ = cameraActor->AddComponent<Core::LoongCCamera>();
        auto& cameraTransform = cameraActor->GetTransform();
        cameraTransform.SetPosition({ 0.0F, 1.0F, 12.0F });
        cameraTransform.LookAt(Math::Zero, Math::kUp);
        cameraActor->SetParent(scene_.get());

        UBO ubo;
        basicUniforms_.BufferData(&ubo, 1); // Note: Must allocate memory first
        basicUniforms_.SetBindingPoint(0, sizeof(UBO));

        gApp->SubscribeUpdate(this, &LoongEditor::OnUpdate);
        gApp->SubscribeRender(this, &LoongEditor::OnRender);
        loongWindow_.ClearChildren();

        auto* button = loongWindow_.CreateChild<Gui::LoongGuiButton>("PushMe");
        button->SubscribeOnClick(this, &LoongEditor::OnPressButton);

        auto* image = loongWindow_.CreateChild<Gui::LoongGuiImage>();
        image->SetTexture(texture_);

        fpsText_ = loongWindow_.CreateChild<Gui::LoongGuiText>("FPS: ");
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

        if (auto* cubeActor = scene_->GetChildByName("ActorCube"); cubeActor != nullptr) {
            cubeActor->GetTransform().Rotate(Math::kUp, clock_.DeltaTime());
            cubeActor->GetTransform().SetPosition({ std::sinf(clock_.ElapsedTime()) * 2.0F, 0.0F, std::cosf(clock_.ElapsedTime()) * 2.0F });
        }

        loongWindow_.Draw();
        // material_->GetUniformsData()["u_TextureTiling"] = Math::Vector2 { std::fabs(std::fmod(clock_.ElapsedTime(), 10.0f) - 5.0F) };
        {
            static int frameCount = 0;
            static int lastTime = 0;
            if ((int)clock_.ElapsedTime() > lastTime) {
                lastTime = (int)clock_.ElapsedTime();
                fpsText_->SetLabel(Foundation::Format("FPS: {}", frameCount));
                frameCount = 0;
            }
            frameCount++;
        }
    }

    void OnRender()
    {
        int width, height;
        {
            gApp->GetFramebufferSize(width, height);
            glEnable(GL_DEPTH_TEST);
            glViewport(0, 0, width, height);
            glClearColor(clearColor_[0], clearColor_[1], clearColor_[2], clearColor_[3]);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        }
        auto& cameraActorTransform = cameraComponent_->GetOwner()->GetTransform();
        cameraComponent_->GetCamera().UpdateMatrices(width, height, cameraActorTransform.GetWorldPosition(), cameraActorTransform.GetWorldRotation());

        UBO ubo;
        ubo.ub_ViewPos = cameraActorTransform.GetWorldPosition();
        ubo.ub_View = cameraComponent_->GetCamera().GetViewMatrix();
        ubo.ub_Projection = cameraComponent_->GetCamera().GetProjectionMatrix();

        scene_->Render(renderer_, *cameraComponent_, nullptr, [&ubo, this](const Math::Matrix4& modelMatrix) {
            ubo.ub_Model = modelMatrix;
            basicUniforms_.SetSubData(&ubo, 0);
        });
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

    Gui::LoongGuiText* fpsText_ { nullptr };
    Gui::LoongGuiText* mouseTexts_[6] { nullptr };
    Gui::LoongGuiText* keyTexts_[26] { nullptr };
    Gui::LoongGuiButton* button_ { nullptr };

    std::shared_ptr<Resource::LoongTexture> texture_ { nullptr };
    Renderer::Renderer renderer_;
    Resource::LoongUniformBuffer basicUniforms_;

    std::shared_ptr<Core::LoongScene> scene_ { nullptr };
    Core::LoongCCamera* cameraComponent_ { nullptr };
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
    Loong::FS::ScopedDriver fsDriver(argv[0]);
    auto path = Loong::Foundation::LoongPathUtils::GetParent(argv[0]) + "/Resources";
    Loong::FS::LoongFileSystem::MountSearchPath(path);

    Loong::Resource::ScopedDriver resourceDriver;

    auto listener = Loong::Foundation::Logger::Get().SubscribeLog([](const Loong::Foundation::LogItem& logItem) {
        std::cout << "[" << logItem.level << "][" << logItem.location << "]: " << logItem.message << std::endl;
    });

    StartApp();

    return 0;
}