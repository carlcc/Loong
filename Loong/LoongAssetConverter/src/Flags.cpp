#pragma once

#include "Flags.h"
#include "LoongFoundation/LoongLogger.h"
#include <functional>
#include <unordered_map>

namespace Loong::AssetConverter {

#define TRY_INCREASE_INDEX(index, argc, error_msg) \
    do {                                           \
        ++index;                                   \
        if (index >= argc) {                       \
            LOONG_ERROR(error_msg);                \
            return false;                          \
        }                                          \
    } while (false)

bool Flags::ParseCommandLine(int argc, char** argv)
{
    auto& flags = GetInterial();

    auto inputOptionHandler = [](int& index, int argc, char** argv) -> bool {
        TRY_INCREASE_INDEX(index, argc, R"(Missing argument for "-i" or "--input")");
        GetInterial().inputFile = argv[index + 1];
        return true;
    };
    const std::unordered_map<std::string, std::function<bool(int&, int, char**)>> kCommandHandlerMap {
        { "-i", inputOptionHandler },
    };

    for (int index = 1; index < argc; ++index) {
        std::string command = argv[index];
        auto it = kCommandHandlerMap.find(command);
        if (it == kCommandHandlerMap.end()) {
            LOONG_ERROR(R"(Unrecognized option "{}")", command);
            return false;
        }
        if (it->second(index, argc, argv)) {
            return false;
        }
    }

    return true;
}

const Flags& Flags::Get()
{
    return GetInterial();
}

Flags& Flags::GetInterial()
{
    static Flags s;
    return s;
}
}