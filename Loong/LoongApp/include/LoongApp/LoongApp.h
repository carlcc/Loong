//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#pragma once
#include "LoongApp/LoongInput.h"
#include "LoongFoundation/LoongSigslotHelper.h"

namespace Loong::App {

class LoongApp {
public:
    enum MouseMode {
        kNormal = 0,
        kHidden,
        kDisabled,
    };
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
    explicit LoongApp(const WindowConfig& config);
    ~LoongApp();

    LoongApp(const LoongApp&) = delete;
    LoongApp(LoongApp&&) = delete;
    LoongApp& operator=(const LoongApp&) = delete;
    LoongApp& operator=(LoongApp&&) = delete;

    int Run();

    void GetFramebufferSize(int& width, int& height) const;

    void SetTitle(const char* title);

    void SetShouldClose(bool b);

    bool IsMouseVisible() const;

    MouseMode GetMouseMode() const;

    void SetMouseMode(MouseMode b);

    const LoongInput& GetInputManager() const;

    void GetWindowSize(int& width, int& height) const;

    void GetFrameBufferSize(int& width, int& height) const;

    LOONG_DECLARE_SIGNAL(FrameBufferResize, int, int);
    LOONG_DECLARE_SIGNAL(WindowResize, int, int);
    LOONG_DECLARE_SIGNAL(KeyBoard, LoongKeyCode, LoongInputAction, int);
    LOONG_DECLARE_SIGNAL(MouseButton, LoongMouseButton, LoongInputAction, int);
    LOONG_DECLARE_SIGNAL(CursorPos, double, double);
    LOONG_DECLARE_SIGNAL(WindowPos, int, int);
    LOONG_DECLARE_SIGNAL(WindowIconify, bool);
    LOONG_DECLARE_SIGNAL(WindowClose);
    LOONG_DECLARE_SIGNAL(Update);
    LOONG_DECLARE_SIGNAL(Render);

private:
    class Impl;
    Impl* impl_ { nullptr };
};

}