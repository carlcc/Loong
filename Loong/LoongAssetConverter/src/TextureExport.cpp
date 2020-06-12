//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//
#include "TextureExport.h"
#include "Flags.h"
#include "LoongFoundation/LoongDefer.h"
#include "LoongFoundation/LoongFormat.h"
#include "LoongFoundation/LoongLogger.h"
#include "LoongFoundation/LoongPathUtils.h"
#include <assimp/scene.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

namespace Loong::AssetConverter {

bool ExportTextureFiles(const aiScene* scene)
{
    if (scene->mNumTextures == 0 && scene->mTextures == nullptr) {
        return true;
    }

    auto& flags = Flags::Get();

    std::string fileName(Foundation::LoongPathUtils::GetFileName(flags.inputFile));
    std::string_view extension = Foundation::LoongPathUtils::GetFileExtension(fileName);
    std::string outputFileNameWihoutExt = fileName.substr(0, fileName.length() - extension.length());

    std::string outputPathWihoutExt = flags.outputDir + '/' + flags.modelPath + '/' + outputFileNameWihoutExt;
    outputPathWihoutExt = Foundation::LoongPathUtils::Normalize(outputPathWihoutExt);

    for (uint32_t i = 0; i < scene->mNumTextures; ++i) {
        auto* texture = scene->mTextures[i];
        std::string hint = texture->achFormatHint;
        uint32_t height = texture->mHeight;
        if (height != 0) {
            uint32_t width = texture->mWidth;
            if (flags.rawTextureOutputFormat == "jpg" || flags.rawTextureOutputFormat == ".jpg") {
                if (0 == stbi_write_jpg(Foundation::Format("{}_{}.{}", outputPathWihoutExt, i, ".jpg").c_str(), width, height, 4, texture->pcData, 10)) {
                    return false;
                }
            } else if (flags.rawTextureOutputFormat == "png" || flags.rawTextureOutputFormat == ".png") {
                if (0 == stbi_write_png(Foundation::Format("{}_{}.{}", outputPathWihoutExt, i, ".png").c_str(), width, height, 4, texture->pcData, 0)) {
                    return false;
                }
            } else if (flags.rawTextureOutputFormat == "bmp" || flags.rawTextureOutputFormat == ".bmp") {
                if (0 == stbi_write_bmp(Foundation::Format("{}_{}.{}", outputPathWihoutExt, i, ".bmp").c_str(), width, height, 4, texture->pcData)) {
                    return false;
                }
            } else if (flags.rawTextureOutputFormat == "tga" || flags.rawTextureOutputFormat == ".tga") {
                if (0 == stbi_write_tga(Foundation::Format("{}_{}.{}", outputPathWihoutExt, i, ".tga").c_str(), width, height, 4, texture->pcData)) {
                    return false;
                }
            } else {
                LOONG_ERROR("Unsupported output texture format: {}", flags.rawTextureOutputFormat);
                abort(); // This should not happen, since we have checked options
            }
            // The data is rgba
        } else {
            // The data is a embedded file (compressed image)
            std::string outputPath = Foundation::Format("{}_{}.{}", outputPathWihoutExt, i, hint);
            FILE* fout = fopen(outputPath.c_str(), "wb");
            if (!fout) {
                LOONG_ERROR("Cannot export texture '{}': {}", outputPath, strerror(errno));
                return false;
            }
            OnScopeExit { fclose(fout); };

            if (1 != fwrite(texture->pcData, texture->mWidth, 1, fout)) {
                LOONG_ERROR("Write texture file '{}' failed: {}", outputPath, strerror(errno));
                return false;
            }
        }
    }
    return true;
}

}