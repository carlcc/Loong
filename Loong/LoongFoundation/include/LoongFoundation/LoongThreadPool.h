//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#pragma once

#include <cstdint>
#include <functional>

namespace Loong::Foundation {

class LoongThreadTask {
public:
    // Returns true if done successfully, else false
    using CallbackType = std::function<bool()>;

    LoongThreadTask(CallbackType&& task)
        : callback_ { std::move(task) }
        , isCaneled_ { false }
    {
    }
    LoongThreadTask(const LoongThreadTask&) = delete;
    LoongThreadTask(LoongThreadTask&& task) noexcept
    {
        callback_ = std::move(task.callback_);
        isCaneled_ = task.isCaneled_;
        isDone_ = task.isDone_;
    }
    LoongThreadTask& operator=(const LoongThreadTask&) = delete;
    LoongThreadTask& operator=(LoongThreadTask&& task) noexcept
    {
        callback_ = std::move(task.callback_);
        isCaneled_ = task.isCaneled_;
        isDone_ = task.isDone_;
        return *this;
    }

    void Cancel() { isCaneled_ = true; }
    bool IsCanceld() const { return isCaneled_; }
    bool IsDone() const { return isDone_; }

private:
    void Done() { isDone_ = true; }

    /// What this task will do
    CallbackType callback_ { []() { return true; } };
    bool isCaneled_ { false };
    bool isDone_ { false };

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

    static size_t GetTasksCount();

    static bool Initialize(int threadNum = 0);

    static void Uninitialize();
};

}