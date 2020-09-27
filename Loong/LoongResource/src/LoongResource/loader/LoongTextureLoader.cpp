//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//
#include <glad/glad.h>

#include "LoongAsset/LoongImage.h"
#include "LoongFileSystem/LoongFileSystem.h"
#include "LoongFoundation/LoongAssert.h"
#include "LoongFoundation/LoongLogger.h"
#include "LoongFoundation/LoongPathUtils.h"
#include "LoongRHI/LoongRHIManager.h"
#include "LoongResource/loader/LoongTextureLoader.h"
#include <Image.h> // Diligent Engine
#include <TextureLoader.h> // Dliigent Engine
#include <cassert>

namespace Loong::Resource {

RHI::RefCntAutoPtr<RHI::ITexture> LoongTextureLoader::Create(const std::string& vfsPath, RHI::RefCntAutoPtr<RHI::IRenderDevice> device, bool isSrgb)
{
    RHI::RefCntAutoPtr<RHI::ITexture> texture { nullptr };

    int64_t fileSize = FS::LoongFileSystem::GetFileSize(vfsPath);
    if (fileSize <= 0) {
        LOONG_ERROR("Failed to load texture '{}': Wrong file size", vfsPath);
        return texture;
    }
    std::vector<uint8_t> buffer(fileSize);
    int64_t size = FS::LoongFileSystem::LoadFileContent(vfsPath, buffer.data(), fileSize);
    (void)size;
    assert(size == fileSize);

    class MyDataBlob : public RHI::ObjectBase<RHI::IDataBlob> {
    public:
        using Base = RHI::ObjectBase<RHI::IDataBlob>;
        explicit MyDataBlob(RHI::IReferenceCounters* pRefCounters, std::vector<uint8_t>&& buffer)
            : Base { pRefCounters }
            , buffer_ { std::move(buffer) }
        {
        }
        void Resize(size_t NewSize) override { abort(); } // Not supported
        size_t GetSize() const override { return buffer_.size(); }
        void* GetDataPtr() override { return buffer_.data(); }
        const void* GetConstDataPtr() const override { return buffer_.data(); }

        std::vector<uint8_t> buffer_ {};
    };
    RHI::RefCntAutoPtr<MyDataBlob> dataBlob { RHI::MakeNewRCObj<MyDataBlob>()(std::move(buffer)) };

    auto imgFileFormat = RHI::Image::GetFileFormat(static_cast<uint8_t*>(dataBlob->GetDataPtr()), dataBlob->GetSize());
    if (imgFileFormat == RHI::IMAGE_FILE_FORMAT_UNKNOWN) {
        LOONG_WARNING("Unable to derive image format from the header for file '{}'. Trying to analyze extension.", vfsPath);

        auto extView = Foundation::LoongPathUtils::GetFileExtension(vfsPath);
        if (extView.empty()) {
            LOONG_WARNING("Unable to recognize file format: file name '{}' does not contain extension, abort", vfsPath);
            return texture;
        }

        std::string extension(extView.data(), extView.size());
        if (extension == "png") {
            imgFileFormat = RHI::IMAGE_FILE_FORMAT_PNG;
        } else if (extension == "jpeg" || extension == "jpg") {
            imgFileFormat = RHI::IMAGE_FILE_FORMAT_JPEG;
        } else if (extension == "tiff" || extension == "tif") {
            imgFileFormat = RHI::IMAGE_FILE_FORMAT_TIFF;
        } else if (extension == "dds") {
            imgFileFormat = RHI::IMAGE_FILE_FORMAT_DDS;
        } else if (extension == "ktx") {
            imgFileFormat = RHI::IMAGE_FILE_FORMAT_KTX;
        } else {
            LOONG_ERROR("Unsupported file format '{}'", extension);
            return texture;
        }
    }

    RHI::TextureLoadInfo loadInfo;
    loadInfo.IsSRGB = isSrgb;
    LOONG_ASSERT(device != nullptr, "Device is nullptr, please make sure the RHI has been initialized");

    if (imgFileFormat == RHI::IMAGE_FILE_FORMAT_PNG || imgFileFormat == RHI::IMAGE_FILE_FORMAT_JPEG || imgFileFormat == RHI::IMAGE_FILE_FORMAT_TIFF) {
        RHI::RefCntAutoPtr<RHI::Image> image { nullptr };
        RHI::ImageLoadInfo imgLoadInfo;
        imgLoadInfo.Format = imgFileFormat;
        RHI::Image::CreateFromDataBlob(dataBlob, imgLoadInfo, &image);
        RHI::CreateTextureFromImage(image, loadInfo, device, &texture);
    } else if (imgFileFormat == RHI::IMAGE_FILE_FORMAT_DDS) {
        RHI::CreateTextureFromDDS(dataBlob, loadInfo, device, &texture);
    } else if (imgFileFormat == RHI::IMAGE_FILE_FORMAT_KTX) {
        RHI::CreateTextureFromKTX(dataBlob, loadInfo, device, &texture);
    } else {
        LOONG_ERROR("Failed to load texture '{}': Unknown format", vfsPath);
        return texture;
    }

    LOONG_VERIFY(texture != nullptr, "Create texture failed!");
    return texture;
}

RHI::RefCntAutoPtr<RHI::ITexture> LoongTextureLoader::CreateColor(uint8_t data[4], bool generateMipmap, const std::function<void(const std::string&)>& onDestroy)
{
    return {};
}

RHI::RefCntAutoPtr<RHI::ITexture> LoongTextureLoader::CreateFromMemory(uint8_t* data, uint32_t width, uint32_t height, bool generateMipmap, const std::function<void(const std::string&)>& onDestroy, int channelCount)
{
    return {};
}

} // namespace Loong
