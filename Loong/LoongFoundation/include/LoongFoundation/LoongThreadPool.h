//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#pragma once

#include "LoongFoundation/LoongMacros.h"
#include <cstdint>
#include <functional>

namespace Loong::Foundation {

class LoongThreadTask {
public:
    // Returns true if done successfully, else false
    using CallbackType = std::function<bool(LoongThreadTask* task)>;

    LoongThreadTask() = default;
    explicit LoongThreadTask(CallbackType&& task)
        : callback_ { std::move(task) }
    {
    }
    LoongThreadTask(const LoongThreadTask&) = delete;
    LoongThreadTask(LoongThreadTask&& task) = delete;
    LoongThreadTask& operator=(const LoongThreadTask&) = delete;
    LoongThreadTask& operator=(LoongThreadTask&& task) = delete;

    LG_NODISCARD bool IsDone() const { return isDone_; }
    LG_NODISCARD bool IsSucceed() const { return isSucceed_; }

    void Done() { isDone_ = true; }
    void Succeed() { isSucceed_ = true; }

private:
    /// What this task will do
    CallbackType callback_ { [](LoongThreadTask*) { return true; } };
    bool isDone_ { false };
    bool isSucceed_ { false };

    friend class LoongThreadPoolImpl;
};

class LoongThreadPool {
public:
    explicit LoongThreadPool() = delete;
    ~LoongThreadPool() = delete;
    LoongThreadPool(const LoongThreadPool&) = delete;
    LoongThreadPool(LoongThreadPool&&) = delete;
    LoongThreadPool& operator=(const LoongThreadPool&) = delete;
    LoongThreadPool& operator=(LoongThreadPool&&) = delete;

    static std::shared_ptr<LoongThreadTask> AddTask(LoongThreadTask::CallbackType&& callback);

    LG_NODISCARD static size_t GetTasksCount();

    static bool Initialize(int threadNum = 0);

    static void Uninitialize();
};

}