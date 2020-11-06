#include "LoongFoundation/LoongClock.h"
#include "LoongFoundation/LoongLogger.h"
#include "LoongRHI/Driver.h"
#include "LoongRHI/LoongRHIManager.h"
#include "LoongWindow/Driver.h"
#include "LoongWindow/LoongApplication.h"
#include "LoongWindow/LoongWindow.h"
#include <GLFW/glfw3.h>
#include <GraphicsUtilities.h>
#include <LoongAsset/LoongMesh.h>
#include <LoongAsset/LoongModel.h>
#include <LoongFileSystem/Driver.h>
#include <LoongFileSystem/LoongFileSystem.h>
#include <LoongFoundation/Driver.h>
#include <LoongFoundation/LoongAssert.h>
#include <LoongFoundation/LoongFormat.h>
#include <LoongFoundation/LoongPathUtils.h>
#include <LoongFoundation/LoongThreadPool.h>
#include <LoongFoundation/LoongTransform.h>
#include <LoongGui/LoongGuiButton.h>
#include <LoongGui/LoongGuiImage.h>
#include <LoongGui/LoongGuiWindow.h>
#include <LoongGui/LoongImGuiIntegration.h>
#include <LoongResource/Driver.h>
#include <LoongResource/LoongGpuMesh.h>
#include <LoongResource/LoongGpuModel.h>
#include <LoongResource/LoongMaterial.h>
#include <LoongResource/LoongResourceManager.h>
#include <LoongResource/LoongTexture.h>
#include <LoongResource/loader/LoongMaterialLoader.h>
#include <cassert>
#include <imgui.h>
#include <iostream>

namespace Loong {

struct UniformConstants {
    RHI::float4x4 ub_MVP;
    RHI::float4x4 ub_Model;
    RHI::float4x4 ub_View;
    RHI::float4x4 ub_Projection;
    RHI::float3 ub_ViewPos;
    float ub_Time;
};

// align to 4*sizeof(float)
struct Light {
    RHI::float3 pos;
    float lightType;

    RHI::float3 dir;
    float falloffRadius;

    RHI::float3 color;
    float intencity;

    float innerAngle;
    float outerAngle;
    RHI::float2 padding1_;
};

#define MAX_LIGHT_COUNT 32

struct PSLightUniforms {
    float ub_LightsCount;
    RHI::float3 padding1_; // align to vec4
    Light ub_Lights[MAX_LIGHT_COUNT];
};

struct PSMaterialUniforms {
    RHI::float2 ub_TextureOffset;
    RHI::float2 ub_TextureTiling;
    RHI::float3 ub_Albedo;
    float ub_Metallic;
    RHI::float3 ub_Reflectance;
    float ub_Roughness;
    RHI::float3 ub_Emissive;
    float ub_EmissiveFactor;
    float ub_ClearCoat;
    float ub_ClearCoatRoughness;
};

RHI::float4x4 Mat4ToFloat4x4(const Math::Matrix4& m)
{
    RHI::float4x4 result;
    memcpy(result.m, &m[0][0], sizeof(RHI::float4x4));
    return result;
}

RHI::float3 Vec3ToFloat3(const Math::Vector3& v)
{
    return { v.x, v.y, v.z };
}

class LoongEditor : public Foundation::LoongHasSlots {
public:
    std::shared_ptr<Resource::LoongGpuModel> model_ { nullptr };
    Window::LoongWindow* window_ { nullptr };
    std::shared_ptr<Gui::LoongImGuiIntegration> imgui_ { nullptr };
    Gui::LoongGuiWindow guiWindow_ {};

    bool Initialize(Window::LoongWindow* window, RHI::RefCntAutoPtr<RHI::ISwapChain> swapChain)
    {
        window_ = window;

        swapChain_ = swapChain;
        window->SubscribeUpdate(this, &LoongEditor::OnUpdate);
        window->SubscribeRender(this, &LoongEditor::OnRender);
        window->SubscribePresent(this, &LoongEditor::OnPresent);
        window->SubscribeFrameBufferResize(this, &LoongEditor::OnFrameBufferResize);
        window->SubscribeWindowClose(this, &LoongEditor::OnClose);
        window->GetFramebufferSize(frameBufferWidth_, frameBufferHeight_);
        OnFrameBufferResize(frameBufferWidth_, frameBufferHeight_);

        clock_.Reset();
        vsConstants_ = RHI::LoongRHIManager::CreateUniformBuffer("VS constants CB", sizeof(UniformConstants));
        psLightUniforms_ = RHI::LoongRHIManager::CreateUniformBuffer("VS constants CB", sizeof(PSLightUniforms));
        psMaterialUniforms_ = RHI::LoongRHIManager::CreateUniformBuffer("VS constants CB", sizeof(PSMaterialUniforms));

        CreatePSO();
        InitResources();
        cameraTransform_.SetPosition({ 0, 0, -4 });

        imgui_ = std::make_shared<Gui::LoongImGuiIntegration>(window_->GetGlfwWindow(), RHI::LoongRHIManager::GetDevice(), swapChain_);
        auto btn = guiWindow_.AddChild<Gui::LoongGuiButton>();
        btn->SetLabel("A Gui Button");
        btn->SubscribeOnClicked(this, &LoongEditor::OnButtonClicked);

        return true;
    }

    void OnButtonClicked(Gui::LoongGuiWidget* btn)
    {
        static int sCount = 1;
        btn->SetLabel(Foundation::Format("You clicked me for {} times", sCount++));
    }

    void CreatePSO()
    {
        std::string shaderSourceCode;
        shaderSourceCode.resize(64000);
        auto actualCount = FS::LoongFileSystem::LoadFileContent("/Shaders/PlayGround.hlsl", shaderSourceCode.data(), shaderSourceCode.size());
        shaderSourceCode.resize(actualCount);

        RHI::ShaderMacroHelper vsMacros;
        vsMacros.AddShaderMacro("IS_VERTEX_SHADER", "1");
        RHI::ShaderCreateInfo vs;
        vs.SourceLanguage = RHI::SHADER_SOURCE_LANGUAGE_HLSL;
        vs.UseCombinedTextureSamplers = true;
        vs.Desc.ShaderType = RHI::SHADER_TYPE_VERTEX;
        vs.EntryPoint = "main";
        vs.Desc.Name = "Cube vertex shader";
        vs.Macros = vsMacros;
        vs.Source = shaderSourceCode.c_str();

        RHI::ShaderMacroHelper psMacros;
        psMacros.AddShaderMacro("IS_PIXEL_SHADER", "1");
        RHI::ShaderCreateInfo ps;
        ps.SourceLanguage = RHI::SHADER_SOURCE_LANGUAGE_HLSL;
        ps.UseCombinedTextureSamplers = true;
        ps.Desc.ShaderType = RHI::SHADER_TYPE_PIXEL;
        ps.EntryPoint = "main";
        ps.Desc.Name = "Cube pixel shader";
        ps.Macros = psMacros;
        ps.Source = shaderSourceCode.c_str();

        RHI::LayoutElement layoutElements[] {
            RHI::LayoutElement { 0, 0, 3, RHI::VT_FLOAT32, false },
            RHI::LayoutElement { 1, 0, 2, RHI::VT_FLOAT32, false },
            RHI::LayoutElement { 2, 0, 2, RHI::VT_FLOAT32, false },
            RHI::LayoutElement { 3, 0, 3, RHI::VT_FLOAT32, false },
            RHI::LayoutElement { 4, 0, 3, RHI::VT_FLOAT32, false },
            RHI::LayoutElement { 5, 0, 3, RHI::VT_FLOAT32, false },
        };
        RHI::InputLayoutDesc inputLayout {};
        inputLayout.LayoutElements = layoutElements;
        inputLayout.NumElements = _countof(layoutElements);

        RHI::PipelineResourceLayoutDesc resourceLayout;
        resourceLayout.DefaultVariableType = RHI::SHADER_RESOURCE_VARIABLE_TYPE_STATIC;
        RHI::ShaderResourceVariableDesc shaderVariables[] {
            { RHI::SHADER_TYPE_PIXEL, "g_Albedo", RHI::SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE },
            { RHI::SHADER_TYPE_PIXEL, "g_Normal", RHI::SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE },
            { RHI::SHADER_TYPE_PIXEL, "g_Roughness", RHI::SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE },
            { RHI::SHADER_TYPE_PIXEL, "g_Metallic", RHI::SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE },
            { RHI::SHADER_TYPE_PIXEL, "g_Emissive", RHI::SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE },
        };

        RHI::SamplerDesc samplerConfig {
            RHI::FILTER_TYPE_LINEAR, RHI::FILTER_TYPE_LINEAR, RHI::FILTER_TYPE_LINEAR,
            RHI::TEXTURE_ADDRESS_WRAP, RHI::TEXTURE_ADDRESS_WRAP, RHI::TEXTURE_ADDRESS_WRAP
        };
        RHI::ImmutableSamplerDesc shaderSamplers[] {
            { RHI::SHADER_TYPE_PIXEL, "g_Albedo", samplerConfig },
            { RHI::SHADER_TYPE_PIXEL, "g_Normal", samplerConfig },
            { RHI::SHADER_TYPE_PIXEL, "g_Roughness", samplerConfig },
            { RHI::SHADER_TYPE_PIXEL, "g_Metallic", samplerConfig },
            { RHI::SHADER_TYPE_PIXEL, "g_Emissive", samplerConfig },
        };
        resourceLayout.Variables = shaderVariables;
        resourceLayout.NumVariables = _countof(shaderVariables);
        resourceLayout.ImmutableSamplers = shaderSamplers;
        resourceLayout.NumImmutableSamplers = _countof(shaderSamplers);

        pso_ = RHI::LoongRHIManager::CreateGraphicsPSOForCurrentSwapChain(swapChain_, "TexturedCube", vs, ps, inputLayout, resourceLayout, true, Diligent::CULL_MODE_BACK);
        if (pso_ == nullptr) {
            LOONG_ERROR("Create pso failed");
            return;
        }
        pso_->GetStaticVariableByName(RHI::SHADER_TYPE_VERTEX, "Constants")->Set(vsConstants_);
        auto var = pso_->GetStaticVariableByName(RHI::SHADER_TYPE_PIXEL, "PSMaterialUniforms");
        if (var != nullptr) {
            var->Set(psMaterialUniforms_);
        }
        var = pso_->GetStaticVariableByName(RHI::SHADER_TYPE_PIXEL, "PSLightUniforms");
        if (var != nullptr) {
            var->Set(psLightUniforms_);
        }
        pso_->CreateShaderResourceBinding(&srb_, true);
        if (albedoTexture_ != nullptr) {
            SetShaderResourceBinding();
        }
    }

    void SetShaderResourceBinding()
    {
        std::shared_ptr<Resource::LoongTexture> textures[] {
            albedoTexture_,
            normalTexture_,
            roughnessTexture_,
            metallicTexture_,
            emissiveTexture_,
        };
        const char* shaderVarNames[] {
            "g_Albedo",
            "g_Normal",
            "g_Roughness",
            "g_Metallic",
            "g_Emissive",
        };
        for (int i = 0; i < _countof(shaderVarNames); ++i) {
            auto tex = textures[i];
            if (tex == nullptr || tex->GetTexture() == nullptr) {
                continue;
            }
            auto var = srb_->GetVariableByName(RHI::SHADER_TYPE_PIXEL, shaderVarNames[i]);
            if (var != nullptr) {
                var->Set(tex->GetTexture()->GetDefaultView(RHI::TEXTURE_VIEW_SHADER_RESOURCE));
                auto img = guiWindow_.AddChild<Gui::LoongGuiImage>();
                img->SetTexture(tex);
            }
        }
    }

    void InitResources()
    {
        LOONG_INFO("Loading texture...");
        Resource::LoongResourceManager::GetMaterialAsync("/Materials/Test.lgmtl").Then([this](const auto& mtlTask) {
            LOONG_ASSERT(!Window::LoongApplication::IsInMainThread(), "");
            auto mtl = mtlTask.GetFuture().GetValue();
            albedoTexture_ = mtl->GetAlbedoMap();
            normalTexture_ = mtl->GetNormalMap();
            roughnessTexture_ = mtl->GetRoughnessMap();
            metallicTexture_ = mtl->GetMetallicMap();
            emissiveTexture_ = mtl->GetEmissiveMap();

            Window::LoongApplication::RunInMainThread([&]() {
                LOONG_ASSERT(Window::LoongApplication::IsInMainThread(), "");
                if (srb_ != nullptr) {
                    SetShaderResourceBinding();
                }
            });
        });

        Resource::LoongResourceManager::GetModelAsync("/Models/DamagedHelmet.lgmdl").Then([this](const auto& modelTask) {
            LOONG_ASSERT(!Window::LoongApplication::IsInMainThread(), "");
            model_ = modelTask.GetFuture().GetValue();
        });
    }

    void OnUpdate()
    {
        LOONG_ASSERT(Window::LoongApplication::IsInMainThread(), "");
        clock_.Update();
        imgui_->NewFrame();
        guiWindow_.Draw();

        if (window_->GetInputManager().IsKeyReleaseEvent(Window::LoongKeyCode::kKeyM)) {
            auto* mat = new Resource::LoongMaterial;
            Resource::LoongMaterialLoader::Write("/Materials/test.lgmtl", mat);
        }
        if (window_->GetInputManager().IsKeyReleaseEvent(Window::LoongKeyCode::kKeyR)) {
            CreatePSO();
        }
        auto euler = Math::QuatToEuler(cameraTransform_.GetRotation());
        const auto& input = window_->GetInputManager();

        if (input.IsMouseButtonPressed(Window::LoongMouseButton::kButtonRight)) {
            euler.x += input.GetMouseDelta().y / 200.0F; // pitch
            euler.y += input.GetMouseDelta().x / 200.0F; // yaw
            euler.z = 0.0F;
            euler.x = Math::Clamp(euler.x, -float(Math::HalfPi) + 0.1F, float(Math::HalfPi) - 0.1F);

            cameraTransform_.SetRotation(Math::EulerToQuat(euler));
            window_->SetMouseMode(Window::LoongWindow::MouseMode::kDisabled);
        } else {
            window_->SetMouseMode(Window::LoongWindow::MouseMode::kNormal);
        }

        {
            Math::Vector3 dir { 0.0F };
            if (input.IsKeyPressed(Window::LoongKeyCode::kKeyW)) {
                dir += cameraTransform_.GetForward();
            }
            if (input.IsKeyPressed(Window::LoongKeyCode::kKeyS)) {
                dir -= cameraTransform_.GetForward();
            }
            if (input.IsKeyPressed(Window::LoongKeyCode::kKeyA)) {
                dir -= cameraTransform_.GetRight();
            }
            if (input.IsKeyPressed(Window::LoongKeyCode::kKeyD)) {
                dir += cameraTransform_.GetRight();
            }
            if (input.IsKeyPressed(Window::LoongKeyCode::kKeyE)) {
                dir += Math::kUp;
            }
            if (input.IsKeyPressed(Window::LoongKeyCode::kKeyQ)) {
                dir -= Math::kUp;
            }
            if (input.IsKeyPressed(Window::LoongKeyCode::kKeyLeftShift) || input.IsKeyPressed(Window::LoongKeyCode::kKeyRightShift)) {
                dir *= 5.0F;
            }
            cameraTransform_.Translate(dir * clock_.DeltaTime() * 2.0F);
        }

        {
            if (input.IsKeyReleaseEvent(Window::LoongKeyCode::kKey1)) {
                cameraTransform_.SetPosition({ 0, 0, -4 });
            }
            if (input.IsKeyReleaseEvent(Window::LoongKeyCode::kKey2)) {
                cameraTransform_.SetRotation(Math::Identity);
            }
            auto forward = cameraTransform_.GetForward();
            clearColor_[0] = forward.r;
            clearColor_[1] = forward.g;
            clearColor_[2] = forward.b;
        }

        using float4x4 = RHI::float4x4;

        uniforms_.ub_Model = float4x4::Identity(); // float4x4::RotationY(clock_.ElapsedTime()) * float4x4::RotationX(-RHI::PI_F * 0.1f);
        // Camera is at (0, 0, -4) looking along the Z axis
        uniforms_.ub_View = Mat4ToFloat4x4(Math::Inverse(cameraTransform_.GetTransformMatrix()));
        uniforms_.ub_Projection = float4x4::Projection(RHI::PI_F / 4.0f, frameBufferAspect_, 0.001f, 1000.f, false);
        uniforms_.ub_ViewPos = Vec3ToFloat3(cameraTransform_.GetPosition());
        uniforms_.ub_MVP = uniforms_.ub_Model * uniforms_.ub_View * uniforms_.ub_Projection;
        uniforms_.ub_Time = clock_.ElapsedTime();

        imgui_->EndFrame();
    }

    void OnRender()
    {
        LOONG_ASSERT(Window::LoongApplication::IsInMainThread(), "");
        auto immediateContext = RHI::LoongRHIManager::GetImmediateContext();
        auto swapChain = swapChain_;

        assert(immediateContext != nullptr);
        assert(swapChain != nullptr);

        RHI::ITextureView* pRTV = swapChain->GetCurrentBackBufferRTV();
        RHI::ITextureView* pDSV = swapChain->GetDepthBufferDSV();
        immediateContext->SetRenderTargets(1, &pRTV, pDSV, RHI::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

        // Let the engine perform required state transitions
        immediateContext->ClearRenderTarget(pRTV, clearColor_, RHI::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
        immediateContext->ClearDepthStencil(pDSV, RHI::CLEAR_DEPTH_FLAG, 1.f, 0, RHI::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

        if (albedoTexture_ != nullptr && model_ != nullptr && pso_ != nullptr) {

            {
                // Map the buffer and write current world-view-projection matrix
                RHI::MapHelper<UniformConstants> uniforms(immediateContext, vsConstants_, RHI::MAP_WRITE, RHI::MAP_FLAG_DISCARD);
                uniforms->ub_View = uniforms_.ub_View.Transpose();
                uniforms->ub_Projection = uniforms_.ub_Projection.Transpose();
                uniforms->ub_Model = uniforms_.ub_Model.Transpose();
                uniforms->ub_MVP = uniforms_.ub_MVP.Transpose();
                uniforms->ub_ViewPos = uniforms_.ub_ViewPos;
                uniforms->ub_Time = uniforms_.ub_Time;
            }

            {
                // Map the buffer and write current world-view-projection matrix
                RHI::MapHelper<PSMaterialUniforms> uniforms(immediateContext, psMaterialUniforms_, RHI::MAP_WRITE, RHI::MAP_FLAG_DISCARD);
                uniforms->ub_ClearCoat = 0.0F;
                uniforms->ub_Reflectance = { 1.0F, 1.0F, 1.0F };
                uniforms->ub_TextureOffset = { 0.0F, 0.0F };
                uniforms->ub_TextureTiling = { 1.0F, 1.0F };
                uniforms->ub_EmissiveFactor = 1.0F;
            }

            {
                // Map the buffer and write current world-view-projection matrix
                RHI::MapHelper<PSLightUniforms> uniforms(immediateContext, psLightUniforms_, RHI::MAP_WRITE, RHI::MAP_FLAG_DISCARD);
                uniforms->ub_LightsCount = 1.0F;
                uniforms->ub_Lights[0].color = { 1.0F, 1.0F, 1.0F };
                uniforms->ub_Lights[0].falloffRadius = 20.0F;
                uniforms->ub_Lights[0].lightType = 0.0F;
                uniforms->ub_Lights[0].pos = uniforms_.ub_ViewPos;
                uniforms->ub_Lights[0].intencity = 19.0F;
                uniforms->ub_Lights[0].dir = uniforms_.ub_ViewPos;
            }

            immediateContext->SetPipelineState(pso_);
            immediateContext->CommitShaderResources(srb_, RHI::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
            uint32_t offset = 0;
            for (auto* mesh : model_->GetMeshes()) {
                RHI::IBuffer* buffers[] = { mesh->GetVBO() };
                immediateContext->SetVertexBuffers(0, 1, buffers, &offset, RHI::RESOURCE_STATE_TRANSITION_MODE_TRANSITION, RHI::SET_VERTEX_BUFFERS_FLAG_RESET);
                immediateContext->SetIndexBuffer(mesh->GetIBO(), 0, RHI::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

                RHI::DrawIndexedAttribs drawAttrs;

                drawAttrs.IndexType = RHI::VT_UINT32;
                drawAttrs.NumIndices = mesh->GetIndexCount();
                drawAttrs.Flags = Diligent::DRAW_FLAG_VERIFY_ALL;
                immediateContext->DrawIndexed(drawAttrs);
            }
        }

        imgui_->Render(RHI::LoongRHIManager::GetImmediateContext());
    }

    void OnPresent()
    {
        bool vsync = false;
        swapChain_->Present(vsync ? 1 : 0);
    }

    void OnFrameBufferResize(int w, int h)
    {
        frameBufferWidth_ = w;
        frameBufferHeight_ = h;
        frameBufferAspect_ = (float)w / (float)h;
        swapChain_->Resize(w, h);
    }

    void OnClose()
    {
        OnClose1();
    }

    virtual void OnClose1()
    {
        Window::LoongApplication::DestroyAllWindows();
    }

    Foundation::LoongClock clock_ {};
    RHI::RefCntAutoPtr<RHI::IPipelineState> pso_ { nullptr };
    RHI::RefCntAutoPtr<RHI::IBuffer> vsConstants_ { nullptr };
    RHI::RefCntAutoPtr<RHI::IBuffer> psLightUniforms_ { nullptr };
    RHI::RefCntAutoPtr<RHI::IBuffer> psMaterialUniforms_ { nullptr };
    RHI::RefCntAutoPtr<RHI::IShaderResourceBinding> srb_ { nullptr };
    std::shared_ptr<Resource::LoongTexture> albedoTexture_ { nullptr };
    std::shared_ptr<Resource::LoongTexture> normalTexture_ { nullptr };
    std::shared_ptr<Resource::LoongTexture> roughnessTexture_ { nullptr };
    std::shared_ptr<Resource::LoongTexture> metallicTexture_ { nullptr };
    std::shared_ptr<Resource::LoongTexture> emissiveTexture_ { nullptr };
    RHI::float4x4 worldViewProjMatrix_ {};
    int frameBufferWidth_ { 0 };
    int frameBufferHeight_ { 0 };
    float frameBufferAspect_ { 1.0F };
    RHI::RefCntAutoPtr<RHI::ISwapChain> swapChain_ { nullptr };
    float clearColor_[4] { 0.350f, 0.350f, 0.350f, 1.0f };

    Foundation::Transform cameraTransform_ {};

    UniformConstants uniforms_ {};
};

}

void StartApp(int argc, char** argv)
{
    Loong::Foundation::ScopedDriver foundationDriver;
    Loong::FS::ScopedDriver fsDriver(argv[0]);
    auto path = Loong::Foundation::LoongPathUtils::GetParent(argv[0]) + "/Resources";
    Loong::FS::LoongFileSystem::SetWriteDir(path);
    Loong::FS::LoongFileSystem::MountSearchPath(path);
    path = Loong::Foundation::LoongPathUtils::Normalize(argv[0]) + "/../../Resources";
    Loong::FS::LoongFileSystem::MountSearchPath(path);
    path = "/Users/chenchen02/gitrepo/Loong/Resources";

    Loong::Window::ScopedDriver appDriver;
    assert(bool(appDriver));

    Loong::Window::WindowConfig config {};
    config.title = "Play Ground";
    auto window = Loong::Window::LoongApplication::CreateWindow(config);

    Loong::RHI::ScopedDriver rhiDriver(window->GetGlfwWindow(), Loong::RHI::RENDER_DEVICE_TYPE_VULKAN);
    assert(bool(rhiDriver));

    Loong::Resource::ScopedDriver resourceDriver;
    assert(bool(resourceDriver));

    auto* ed = new Loong::LoongEditor;
    ed->Initialize(window, Loong::RHI::LoongRHIManager::GetPrimarySwapChain());
    Loong::Window::LoongApplication::SetDeleterForWindow(window, [ed](auto* w) {
        delete ed;
    });

    Loong::Window::LoongApplication::Run();
}

#ifdef ELIMINATE_CONSOLE_WINDOW
#ifdef APIENTRY
#undef APIENTRY
#endif
#include <Windows.h>
void Stealth()
{
    HWND Stealth;
    AllocConsole();
    Stealth = FindWindowA("ConsoleWindowClass", NULL);
    ShowWindow(Stealth, 0);
}
#endif

int main(int argc, char** argv)
{
#ifdef ELIMINATE_CONSOLE_WINDOW
    Stealth();
#endif
    auto listener = Loong::Foundation::Logger::Get().SubscribeLog([](const Loong::Foundation::LogItem& logItem) {
        std::cout << "[" << logItem.level << "]: " << logItem.message << " (" << logItem.location << ")" << std::endl;
    });

    StartApp(argc, argv);

    return 0;
}
