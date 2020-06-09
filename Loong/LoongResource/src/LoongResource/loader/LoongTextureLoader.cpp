//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//
#include <glad/glad.h>

#include "LoongAsset/LoongImage.h"
#include "LoongFoundation/LoongLogger.h"
#include "LoongResource/LoongTexture.h"
#include "LoongTextureLoader.h"
#include <cassert>

namespace Loong::Resource {

inline GLenum ChannelCountToGLTextureFormat(int count)
{
    switch (count) {
    case 1:
        return GL_RED;
    case 3:
        return GL_RGB;
    case 4:
        return GL_RGBA;
    default:
        return -1;
    }
}

std::shared_ptr<LoongTexture> LoongTextureLoader::Create(const Asset::LoongImage& image, bool generateMipmap, const std::function<void(const std::string&)>& onDestroy)
{
    assert(bool(image));

    auto imageFormat = ChannelCountToGLTextureFormat(image.GetChannelCount());
    if (imageFormat == -1) {
        LOONG_WARNING("Cannot crate texture from image '{}': Unknown format", image.GetPath());
        return {};
    }

    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // Disable alignment
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, image.GetWidth(), image.GetHeight(), 0, imageFormat, GL_UNSIGNED_BYTE, image.GetData());

    if (generateMipmap) {
        glGenerateMipmap(GL_TEXTURE_2D);
    }

    // TODO: Configurable
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);

    glBindTexture(GL_TEXTURE_2D, 0);

    auto* tex = new LoongTexture(textureID, image.GetWidth(), image.GetHeight(), image.GetChannelCount(), generateMipmap);
    if (onDestroy != nullptr) {
        return std::shared_ptr<LoongTexture>(tex, [onDestroy, path = image.GetPath()](LoongTexture* tex) {
            onDestroy(path);
            delete tex;
        });
    } else {
        return std::shared_ptr<LoongTexture>(tex);
    }
}

std::shared_ptr<LoongTexture> LoongTextureLoader::CreateColor(uint32_t data, bool generateMipmap, const std::function<void(const std::string&)>& onDestroy)
{
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, &data);

    if (generateMipmap) {
        glGenerateMipmap(GL_TEXTURE_2D);
    }

    // TODO: Configurable
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);

    glBindTexture(GL_TEXTURE_2D, 0);

    auto* tex = new LoongTexture(textureID, 1, 1, 32, generateMipmap);
    if (onDestroy != nullptr) {
        return std::shared_ptr<LoongTexture>(tex, [onDestroy](LoongTexture* tex) {
            onDestroy("");
            delete tex;
        });
    } else {
        return std::shared_ptr<LoongTexture>(tex);
    }
}

std::shared_ptr<LoongTexture> LoongTextureLoader::CreateFromMemory(uint8_t* data, uint32_t width, uint32_t height, bool generateMipmap, const std::function<void(const std::string&)>& onDestroy)
{
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

    if (generateMipmap) {
        glGenerateMipmap(GL_TEXTURE_2D);
    }

    // TODO: configurable
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);

    glBindTexture(GL_TEXTURE_2D, 0);

    auto* tex = new LoongTexture(textureID, 1, 1, 32, generateMipmap);
    if (onDestroy != nullptr) {
        return std::shared_ptr<LoongTexture>(tex, [onDestroy](LoongTexture* tex) {
            onDestroy("");
            delete tex;
        });
    } else {
        return std::shared_ptr<LoongTexture>(tex);
    }
}

} // namespace Ss
