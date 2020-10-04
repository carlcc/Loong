//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#pragma once
#include "LoongFoundation/LoongMacros.h"
#include "LoongFoundation/LoongSigslotHelper.h"
#include "LoongWindow/LoongInput.h"

struct GLFWwindow;
namespace Loong::Window {

struct WindowConfig {
    int width { 640 };
    int height { 480 };
    const char* title { "" };
    int resizable { 1 };
    int decorated { 1 };
    int focused { 1 };
    int maximized { 0 };
    int floating { 0 };
    int visible { 1 };
    int autoIconify { 0 };
    int refreshRate { 60 };
    int samples { 0 };
};

class LoongWindow {
public:
    enum MouseMode {
        kNormal = 0,
        kHidden,
        kDisabled,
    };

    LoongWindow(const LoongWindow&) = delete;
    LoongWindow(LoongWindow&&) = delete;
    LoongWindow& operator=(const LoongWindow&) = delete;
    LoongWindow& operator=(LoongWindow&&) = delete;

    void SetVisible(bool visible);

    void GetFramebufferSize(int& width, int& height) const;

    void SetTitle(const char* title);

    void SetShouldClose(bool b);

    LG_NODISCARD bool IsMouseVisible() const;

    LG_NODISCARD MouseMode GetMouseMode() const;

    void SetMouseMode(MouseMode b);

    LG_NODISCARD const LoongInput& GetInputManager() const;

    void GetWindowSize(int& width, int& height) const;

    void GetFrameBufferSize(int& width, int& height) const;

    LG_NODISCARD GLFWwindow* GetGlfwWindow() const;

    LOONG_DECLARE_SIGNAL(FrameBufferResize, int, int);
    LOONG_DECLARE_SIGNAL(WindowResize, int, int);
    LOONG_DECLARE_SIGNAL(KeyBoard, LoongKeyCode, LoongInputAction, int);
    LOONG_DECLARE_SIGNAL(MouseButton, LoongMouseButton, LoongInputAction, int);
    LOONG_DECLARE_SIGNAL(CursorPos, double, double);
    LOONG_DECLARE_SIGNAL(WindowPos, int, int);
    LOONG_DECLARE_SIGNAL(WindowIconify, bool);
    LOONG_DECLARE_SIGNAL(WindowClose);
    LOONG_DECLARE_SIGNAL(WindowDestroy);
    LOONG_DECLARE_SIGNAL(BeginFrame);
    LOONG_DECLARE_SIGNAL(Update);
    LOONG_DECLARE_SIGNAL(Render);
    LOONG_DECLARE_SIGNAL(LateUpdate);
    LOONG_DECLARE_SIGNAL(Present);

private:
    explicit LoongWindow(const WindowConfig& config);
    ~LoongWindow();

private:
    class Impl;
    Impl* impl_ { nullptr };
    friend class LoongApplicationImpl;
};

}