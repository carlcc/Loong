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
#include <iostream>

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
        scene_.reset(Core::LoongScene::CreateScene("SceneRoot").release());

        auto fireTexture = Resource::LoongResourceManager::GetTexture("Textures/fire.jpg");
        auto material = std::make_shared<Resource::LoongMaterial>();
        material->SetShaderByFile("Shaders/unlit.glsl");
        material->GetUniformsData()["u_DiffuseMap"] = fireTexture;

        auto cubeModel = Resource::LoongResourceManager::GetModel("Models/cube.fbx");
        auto* actor = Core::LoongScene::CreateActor("ActorCube").release();
        auto* modelRenderer = actor->AddComponent<Core::LoongCModelRenderer>();
        modelRenderer->SetModel(cubeModel);
        modelRenderer->SetMaterial(0, material);

        actor->SetParent(scene_.get());

        auto* cameraActor = Core::LoongScene::CreateActor("ActorCamera").release();
        cameraComponent_ = cameraActor->AddComponent<Core::LoongCCamera>();
        auto& cameraTransform = cameraActor->GetTransform();
        cameraTransform.SetPosition({ 0.0F, 1.0F, 12.0F });
        cameraTransform.LookAt(Math::Zero, Math::kUp);
        cameraActor->SetParent(scene_.get());

        UBO ubo;
        basicUniforms_.BufferData(&ubo, 1, Resource::LoongGpuBufferUsage::kStreamDraw); // Note: Must allocate memory first
        basicUniforms_.SetBindingPoint(0, sizeof(UBO));

        gApp->SubscribeUpdate(this, &LoongEditor::OnUpdate);
        gApp->SubscribeRender(this, &LoongEditor::OnRender);
    }

    void OnUpdate()
    {
        clock_.Update();

        auto& input = gApp->GetInputManager();

        if (ImGui::Begin("MainWindow")) {

            if (ImGui::Button("PressMe")) {
                OnPressButton();
            }
            ImGui::Image((void*)intptr_t(texture_->GetId()), { 100.F, 100.F });
            for (int i = 0; i < 6; ++i) {
                int btn = int(App::LoongMouseButton::kButton1) + i;
                if (input.IsMouseButtonPressed(App::LoongMouseButton(btn))) {
                    ImGui::Text("%s", Foundation::Format("Pressed button {}", btn).c_str());
                }
            }

            for (int i = 0; i < 26; ++i) {
                int key = int(App::LoongKeyCode::kKeyA) + i;
                if (input.IsKeyPressed(App::LoongKeyCode(key))) {
                    ImGui::Text("%s", Foundation::Format("Pressed key {}", char(key)).c_str());
                }
            }

            if (auto* cubeActor = scene_->GetChildByName("ActorCube"); cubeActor != nullptr) {
                cubeActor->GetTransform().Rotate(Math::kUp, clock_.DeltaTime());
                cubeActor->GetTransform().SetPosition({ std::sinf(clock_.ElapsedTime()) * 2.0F, 0.0F, std::cosf(clock_.ElapsedTime()) * 2.0F });
            }
        }
        ImGui::End();
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

    void OnPressButton()
    {
        for (auto& c : clearColor_) {
            c = 1.0F * rand() / RAND_MAX;
        }
    }

    float clearColor_[4] { 0.3F, 0.4F, 0.5F, 1.0F };

    Foundation::LoongClock clock_;

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