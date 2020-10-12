//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#include "LoongFoundation/LoongThreadPool.h"
#include "LoongFoundation/LoongAssert.h"
#include "LoongFoundation/LoongLogger.h"
#include <condition_variable>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>

namespace Loong::Foundation {

class LoongThreadPoolImpl {
public:
    explicit LoongThreadPoolImpl(int threadNum)
    {
        if (threadNum == 0) {
            threadNum = (int)std::thread::hardware_concurrency();
            LOONG_INFO("{} CPU cores found", threadNum);
        }
        LOONG_ASSERT(threadNum >= 0, "Thread count of threadpool should >= 0");

        LOONG_INFO("Create {} threads for thread pool", threadNum);
        threads_.resize(threadNum);
        for (auto& th : threads_) {
            th = std::thread([this]() {
                threadRoutine();
            });
        }
    }
    ~LoongThreadPoolImpl()
    {
        {
            std::unique_lock<std::mutex> lck(tasksMutex_);
            isStop_ = true;
        }
        hasTaskCv_.notify_all();
        for (auto& th : threads_) {
            if (th.joinable()) {
                th.join();
            }
        }
        threads_.clear();
        // TODO: Should we finish all the tasks first?
    }
    LoongThreadPoolImpl(const LoongThreadPoolImpl&) = delete;
    LoongThreadPoolImpl(LoongThreadPoolImpl&&) = delete;
    LoongThreadPoolImpl& operator=(const LoongThreadPoolImpl&) = delete;
    LoongThreadPoolImpl& operator=(LoongThreadPoolImpl&&) = delete;

    std::shared_ptr<LoongThreadTask> AddTask(LoongThreadTask::CallbackType&& callback)
    {
        auto task = std::make_shared<LoongThreadTask>(std::move(callback));
        {
            std::unique_lock<std::mutex> lck(tasksMutex_);
            tasks_.push(task);
            ++tasksCount_;
        }
        hasTaskCv_.notify_one();
        return task;
    }

    LG_NODISCARD size_t GetTasksCount() const { return tasksCount_; }

private:
    void threadRoutine()
    {
        while (!isStop_) {
            std::shared_ptr<LoongThreadTask> task { nullptr };
            {
                std::unique_lock<std::mutex> lck(tasksMutex_);
                hasTaskCv_.wait(lck, [this]() {
                    return !tasks_.empty() || isStop_;
                });
                if (isStop_) {
                    break;
                }
                task = std::move(tasks_.front());
                tasks_.pop();
                --tasksCount_;
            }

            if (task->callback_(task.get())) {
                task->Succeed();
            }

            task->Done();
        }
    }

private:
    // TODO: optimize
    std::mutex tasksMutex_ {};
    std::condition_variable hasTaskCv_ {};
    std::queue<std::shared_ptr<LoongThreadTask>> tasks_ {};
    size_t tasksCount_ { 0 };
    std::vector<std::thread> threads_ {};
    bool isStop_ { false };
};

static LoongThreadPoolImpl* gThreadPool { nullptr };
static std::mutex gThreadPoolMutex {};

std::shared_ptr<LoongThreadTask> LoongThreadPool::AddTask(LoongThreadTask::CallbackType&& callback)
{
    return gThreadPool->AddTask(std::move(callback));
}

size_t LoongThreadPool::GetTasksCount()
{
    LOONG_ASSERT(gThreadPool != nullptr, "You seems to forgot initialize LoongFoundation, try to construct a Loong::Foundation::ScopedDriver");
    return gThreadPool->GetTasksCount();
}

bool LoongThreadPool::Initialize(int threadNum)
{
    std::unique_lock<std::mutex> lck(gThreadPoolMutex);
    LOONG_ASSERT(gThreadPool == nullptr, "You have already initialized the thread pool");

    gThreadPool = new LoongThreadPoolImpl(threadNum);
    return gThreadPool != nullptr;
}

void LoongThreadPool::Uninitialize()
{
    std::unique_lock<std::mutex> lck(gThreadPoolMutex);
    LOONG_ASSERT(gThreadPool != nullptr, "The thread pool was not initialed or has been uninitialized.");
    delete gThreadPool;
    gThreadPool = nullptr;
}

}