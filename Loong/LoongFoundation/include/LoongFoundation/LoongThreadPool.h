//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#pragma once

#include "LoongFoundation/LoongMacros.h"

namespace tpl {
class ITaskScheduler;
}

namespace Loong::Foundation {

class LoongThreadPool {
public:
    explicit LoongThreadPool() = delete;
    ~LoongThreadPool() = delete;
    LoongThreadPool(const LoongThreadPool&) = delete;
    LoongThreadPool(LoongThreadPool&&) = delete;
    LoongThreadPool& operator=(const LoongThreadPool&) = delete;
    LoongThreadPool& operator=(LoongThreadPool&&) = delete;

    static bool Initialize(int threadNum = 0);

    static void Uninitialize();

    static tpl::ITaskScheduler* GetScheduler();
};

}