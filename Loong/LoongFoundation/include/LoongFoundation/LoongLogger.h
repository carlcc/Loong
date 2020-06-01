//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#pragma once

#include "LoongFoundation/LoongSigslotHelper.h"
#include "fmt/format.h"
#include <ostream>
#include <string>

namespace Loong::Foundation {

enum class LogLevel {
    kTrace = 0,
    kDebug = 1,
    kInfo = 2,
    kWarning = 3,
    kError = 4,

    kLogLevelCount
};

inline const char* GetLogLevelName(LogLevel level)
{
    const char* kLevelNames[int(LogLevel::kLogLevelCount)] = {
        "Trace", "Debug", "Info ", "Warn", "Error"
    };
    return kLevelNames[int(level)];
}

inline std::ostream& operator<<(std::ostream& os, LogLevel level)
{
    os << GetLogLevelName(level);
    return os;
}

struct LogItem {
    LogLevel level;
    std::string_view location; // File and line
    std::string message;
};

class Logger {
public:
    static Logger& Get()
    {
        static Logger l;
        return l;
    }

    template <LogLevel level, class... Args>
    void Log(std::string_view location, std::string_view fmt, Args... args)
    {
        LogSignal_.emit(LogItem {
            level,
            location,
            fmt::format(fmt, std::forward<Args>(args)...),
        });
    }

    LOONG_DECLARE_SIGNAL(Log, const LogItem&);
};

#define LOONG_LOG_TOSTRING1(X) #X
#define LOONG_LOG_TOSTRING(X) LOONG_LOG_TOSTRING1(X)
#define LOONG_LOG(level, fmt, ...)                                                                                    \
    do {                                                                                                              \
        ::Loong::Foundation::Logger::Get().Log<level>(__FILE__ ":" LOONG_LOG_TOSTRING(__LINE__), fmt, ##__VA_ARGS__); \
    } while (false)
// clang-format off
#define LOONG_TRACE(fmt, ...)   LOONG_LOG(::Loong::Foundation::LogLevel::kTrace,   fmt, ##__VA_ARGS__)
#define LOONG_DEBUG(fmt, ...)   LOONG_LOG(::Loong::Foundation::LogLevel::kDebug,   fmt, ##__VA_ARGS__)
#define LOONG_INFO(fmt, ...)    LOONG_LOG(::Loong::Foundation::LogLevel::kInfo,    fmt, ##__VA_ARGS__)
#define LOONG_WARNING(fmt, ...) LOONG_LOG(::Loong::Foundation::LogLevel::kWarning, fmt, ##__VA_ARGS__)
#define LOONG_ERROR(fmt, ...)   LOONG_LOG(::Loong::Foundation::LogLevel::kError,   fmt, ##__VA_ARGS__)
// clang-format on
}