//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#include "LoongWindow/LoongWindow.h"
#include "LoongFoundation/LoongLogger.h"
#include "LoongWindow/LoongApplication.h"
#include <GLFW/glfw3.h>
#include <mutex>
#include <queue>
#include <set>
#include <thread>
#include <vector>

namespace Loong::Window {

class LoongWindow::Impl {
public:
    explicit Impl(LoongWindow* self, const WindowConfig& config)
    {
        // Decide GL+GLSL versions
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
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
            LOONG_ERROR("Create glfwWindow failed");
            exit(-1);
        }
        glfwSetWindowUserPointer(glfwWindow_, self);
        self_ = self;

        glfwSetFramebufferSizeCallback(glfwWindow_, [](GLFWwindow* win, int w, int h) {
            auto* ptr = glfwGetWindowUserPointer(win);
            reinterpret_cast<LoongWindow*>(ptr)->FrameBufferResizeSignal_.emit(w, h);
        });
        glfwSetWindowSizeCallback(glfwWindow_, [](GLFWwindow* win, int w, int h) {
            auto* ptr = glfwGetWindowUserPointer(win);
            reinterpret_cast<LoongWindow*>(ptr)->WindowResizeSignal_.emit(w, h);
        });
        glfwSetKeyCallback(glfwWindow_, [](GLFWwindow* win, int key, int scancode, int action, int mod) {
            auto* ptr = glfwGetWindowUserPointer(win);
            (void)scancode;
            auto* self = reinterpret_cast<LoongWindow*>(ptr);
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
            auto* self = reinterpret_cast<LoongWindow*>(ptr);
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
            auto* self = reinterpret_cast<LoongWindow*>(ptr);
            self->impl_->input_.SetMousePosition(float(x), float(y));
            self->CursorPosSignal_.emit(x, y);
        });
        glfwSetWindowPosCallback(glfwWindow_, [](GLFWwindow* win, int x, int y) {
            auto* ptr = glfwGetWindowUserPointer(win);
            reinterpret_cast<LoongWindow*>(ptr)->WindowPosSignal_.emit(x, y);
        });
        glfwSetWindowIconifyCallback(glfwWindow_, [](GLFWwindow* win, int iconified) {
            auto* ptr = glfwGetWindowUserPointer(win);
            reinterpret_cast<LoongWindow*>(ptr)->WindowIconifySignal_.emit(iconified == GLFW_TRUE);
        });
        glfwSetWindowCloseCallback(glfwWindow_, [](GLFWwindow* win) {
            auto* ptr = glfwGetWindowUserPointer(win);
            reinterpret_cast<LoongWindow*>(ptr)->WindowCloseSignal_.emit();
        });

        // InitImGui();
    }
    ~Impl()
    {
        if (glfwWindow_ != nullptr) {
            glfwDestroyWindow(glfwWindow_);
            glfwWindow_ = nullptr;
        }
    }

    //void InitImGui()
    //{
    //    IMGUI_CHECKVERSION();
    //    ImGui::CreateContext();
    //    ImGuiIO& io = ImGui::GetIO();
    //    (void)io;
    //    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    //    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    //    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable; // Enable Docking
    //    // io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable; // Enable Multi-Viewport / Platform Windows
    //    //io.ConfigViewportsNoAutoMerge = true;
    //    //io.ConfigViewportsNoTaskBarIcon = true;

    //    // Setup Dear ImGui style
    //    ImGui::StyleColorsDark();
    //    //ImGui::StyleColorsClassic();

    //    // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
    //    ImGuiStyle& style = ImGui::GetStyle();
    //    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
    //        style.WindowRounding = 0.0f;
    //        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    //    }

    //    // Setup Platform/Renderer bindings
    //    // const char* glsl_version = "#version 150";
    //    ImGui_ImplGlfw_InitForDiligentEngine(glfwWindow_, true);

    //    // ImGui_ImplOpenGL3_Init(glsl_version);

    //    // Load Fonts
    //    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
    //    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
    //    // - If the file cannot be loaded, the function will return NULL. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
    //    // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
    //    // - Read 'docs/FONTS.txt' for more instructions and details.
    //    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
    //    //io.Fonts->AddFontDefault();
    //    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
    //    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
    //    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
    //    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/ProggyTiny.ttf", 10.0f);
    //    //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
    //    // io.Fonts->AddFontFromFileTTF("resource/fonts/wqymicroheimono.ttf", 16.0f);
    //    //IM_ASSERT(font != NULL);
    //}

    const LoongInput& GetInputManager() const
    {
        return input_;
    }

    GLFWwindow* GetGlfwWindow() const
    {
        return glfwWindow_;
    }

    void SetVisible(bool b)
    {
        if (b) {
            glfwShowWindow(glfwWindow_);
        } else {
            glfwHideWindow(glfwWindow_);
        }
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
        return GetMouseMode() == LoongWindow::MouseMode::kNormal;
    }

    MouseMode GetMouseMode() const
    {
        auto mode = glfwGetInputMode(glfwWindow_, GLFW_CURSOR);
        if (mode == GLFW_CURSOR_NORMAL) {
            return LoongWindow::MouseMode::kNormal;
        }
        if (mode == GLFW_CURSOR_HIDDEN) {
            return LoongWindow::MouseMode::kHidden;
        }
        return LoongWindow::MouseMode::kDisabled;
    }

    void SetMouseMode(MouseMode b)
    {
        auto oldMode = GetMouseMode();
        if (oldMode == b) {
            return;
        }
        int modes[] = { GLFW_CURSOR_NORMAL, GLFW_CURSOR_HIDDEN, GLFW_CURSOR_DISABLED };
        glfwSetInputMode(glfwWindow_, GLFW_CURSOR, modes[b]);
        if (oldMode == LoongWindow::MouseMode::kDisabled && b == LoongWindow::MouseMode::kNormal) {
            // from hidden to normal. Since the mouse pos while it was disabled does not equals to our latest mouse pos.
            // We need to reset the mouse pos and clear the mouse delta
            double x, y;
            glfwGetCursorPos(glfwWindow_, &x, &y);
            // set mouse position twice to clear the delta to Zero
            input_.SetMousePosition(float(x), float(y));
            input_.SetMousePosition(float(x), float(y));
        }
    }

    void GetWindowSize(int& width, int& height) const
    {
        glfwGetWindowSize(glfwWindow_, &width, &height);
    }

private:
    GLFWwindow* glfwWindow_ { nullptr };
    LoongWindow* self_ { nullptr };
    LoongInput input_ {};
    // TODO: Place it here, find a better place later
    std::function<void(LoongWindow*)> onDelete_ { nullptr };

    friend class LoongApplicationImpl;
};

class LoongApplicationImpl {
public:
    static LoongApplicationImpl& Get()
    {
        static LoongApplicationImpl wm;
        return wm;
    }

    ~LoongApplicationImpl()
    {
        for (auto* w : windowsToRun_) {
            DoDestroyWindow(w);
        }
        for (auto* w : newWindowsToRun_) {
            DoDestroyWindow(w);
        }
        windowsToDestroy_.clear();
        newWindowsToRun_.clear();
        windowsToRun_.clear();
    }

private:
    LoongApplicationImpl() = default;

public:
    void DoDestroyWindow(LoongWindow* w)
    {
        w->WindowDestroySignal_.emit();
        if (w->impl_->onDelete_) {
            w->impl_->onDelete_(w);
        }
        delete w;
    }

    int Run()
    {
        mainThreadId_ = std::this_thread::get_id();
        while (true) {
            if (!windowsToDestroy_.empty()) {
                auto toStop = std::move(windowsToDestroy_);
                for (auto w : toStop) {
                    DoDestroyWindow(w);
                    windowsToRun_.erase(w);
                }
            }
            if (!newWindowsToRun_.empty()) {
                for (auto* w : newWindowsToRun_) {
                    windowsToRun_.insert(w);
                }
                newWindowsToRun_.clear();
            }
            if (windowsToRun_.empty()) {
                break;
            }

            for (auto* win : windowsToRun_) {
                win->impl_->input_.BeginFrame();
            }
            glfwPollEvents();
            for (auto* win : windowsToRun_) {
                win->BeginFrameSignal_.emit();
            }
            for (auto* win : windowsToRun_) {
                win->UpdateSignal_.emit();
            }
            for (auto* win : windowsToRun_) {
                win->RenderSignal_.emit();
            }
            for (auto* win : windowsToRun_) {
                win->PresentSignal_.emit();
            }
            RunTasks();
        }
        return 0;
    }

    void RunInMainThread(LoongApplication::Task&& task)
    {
        std::unique_lock<std::mutex> lck(tasksMutex_);
        tasks_.push(std::move(task));
        ++tasksCount_;
    }

    void RunTasks()
    {
        while (tasksCount_ > 0) {
            LoongApplication::Task task { nullptr };
            {
                std::unique_lock<std::mutex> lck(tasksMutex_);
                task = std::move(tasks_.front());
                tasks_.pop();
                --tasksCount_;
            }
            task();
        }
    }

    bool IsInMainThread()
    {
        auto id = std::this_thread::get_id();
        return mainThreadId_ == id;
    }

    void AddWindow(LoongWindow* window)
    {
        assert(windowsToRun_.count(window) == 0);
        glfwSetWindowShouldClose(window->impl_->glfwWindow_, 0);
        {
            double x, y;
            glfwGetCursorPos(window->impl_->glfwWindow_, &x, &y);
            // set mouse position twice to clear the delta to Zero
            window->impl_->input_.SetMousePosition(float(x), float(y));
            window->impl_->input_.SetMousePosition(float(x), float(y));
        }
        newWindowsToRun_.insert(window);
    }

    LoongWindow* CreateWindow(const WindowConfig& cfg, std::function<void(LoongWindow*)>&& onDelete)
    {
        auto* win = new LoongWindow(cfg);
        SetDeleterForWindow(win, std::move(onDelete));
        AddWindow(win);
        return win;
    }

    void SetDeleterForWindow(LoongWindow* win, std::function<void(LoongWindow*)>&& onDelete)
    {
        win->impl_->onDelete_ = std::move(onDelete);
    }

    void DestroyWindow(LoongWindow* window)
    {
        windowsToDestroy_.insert(window);
    }

    void DestroyAllWindows()
    {
        for (auto& w : windowsToRun_) {
            windowsToDestroy_.insert(w);
        }
    }

    std::set<LoongWindow*> windowsToRun_ {};
    std::set<LoongWindow*> newWindowsToRun_ {};
    std::set<LoongWindow*> windowsToDestroy_ {};

    std::queue<LoongApplication::Task> tasks_ {};
    size_t tasksCount_ { 0 };
    std::mutex tasksMutex_ {};
    std::thread::id mainThreadId_ {};
};

LoongWindow* LoongApplication::CreateWindow(const WindowConfig& cfg, std::function<void(LoongWindow*)>&& onDelete)
{
    return LoongApplicationImpl::Get().CreateWindow(cfg, std::move(onDelete));
}

int LoongApplication::Run()
{
    return LoongApplicationImpl::Get().Run();
}

void LoongApplication::DestroyWindow(LoongWindow* win)
{
    LoongApplicationImpl::Get().DestroyWindow(win);
}

void LoongApplication::DestroyAllWindows()
{
    LoongApplicationImpl::Get().DestroyAllWindows();
}

void LoongApplication::SetDeleterForWindow(LoongWindow* win, std::function<void(LoongWindow*)>&& onDelete)
{
    LoongApplicationImpl::Get().SetDeleterForWindow(win, std::move(onDelete));
}

void LoongApplication::RunInMainThread(Task&& task)
{
    LoongApplicationImpl::Get().RunInMainThread(std::move(task));
}

bool LoongApplication::IsInMainThread()
{
    return LoongApplicationImpl::Get().IsInMainThread();
}

LoongWindow::LoongWindow(const WindowConfig& config)
    : impl_(new Impl(this, config))
{
}

LoongWindow::~LoongWindow()
{
    delete impl_;
}

void LoongWindow::SetVisible(bool visible)
{
    impl_->SetVisible(visible);
}

void LoongWindow::GetFramebufferSize(int& width, int& height) const
{
    impl_->GetFramebufferSize(width, height);
}

void LoongWindow::SetTitle(const char* title)
{
    impl_->SetTitle(title);
}
void LoongWindow::SetShouldClose(bool b)
{
    impl_->SetShouldClose(b);
}

bool LoongWindow::IsMouseVisible() const
{
    return impl_->IsMouseVisible();
}
LoongWindow::MouseMode LoongWindow::GetMouseMode() const
{
    return impl_->GetMouseMode();
}

void LoongWindow::SetMouseMode(LoongWindow::MouseMode b)
{
    impl_->SetMouseMode(b);
}

const LoongInput& LoongWindow::GetInputManager() const
{
    return impl_->GetInputManager();
}

void LoongWindow::GetWindowSize(int& width, int& height) const
{
    impl_->GetWindowSize(width, height);
}

void LoongWindow::GetFrameBufferSize(int& width, int& height) const
{
    impl_->GetFramebufferSize(width, height);
}

GLFWwindow* LoongWindow::GetGlfwWindow() const
{
    return impl_->GetGlfwWindow();
}

}
