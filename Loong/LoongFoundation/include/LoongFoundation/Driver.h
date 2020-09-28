//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#pragma once

#include "LoongFoundation/LoongThreadPool.h"

namespace Loong::Foundation {

struct ScopedDriver {
    explicit ScopedDriver(int threadNum = 0)
    {
        threadPoolSuc_ = LoongThreadPool::Initialize(threadNum);
    }
    ScopedDriver(const ScopedDriver&) = delete;
    ScopedDriver(ScopedDriver&&) = delete;
    ScopedDriver& operator=(const ScopedDriver&) = delete;
    ScopedDriver& operator=(ScopedDriver&&) = delete;

    ~ScopedDriver()
    {
        if (threadPoolSuc_) {
            LoongThreadPool::Uninitialize();
        }
    }

    bool operator!() const
    {
        return !threadPoolSuc_;
    }

private:
    bool threadPoolSuc_ { false };
};

}