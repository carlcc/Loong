//
// Created by carl on 20-4-3.
//

#pragma once

#include <functional>
#include <utility>

#define LOONG_DEFER_COMBINE1(X, Y) X##Y
#define LOONG_DEFER_COMBINE(X, Y) LOONG_DEFER_COMBINE1(X, Y)

#define OnScopeExit ::Loong::Foundation::LoongDefer LOONG_DEFER_COMBINE(_defer_, __LINE__) = [&]() -> void

namespace Loong::Foundation {

class LoongDefer {
public:
    template <typename Callable>
    LoongDefer(Callable&& defer) // NOLINT(google-explicit-constructor,hicpp-explicit-conversions)
        : defered_(std::forward<Callable>(defer))
    {
    }

    LoongDefer()
        : defered_([]() {})
    {
    }
    LoongDefer(const LoongDefer&) = delete;
    LoongDefer(LoongDefer&& d) noexcept = delete;
    LoongDefer(const LoongDefer&& d) noexcept = delete;
    LoongDefer& operator=(const LoongDefer&) = delete;
    LoongDefer& operator=(LoongDefer&& d) noexcept
    {
        defered_ = d.defered_;
        d.defered_ = []() {};
        return *this;
    }

    ~LoongDefer()
    {
        defered_();
    }

    void Cancel()
    {
        defered_ = []() {};
    }

private:
    std::function<void()> defered_;
};

}