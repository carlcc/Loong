//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#include <glad/glad.h>

#include "LoongApp/LoongApp.h"
#include "LoongFoundation/LoongLogger.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>
#include <imgui.h>

namespace Loong::App {

class LoongApp::Impl {
public:
    explicit Impl(LoongApp* self, const WindowConfig& config)
    {
        // Decide GL+GLSL versions
#if __APPLE__
        // GL 3.2 + GLSL 150
        int majorVersion = 4;
        int minorVersion = 1;
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, majorVersion);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, minorVersion);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // 3.2+ only
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // Required on Mac
#else
        // GL 3.0 + GLSL 130
        int majorVersion = 4;
        int minorVersion = 3;
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, majorVersion);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, minorVersion);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // 3.2+ only
        //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
#endif
        glfwWindowHint(GLFW_RESIZABLE, config.resizable);
        glfwWindowHint(GLFW_DECORATED, config.decorated);
        glfwWindowHint(GLFW_FOCUSED, config.focused);
        glfwWindowHint(GLFW_MAXIMIZED, config.maximized);
        glfwWindowHint(GLFW_FLOATING, config.floating);
        glfwWindowHint(GLFW_VISIBLE, config.visible);
        glfwWindowHint(GLFW_AUTO_ICONIFY, config.autoIconify);
        glfwWindowHint(GLFW_REFRESH_RATE, config.refreshRate);
        glfwWindowHint(GLFW_SAMPLES, config.samples);
        glfwWindow_ = glfwCreateWindow(config.width, config.height, config.title, nullptr, nullptr);

        if (glfwWindow_ == nullptr) {
            LOONG_ERROR("Create glfwWindow failed, please make sure your GPU supports OpenGL {}.{}", majorVersion, minorVersion);
            exit(-1);
        }
        glfwSetWindowUserPointer(glfwWindow_, self);
        self_ = self;

        glfwSetFramebufferSizeCallback(glfwWindow_, [](GLFWwindow* win, int w, int h) {
            auto* ptr = glfwGetWindowUserPointer(win);
            reinterpret_cast<LoongApp*>(ptr)->FrameBufferResizeSignal_.emit(w, h);
        });
        glfwSetWindowSizeCallback(glfwWindow_, [](GLFWwindow* win, int w, int h) {
            auto* ptr = glfwGetWindowUserPointer(win);
            reinterpret_cast<LoongApp*>(ptr)->WindowResizeSignal_.emit(w, h);
        });
        glfwSetKeyCallback(glfwWindow_, [](GLFWwindow* win, int key, int scancode, int action, int mod) {
            auto* ptr = glfwGetWindowUserPointer(win);
            (void)scancode;
            auto* self = reinterpret_cast<LoongApp*>(ptr);
            switch (action) {
            case GLFW_PRESS:
                self->impl_->input_.SetIsKeyPressed(LoongKeyCode(key));
                self->impl_->input_.SetIsKeyPressEvent(LoongKeyCode(key));
                break;
            case GLFW_RELEASE:
                self->impl_->input_.SetIsKeyReleaseEvent(LoongKeyCode(key));
                self->impl_->input_.SetIsKeyReleased(LoongKeyCode(key));
                break;
            case GLFW_REPEAT:
                self->impl_->input_.SetIsKeyRepeatEvent(LoongKeyCode(key));
                break;
            default: // ignore
                break;
            }
            self->KeyBoardSignal_.emit(LoongKeyCode(key), LoongInputAction(action), mod);
        });
        glfwSetMouseButtonCallback(glfwWindow_, [](GLFWwindow* win, int button, int action, int mod) {
            auto* ptr = glfwGetWindowUserPointer(win);
            auto* self = reinterpret_cast<LoongApp*>(ptr);
            switch (action) {
            case GLFW_PRESS:
                self->impl_->input_.SetIsMouseButtonPressed(LoongMouseButton(button));
                self->impl_->input_.SetIsMouseButtonPressEvent(LoongMouseButton(button));
                {
                    double x, y;
                    glfwGetCursorPos(win, &x, &y);
                    self->impl_->input_.SetMouseDownPosition(float(x), float(y));
                }
                break;
            case GLFW_RELEASE:
                self->impl_->input_.SetIsMouseButtonReleased(LoongMouseButton(button));
                self->impl_->input_.SetIsMouseButtonReleaseEvent(LoongMouseButton(button));
                break;
            default: // ignore
                break;
            }
            self->MouseButtonSignal_.emit(LoongMouseButton(button), LoongInputAction(action), mod);
        });
        glfwSetCursorPosCallback(glfwWindow_, [](GLFWwindow* win, double x, double y) {
            auto* ptr = glfwGetWindowUserPointer(win);
            auto* self = reinterpret_cast<LoongApp*>(ptr);
            self->impl_->input_.SetMousePosition(float(x), float(y));
            self->CursorPosSignal_.emit(x, y);
        });
        glfwSetWindowPosCallback(glfwWindow_, [](GLFWwindow* win, int x, int y) {
            auto* ptr = glfwGetWindowUserPointer(win);
            reinterpret_cast<LoongApp*>(ptr)->WindowPosSignal_.emit(x, y);
        });
        glfwSetWindowIconifyCallback(glfwWindow_, [](GLFWwindow* win, int iconified) {
            auto* ptr = glfwGetWindowUserPointer(win);
            reinterpret_cast<LoongApp*>(ptr)->WindowIconifySignal_.emit(iconified == GLFW_TRUE);
        });
        glfwSetWindowCloseCallback(glfwWindow_, [](GLFWwindow* win) {
            auto* ptr = glfwGetWindowUserPointer(win);
            reinterpret_cast<LoongApp*>(ptr)->WindowCloseSignal_.emit();
        });

        glfwMakeContextCurrent(glfwWindow_);
        if (0 == gladLoadGL()) {
            LOONG_ERROR("Load OpenGL failed. Please make sure your GPU supports OpenGL {}.{}", majorVersion, minorVersion);
            exit(-1);
        }
        glfwSwapInterval(1);
        InitImGui();
    }
    ~Impl()
    {
        if (glfwWindow_ != nullptr) {
            glfwDestroyWindow(glfwWindow_);
            glfwWindow_ = nullptr;
        }
    }

    void InitImGui()
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

        // Setup Platform/Renderer bindings
        const char* glsl_version = "#version 150";
        ImGui_ImplGlfw_InitForOpenGL(glfwWindow_, true);
        ImGui_ImplOpenGL3_Init(glsl_version);

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

    int Run()
    {
        glfwSetWindowShouldClose(glfwWindow_, 0);

        {
            double x, y;
            glfwGetCursorPos(glfwWindow_, &x, &y);
            // set mouse position twice to clear the delta to Zero
            input_.SetMousePosition(float(x), float(y));
            input_.SetMousePosition(float(x), float(y));
        }
        while (!glfwWindowShouldClose(glfwWindow_)) {
            input_.BeginFrame();
            glfwPollEvents();
            int display_w, display_h;
            GetFramebufferSize(display_w, display_h);
            glViewport(0, 0, display_w, display_h);
            glClearColor(0.0F, 0.0F, 0.0F, 1.0F);
            glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            self_->BeginFrameSignal_.emit();

            self_->UpdateSignal_.emit();

            ImGui::Render();

            self_->RenderSignal_.emit();

            GetFramebufferSize(display_w, display_h);
            glViewport(0, 0, display_w, display_h);
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

            self_->LateUpdateSignal_.emit();

            glfwSwapBuffers(glfwWindow_);
        }
        return 0;
    }

    const LoongInput& GetInputManager() const
    {
        return input_;
    }

    void* GetGlfwWindow() const
    {
        return reinterpret_cast<void*>(glfwWindow_);
    }

    void GetFramebufferSize(int& width, int& height)
    {
        glfwGetFramebufferSize(glfwWindow_, &width, &height);
    }

    void SetTitle(const char* title)
    {
        glfwSetWindowTitle(glfwWindow_, title);
    }

    void SetShouldClose(bool b)
    {
        glfwSetWindowShouldClose(glfwWindow_, int(b));
    }

    bool IsMouseVisible() const
    {
        return GetMouseMode() == LoongApp::MouseMode::kNormal;
    }

    MouseMode GetMouseMode() const
    {
        auto mode = glfwGetInputMode(glfwWindow_, GLFW_CURSOR);
        if (mode == GLFW_CURSOR_NORMAL) {
            return LoongApp::MouseMode::kNormal;
        }
        if (mode == GLFW_CURSOR_HIDDEN) {
            return LoongApp::MouseMode::kHidden;
        }
        return LoongApp::MouseMode::kDisabled;
    }

    void SetMouseMode(MouseMode b)
    {
        int modes[] = { GLFW_CURSOR_NORMAL, GLFW_CURSOR_HIDDEN, GLFW_CURSOR_DISABLED };
        glfwSetInputMode(glfwWindow_, GLFW_CURSOR, modes[b]);
    }

    void GetWindowSize(int& width, int& height) const
    {
        glfwGetWindowSize(glfwWindow_, &width, &height);
    }

private:
    GLFWwindow* glfwWindow_ { nullptr };
    LoongApp* self_ { nullptr };
    LoongInput input_ {};
};

LoongApp::LoongApp(const LoongApp::WindowConfig& config)
    : impl_(new Impl(this, config))
{
}

LoongApp::~LoongApp()
{
    delete impl_;
}

int LoongApp::Run()
{
    return impl_->Run();
}

void LoongApp::GetFramebufferSize(int& width, int& height) const
{
    impl_->GetFramebufferSize(width, height);
}

void LoongApp::SetTitle(const char* title)
{
    impl_->SetTitle(title);
}
void LoongApp::SetShouldClose(bool b)
{
    impl_->SetShouldClose(b);
}

bool LoongApp::IsMouseVisible() const
{
    return impl_->IsMouseVisible();
}
LoongApp::MouseMode LoongApp::GetMouseMode() const
{
    return impl_->GetMouseMode();
}

void LoongApp::SetMouseMode(LoongApp::MouseMode b)
{
    impl_->SetMouseMode(b);
}

const LoongInput& LoongApp::GetInputManager() const
{
    return impl_->GetInputManager();
}

void LoongApp::GetWindowSize(int& width, int& height) const
{
    impl_->GetWindowSize(width, height);
}

void LoongApp::GetFrameBufferSize(int& width, int& height) const
{
    impl_->GetFramebufferSize(width, height);
}

}
