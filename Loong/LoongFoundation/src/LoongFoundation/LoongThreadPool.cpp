//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#include "LoongFoundation/LoongThreadPool.h"
#include "LoongFoundation/LoongAssert.h"
#include "LoongFoundation/LoongLogger.h"
#include <TPL/TPL.h>

namespace Loong::Foundation {

static tpl::ITaskScheduler* gDefaultScheduler { nullptr };

bool LoongThreadPool::Initialize(int threadNum)
{
    LOONG_ASSERT(tpl::gDefaultTaskScheduler == nullptr && gDefaultScheduler == nullptr, "The default task scheduler has been set");
    gDefaultScheduler = new tpl::ParallelTaskScheduler(size_t(threadNum));
    tpl::SetDefaultTaskScheduler(gDefaultScheduler);
    return gDefaultScheduler != nullptr;
}

void LoongThreadPool::Uninitialize()
{
    LOONG_ASSERT(gDefaultScheduler != nullptr, "The thread pool was not initialed or has been uninitialized.");
    tpl::SetDefaultTaskScheduler(nullptr);
    delete gDefaultScheduler;
    gDefaultScheduler = nullptr;
}

tpl::ITaskScheduler* LoongThreadPool::GetScheduler()
{
    return gDefaultScheduler;
}

}