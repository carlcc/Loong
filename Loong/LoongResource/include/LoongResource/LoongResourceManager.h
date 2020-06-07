//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#pragma once

#include <memory>
#include <string>

namespace Loong::Resource {

class LoongTexture;

class LoongResourceManager {
public:
    LoongResourceManager() = delete;

    static bool Initialize();

    static void Uninitialize();

    static std::shared_ptr<LoongTexture> GetTexture(const std::string& path);
};

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

    bool operator!() const
    {
        return !suc_;
    }

private:
    bool suc_ { false };
};

}