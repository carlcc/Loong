//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#pragma once

#include <chrono>
#include <cstdint>

namespace Loong::Foundation {

class LoongClock {
public:
    void Reset()
    {
        using namespace std::chrono;
        currentSystemTimeMicros_ = duration_cast<microseconds>(system_clock::now().time_since_epoch()).count();
        baseSteadyTimeMicros_ = duration_cast<microseconds>(high_resolution_clock::now().time_since_epoch()).count();
        currentSteadyTimeMicros_ = baseSteadyTimeMicros_;
        deltaTimeMicros_ = 0;
        elapsedTimeMicros_ = 0;
    }

    void Update()
    {
        using namespace std::chrono;
        currentSystemTimeMicros_ = duration_cast<microseconds>(system_clock::now().time_since_epoch()).count();
        int64_t steadyNow = duration_cast<microseconds>(high_resolution_clock::now().time_since_epoch()).count();
        deltaTimeMicros_ = steadyNow - currentSteadyTimeMicros_;
        elapsedTimeMicros_ = steadyNow - baseSteadyTimeMicros_;
        currentSteadyTimeMicros_ = steadyNow;
    }

    float DeltaTime() const
    {
        return deltaTimeMicros_ / 1000000.F;
    }

    float ElapsedTime() const
    {
        return elapsedTimeMicros_ / 1000000.F;
    }

private:
    int64_t baseSteadyTimeMicros_ { 0 };
    int64_t currentSteadyTimeMicros_ { 0 };
    int64_t currentSystemTimeMicros_ { 0 };
    int64_t deltaTimeMicros_ { 0 };
    int64_t elapsedTimeMicros_ { 0 };
};

}