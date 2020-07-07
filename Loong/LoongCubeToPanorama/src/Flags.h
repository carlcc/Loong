#pragma once

#include <string>

namespace Loong::CubeToPanorama {

static const int kDefaultOutputWidth = 1024 * 4;
static const int kDefaultOutputHeight = 1024 * 2;
static const char* kDefaultOutputFormat = "png";

struct Flags {

    static bool ParseCommandLine(int argc, char** argv);

    static const Flags& Get();

    std::string positiveXPath;

    std::string negativeXPath;

    std::string positiveYPath;

    std::string negativeYPath;

    std::string positiveZPath;

    std::string negativeZPath;

    int outWidth { kDefaultOutputWidth };

    int outHeight { kDefaultOutputHeight };

    std::string outputPath;

    std::string outputFormat = kDefaultOutputFormat;

private:
    Flags() = default;
    static Flags& GetInterial();
    static bool CheckFlags();
};

}