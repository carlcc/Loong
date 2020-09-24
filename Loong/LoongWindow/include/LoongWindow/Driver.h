//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#pragma once

namespace Loong::Window {

bool Initialize();

void Uninitialize();

struct ScopedDriver {
    ScopedDriver()
    {
        suc_ = Initialize();
    }
    ScopedDriver(const ScopedDriver&) = delete;
    ScopedDriver(ScopedDriver&&) = delete;
    ScopedDriver& operator=(const ScopedDriver&) = delete;
    ScopedDriver& operator=(ScopedDriver&&) = delete;

    ~ScopedDriver()
    {
        if (suc_) {
            Uninitialize();
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