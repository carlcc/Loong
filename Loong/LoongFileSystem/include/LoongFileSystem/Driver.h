//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#pragma once

#include "LoongFileSystem/LoongFileSystem.h"

namespace Loong::FS {

struct ScopedDriver {
    explicit ScopedDriver(const char* argv0)
    {
        suc_ = LoongFileSystem::Initialize(argv0);
    }
    ScopedDriver(const ScopedDriver&) = delete;
    ScopedDriver(ScopedDriver&&) = delete;
    ScopedDriver& operator=(const ScopedDriver&) = delete;
    ScopedDriver& operator=(ScopedDriver&&) = delete;

    ~ScopedDriver()
    {
        if (suc_) {
            LoongFileSystem::Uninitialize();
        }
    }

    bool operator!() const
    {
        return !suc_;
    }

private:
    bool suc_ { false };
};

}