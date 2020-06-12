#pragma once

#include <string>

namespace Loong::AssetConverter {

struct Flags {

    static bool ParseCommandLine(int argc, char** argv);

    static const Flags& Get();

    std::string inputFile;

    std::string outputDir;

    std::string modelPath;

    std::string texturePath;

    std::string rawTextureOutputFormat = ".jpg";

private:
    Flags() = default;
    static Flags& GetInterial();
    static bool CheckFlags();
};

}