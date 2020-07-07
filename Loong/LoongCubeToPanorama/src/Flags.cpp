#include "Flags.h"
#include "LoongFoundation/LoongFormat.h"
#include "LoongFoundation/LoongLogger.h"
#include "LoongFoundation/LoongStringUtils.h"
#include <functional>
#include <iostream>

namespace Loong::CubeToPanorama {

void PrintHelp(int argc, char** argv)
{
    (void)argc;
    std::cout << "Loong Cube to panorama converter" << std::endl;
    std::cout << "Usage:" << std::endl;
    std::cout << Foundation::Format("        {} [options]\n", argv[0]) << std::endl;
    std::cout << "e.g:" << std::endl;
    std::cout << Foundation::Format("        {} +x right.jpg -x left.jpg +y top.jpg -y bottom.jpg +z back.jpg -z front.jpg -f png -o panorama.png\n", argv[0]) << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "\t+x\tPositive x image" << std::endl;
    std::cout << "\t-x\tNegative x image" << std::endl;
    std::cout << "\t+y\tPositive y image" << std::endl;
    std::cout << "\t-y\tNegative y image" << std::endl;
    std::cout << "\t+z\tPositive z image" << std::endl;
    std::cout << "\t-z\tNegative z image" << std::endl;
    std::cout << "\t-o\tThe output panorama image" << std::endl;
    std::cout << "\t-w\tThe output image width (optional, default " << kDefaultOutputWidth << ")" << std::endl;
    std::cout << "\t-h\tThe output image height (optional, default " << kDefaultOutputHeight<< ")" << std::endl;
    std::cout << "\t-f\tThe output panorama image's format. (.jpg,.png,.bmp,.tga, optional, default " << kDefaultOutputFormat << ")" << std::endl;
}

#define GET_NEXT_ARGUMENT_AS_STRING(var)                        \
    do {                                                        \
        ++index;                                                \
        if (index >= argc) {                                    \
            LOONG_ERROR("Missing parameter for '{}'", command); \
            return false;                                       \
        }                                                       \
        var = argv[index];                                      \
    } while (false)

bool Flags::ParseCommandLine(int argc, char** argv)
{
    for (int index = 1; index < argc; ++index) {
        std::string command = argv[index];
        if (command == "-x") {
            GET_NEXT_ARGUMENT_AS_STRING(Flags::GetInterial().negativeXPath);
        } else if (command == "+x") {
            GET_NEXT_ARGUMENT_AS_STRING(Flags::GetInterial().positiveXPath);
        } else if (command == "-y") {
            GET_NEXT_ARGUMENT_AS_STRING(Flags::GetInterial().negativeYPath);
        } else if (command == "+y") {
            GET_NEXT_ARGUMENT_AS_STRING(Flags::GetInterial().positiveYPath);
        } else if (command == "-z") {
            GET_NEXT_ARGUMENT_AS_STRING(Flags::GetInterial().negativeZPath);
        } else if (command == "+z") {
            GET_NEXT_ARGUMENT_AS_STRING(Flags::GetInterial().positiveZPath);
        } else if (command == "-f") {
            GET_NEXT_ARGUMENT_AS_STRING(Flags::GetInterial().outputFormat);
        } else if (command == "-o") {
            GET_NEXT_ARGUMENT_AS_STRING(Flags::GetInterial().outputPath);
        } else {
            LOONG_ERROR("Unknown option '{}'", command);
            return false;
        }
    }

    if (!CheckFlags()) {
        PrintHelp(argc, argv);
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
    MUST_BE_SET_STRING(flags.positiveXPath);
    MUST_BE_SET_STRING(flags.negativeXPath);
    MUST_BE_SET_STRING(flags.positiveYPath);
    MUST_BE_SET_STRING(flags.negativeYPath);
    MUST_BE_SET_STRING(flags.positiveZPath);
    MUST_BE_SET_STRING(flags.negativeZPath);

    if (flags.outWidth <= 0 || flags.outHeight <= 0) {
        LOONG_ERROR("The output image's dimension should be positive");
        return false;
    }

    Foundation::LoongStringUtils::ToLower(flags.outputFormat);
    std::set<std::string> kSupportedImageFormats { "jpg", ".jpg", "png", ".png", "bmp", ".bmp", "tga", ".tga" };
    if (kSupportedImageFormats.count(flags.outputFormat) == 0) {
        LOONG_ERROR("Unsupported output texture format: {}", flags.outputFormat);
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