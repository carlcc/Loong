#pragma once

#include <string>

namespace Loong::AssetConverter {

struct Flags {

    static bool ParseCommandLine(int argc, char** argv);

    static const Flags& Get();

    std::string inputFile;

private:
    Flags() = default;
    static Flags& GetInterial();
};

}