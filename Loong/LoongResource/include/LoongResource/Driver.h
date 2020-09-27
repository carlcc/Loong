//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#pragma once

#include "LoongResource/LoongResourceManager.h"

namespace Loong::Resource {

struct ScopedDriver {
    ScopedDriver()
    {
        suc_ = LoongResourceManager::Initialize();
    }
    ScopedDriver(const ScopedDriver&) = delete;
    ScopedDriver(ScopedDriver&&) = delete;
    ScopedDriver& operator=(const ScopedDriver&) = delete;
    ScopedDriver& operator=(ScopedDriver&&) = delete;

    ~ScopedDriver()
    {
        if (suc_) {
            LoongResourceManager::Uninitialize();
        }
    }

    bool operator!() const { return !suc_; }

    explicit operator bool() const { return suc_; }

private:
    bool suc_ { false };
};

}