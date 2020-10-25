#include "Flags.h"
#include "LoongFoundation/LoongFormat.h"
#include "LoongFoundation/LoongLogger.h"
#include "LoongFoundation/LoongStringUtils.h"
#include <functional>
#include <iostream>
#include <unordered_map>
#include <vector>

namespace Loong::AssetConverter {

#define DEFINE_STRING_OPTION_HANDLER(optionVariable)                                                           \
    [](int& index, int argc, char** argv) -> bool {                                                            \
        if (index + 1 >= argc) {                                                                               \
            std::cout << Foundation::Format(R"(Missing argument for "{}" option!)", argv[index]) << std::endl; \
            return false;                                                                                      \
        }                                                                                                      \
        ++index;                                                                                               \
        optionVariable = argv[index];                                                                          \
        return true;                                                                                           \
    }

struct CommandOptionDesc {
    std::vector<std::string> option;
    std::string helpDesc;
    std::function<bool(int&, int, char**)> parserFunc;
};

void PrintHelp(const std::vector<CommandOptionDesc>& kOptionDescs, int argc, char** argv)
{
    (void)argc;
    std::cout << "Loong Asset Converter" << std::endl;
    std::cout << "Usage:" << std::endl;
    std::cout << Foundation::Format("        {} [options]\n", argv[0]) << std::endl;
    std::cout << "Options:" << std::endl;
    for (auto& optDesc : kOptionDescs) {
        std::string left = Foundation::LoongStringUtils::Join(optDesc.option, ",");
        std::string right = optDesc.helpDesc;

        const int kRightSpace = 120 - 20 - 4 - 4 - 1;
        std::cout << Foundation::Format("    {:<20}{}", left, right) << std::endl;;
    }
}

bool Flags::ParseCommandLine(int argc, char** argv)
{

    std::vector<CommandOptionDesc> kOptionDescs {
        { { "-i", "--input" }, "Specify an input file", DEFINE_STRING_OPTION_HANDLER(GetInterial().inputFile) },
        { { "-o", "--output" }, "Specify an output directory", DEFINE_STRING_OPTION_HANDLER(GetInterial().outputDir) },
        { { "-mp", "--model-path" }, "Specify the path (under output path) of model files", DEFINE_STRING_OPTION_HANDLER(GetInterial().modelPath) },
        { { "-tt", "--texture-type" }, "Specify the image format to store uncompressed embedded texture (support png/jpg/bmp/tga, default jpg)",
            DEFINE_STRING_OPTION_HANDLER(GetInterial().rawTextureOutputFormat) },
        { { "-tp", "--texture-path" }, "Specify the path (under output path) of texture files", DEFINE_STRING_OPTION_HANDLER(GetInterial().texturePath) },
        { { "-h", "--help" }, "Print this help", [](int& index, int argc, char** argv) -> bool { return false; } },
    };
    std::unordered_map<std::string, std::function<bool(int&, int, char**)>> kCommandHandlerMap;
    for (auto& optDesc : kOptionDescs) {
        for (auto& opt : optDesc.option) {
            kCommandHandlerMap[opt] = optDesc.parserFunc;
        }
    }

    for (int index = 1; index < argc; ++index) {
        std::string command = argv[index];
        auto it = kCommandHandlerMap.find(command);
        if (it == kCommandHandlerMap.end()) {
            LOONG_ERROR(R"(Unrecognized option "{}")", command);
            PrintHelp(kOptionDescs, argc, argv);
            return false;
        }
        if (!it->second(index, argc, argv)) {
            PrintHelp(kOptionDescs, argc, argv);
            return false;
        }
    }

    if (!CheckFlags()) {
        PrintHelp(kOptionDescs, argc, argv);
        return false;
    }

    return true;
}

#define MUST_BE_SET_STRING(str) \
    do {                        \
        if (str.empty())        \
            return false;       \
    } while (false)

bool Flags::CheckFlags()
{
    auto& flags = GetInterial();
    MUST_BE_SET_STRING(flags.inputFile);
    MUST_BE_SET_STRING(flags.outputDir);

    Foundation::LoongStringUtils::ToLower(flags.rawTextureOutputFormat);
    std::set<std::string> kSupportedImageFormats { "jpg", ".jpg", "png", ".png", "bmp", ".bmp", "tga", ".tga" };
    if (kSupportedImageFormats.count(flags.rawTextureOutputFormat) == 0) {
        LOONG_ERROR("Unsupported output texture format: {}", flags.rawTextureOutputFormat);
        return false;
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