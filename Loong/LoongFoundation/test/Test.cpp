//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#include "LoongFoundation/LoongLogger.h"
#include "LoongFoundation/LoongPathUtils.h"
#include "LoongFoundation/LoongSigslotHelper.h"
#include "LoongFoundation/LoongStringUtils.h"
#include <cassert>
#include <iostream>
#include <vector>

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
        std::cout << fmt::format("[{}][{}] {}", GetLogLevelName(logItem.level), logItem.location, logItem.message) << std::endl;
    }
};

void TestPathUtils();

int main(int argc, const char* argv[])
{
    LogWriter writer;

    SigReceiver receiver;
    SigEmmiter emmiter;

    emmiter.SubscribeMySig(receiver, &SigReceiver::foo);
    auto listenerId = emmiter.SubscribeMySig([](int a) {
        std::cout << "Hello_" << a << std::endl;
    });

    emmiter.DoSomething();

    std::cout << "======" << std::endl;

    listenerId.reset();
    emmiter.DoSomething();

    TestPathUtils();

    return 0;
}

void TestPathUtils()
{
    std::vector<std::string> expected1 { "he", "ll", "", "o", "world", "!" };
    std::vector<std::string> got1 = LoongStringUtils::Split("he/ll//o/world/!", "/");
    assert(expected1 == got1);
    std::vector<std::string> got2 = LoongStringUtils::Split("he/ll//o/world/!", "");
    assert(got2.size() == 1 && "he/ll//o/world/!" == got2[0]);

#if WIN32
    assert(LoongPathUtils::Normalize("C:/a/b\\//c") == "C:/a/b/c");
#else
    assert(LoongPathUtils::Normalize("/a/b\\//c") == "/a/b/c");
#endif
    assert(LoongPathUtils::Normalize("/") == "/");
    assert(LoongPathUtils::Normalize(".") == ".");
    assert(LoongPathUtils::Normalize("") == "");
    assert(LoongPathUtils::Normalize("././a/b\\//c") == "a/b/c");
    assert(LoongPathUtils::Normalize("./../a/b\\//c") == "../a/b/c");
    assert(LoongPathUtils::Normalize("./../a/../b\\//c") == "../b/c");

    assert(LoongPathUtils::GetParent("a/b/c//\\d") == "a/b/c");
    assert(LoongPathUtils::GetParent("a/b/.././c//\\d") == "a/c");
#if WIN32
    assert(LoongPathUtils::GetParent("C:/a") == "C:/");
#else
    assert(LoongPathUtils::GetParent("/a") == "/");
#endif
}