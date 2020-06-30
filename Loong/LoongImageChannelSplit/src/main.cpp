#ifdef _MSC_VER
// e.g. This function or variable may be unsafe. Consider using fopen_s instead.
#pragma warning(disable : 4996)
#endif
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image.h"
#include "stb_image_write.h"
#include <LoongFoundation/LoongDefer.h>
#include <LoongFoundation/LoongFormat.h>
#include <LoongFoundation/LoongPathUtils.h>
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

using namespace Loong;

int main(int argc, char* argv[])
{
    std::string inputFile;
    std::string outputDir;
    std::string outputFormat = "jpg";

    for (int i = 0; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-i") {
            ++i;
            if (i >= argc) {
                std::cerr << "Missing parameter for option '-i'" << std::endl;
                return -1;
            }
            inputFile = argv[i];
        }
        if (arg == "-o") {
            ++i;
            if (i >= argc) {
                std::cerr << "Missing parameter for option '-o'" << std::endl;
                return -1;
            }
            outputDir = argv[i];
        }
        if (arg == "-ofmt") {
            ++i;
            if (i >= argc) {
                std::cerr << "Missing parameter for option '-ofmt'" << std::endl;
                return -1;
            }
            outputFormat = argv[i];
        }
    }

    std::string fileName(Foundation::LoongPathUtils::GetFileName(inputFile));
    std::string_view extension = Foundation::LoongPathUtils::GetFileExtension(fileName);
    std::string outputFileNameWithoutExt = fileName.substr(0, fileName.length() - extension.length());

    std::string outputPathWithoutExt = outputDir + '/' + outputFileNameWithoutExt;
    outputPathWithoutExt = Foundation::LoongPathUtils::Normalize(outputPathWithoutExt);
    // TODO: check output format

    int width, height, channelCount;
    auto* imageData = (uint8_t*)stbi_load(inputFile.c_str(), &width, &height, &channelCount, 0);
    if (imageData == nullptr || channelCount <= 0 || channelCount > 4) {
        std::cerr << "Load file '" << inputFile << "' failed!" << std::endl;
        return -2;
    }

    auto* tmpData = new uint8_t[width * height];
    OnScopeExit
    {
        free(imageData);
        delete[] tmpData;
    };

    for (int channel = 0; channel < channelCount; ++channel) {
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                tmpData[y * width + x] = imageData[(y * width + x) * channelCount + channel];
            }
        }

        if (outputFormat == "jpg" || outputFormat == ".jpg") {
            if (0 == stbi_write_jpg(Foundation::Format("{}_{}.{}", outputPathWithoutExt, channel, ".jpg").c_str(), width, height, 1, tmpData, 10)) {
                return false;
            }
        } else if (outputFormat == "png" || outputFormat == ".png") {
            if (0 == stbi_write_png(Foundation::Format("{}_{}.{}", outputPathWithoutExt, channel, ".png").c_str(), width, height, 1, tmpData, 0)) {
                return false;
            }
        } else if (outputFormat == "bmp" || outputFormat == ".bmp") {
            if (0 == stbi_write_bmp(Foundation::Format("{}_{}.{}", outputPathWithoutExt, channel, ".bmp").c_str(), width, height, 1, tmpData)) {
                return false;
            }
        } else if (outputFormat == "tga" || outputFormat == ".tga") {
            if (0 == stbi_write_tga(Foundation::Format("{}_{}.{}", outputPathWithoutExt, channel, ".tga").c_str(), width, height, 1, tmpData)) {
                return false;
            }
        } else {
            abort(); // This should not happen, since we have checked options
        }
    }

    return 0;
}