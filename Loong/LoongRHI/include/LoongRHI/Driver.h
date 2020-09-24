//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#pragma once

#include "LoongRHI/LoongRHIManager.h"

namespace Loong::RHI {

struct ScopedDriver {
    explicit ScopedDriver(GLFWwindow* glfwWindow, RENDER_DEVICE_TYPE deviceType)
    {
        suc_ = LoongRHIManager::Initialize(glfwWindow, deviceType);
    }
    ScopedDriver(const ScopedDriver&) = delete;
    ScopedDriver(ScopedDriver&&) = delete;
    ScopedDriver& operator=(const ScopedDriver&) = delete;
    ScopedDriver& operator=(ScopedDriver&&) = delete;

    ~ScopedDriver()
    {
        if (suc_) {
            LoongRHIManager::Uninitialize();
        }
    }

    bool operator!() const
    {
        return !suc_;
    }

    explicit operator bool() const
    {
        return suc_;
    }

private:
    bool suc_ { false };
};

}