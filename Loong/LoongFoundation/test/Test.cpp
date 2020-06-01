//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#include "LoongFoundation/LoongLogger.h"
#include "LoongFoundation/LoongSigslotHelper.h"
#include <cassert>
#include <iostream>

using namespace Loong::Foundation;

class SigEmmiter {
public:
    void DoSomething()
    {
        MySigSignal_.emit(1234);
    }
    LOONG_DECLARE_SIGNAL(MySig, int);
};

class SigReceiver : public LoongHasSlots {
public:
    void foo(int a)
    {
        LOONG_INFO("Received {}", a);
    }
};

class LogWriter : public LoongHasSlots {
public:
    LogWriter()
    {
        Logger::Get().SubscribeLog(this, &LogWriter::OnLog);
    }
    void OnLog(const LogItem& logItem)
    {
        std::cout << fmt::format("[{}][{}] {}", GetLogLevelName(logItem.level), logItem.location, logItem.message);
    }
};

int main(int argc, const char* argv[])
{
    LogWriter writer;

    SigReceiver receiver;
    SigEmmiter emmiter;

    emmiter.SubscribeMySig(receiver, &SigReceiver::foo);

    emmiter.DoSomething();

    return 0;
}