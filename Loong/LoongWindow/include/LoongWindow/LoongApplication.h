//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#pragma once
#include <functional>

namespace Loong::Window {

class LoongWindow;
struct WindowConfig;

class LoongApplication {
public:
    using Task = std::function<void()>;

    static int Run();

    // NOTE: If you want to destroy this window, invoke Destroy function
    static LoongWindow* CreateWindow(const WindowConfig& cfg, std::function<void(LoongWindow*)>&& onDelete = nullptr);

    static void SetDeleterForWindow(LoongWindow* win, std::function<void(LoongWindow*)>&& onDelete);

    static void DestroyWindow(LoongWindow* win);

    static void DestroyAllWindows();

    static void RunInMainThread(Task&& task);

    static bool IsInMainThread();
};
}