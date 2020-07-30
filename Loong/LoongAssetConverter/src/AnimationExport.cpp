//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//
#ifdef _MSC_VER
// e.g. This function or variable may be unsafe. Consider using fopen_s instead.
#pragma warning(disable : 4996)
#endif

#include "Flags.h"
#include "LoongAsset/LoongMesh.h"
#include "LoongAsset/LoongModel.h"
#include "LoongFoundation/LoongDefer.h"
#include "LoongFoundation/LoongLogger.h"
#include "LoongFoundation/LoongMath.h"
#include "LoongFoundation/LoongPathUtils.h"
#include "LoongFoundation/LoongSerializer.h"
#include "SkeletonExport.h"
#include "Utils.h"
#include <assimp/matrix4x4.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <string>
#include <vector>

namespace Loong::AssetConverter {

class LoongAnimation {
public:
    struct Channel {
        std::string nodeName {};
        std::vector<float> positionKeyTimes {};
        std::vector<Math::Vector3> positionKeys {};
        std::vector<float> rotationKeyTimes {};
        std::vector<Math::Quat> rotationKeys {};
        std::vector<float> scaleKeyTimes {};
        std::vector<Math::Vector3> scaleKeys {};

        template <class Archive>
        bool Serialize(Archive& archive)
        {
            return archive(nodeName, positionKeyTimes, positionKeys, rotationKeyTimes,
                rotationKeys, scaleKeyTimes, scaleKeys);
        }
    };

    template <class Archive>
    bool Serialize(Archive& archive) { return archive(name, duration, ticksPerSecond, channels); }

    std::string name {};
    float duration { 0.0F };
    float ticksPerSecond { 0.0F };
    std::vector<Channel> channels {};
};

bool ParseChannel(const aiNodeAnim* aiChannel, LoongAnimation::Channel* channel)
{
    channel->nodeName = aiChannel->mNodeName.C_Str();

    channel->positionKeys.resize(aiChannel->mNumPositionKeys);
    channel->positionKeyTimes.resize(aiChannel->mNumPositionKeys);
    for (uint32_t i = 0; i < aiChannel->mNumPositionKeys; ++i) {
        auto& value = aiChannel->mPositionKeys[i].mValue;
        channel->positionKeyTimes[i] = aiChannel->mPositionKeys[i].mTime;
        channel->positionKeys[i] = { value.x, value.y, value.z };
    }

    channel->rotationKeys.resize(aiChannel->mNumRotationKeys);
    channel->rotationKeyTimes.resize(aiChannel->mNumRotationKeys);
    for (uint32_t i = 0; i < aiChannel->mNumRotationKeys; ++i) {
        auto& value = aiChannel->mRotationKeys[i].mValue;
        channel->rotationKeyTimes[i] = aiChannel->mRotationKeys[i].mTime;
        channel->rotationKeys[i] = { value.x, value.y, value.z, value.w };
    }

    channel->scaleKeys.resize(aiChannel->mNumScalingKeys);
    channel->scaleKeyTimes.resize(aiChannel->mNumScalingKeys);
    for (uint32_t i = 0; i < aiChannel->mNumScalingKeys; ++i) {
        auto& value = aiChannel->mScalingKeys[i].mValue;
        channel->scaleKeyTimes[i] = aiChannel->mScalingKeys[i].mTime;
        channel->scaleKeys[i] = { value.x, value.y, value.z };
    }
}

bool ParseAnimation(const aiAnimation* aiAnim, LoongAnimation* anim)
{
    anim->name = aiAnim->mName.C_Str();
    anim->duration = aiAnim->mDuration;
    anim->ticksPerSecond = aiAnim->mTicksPerSecond;
    anim->channels.resize(aiAnim->mNumChannels);

    for (uint32_t i = 0; i < aiAnim->mNumChannels; ++i) {
        if (!ParseChannel(aiAnim->mChannels[i], &anim->channels[i])) {
            return false;
        }
    }
    return true;
}

bool ExportAnimationFiles(const aiScene* scene)
{
    auto& flags = Flags::Get();

    std::string fileName(Foundation::LoongPathUtils::GetFileName(flags.inputFile));
    std::string_view extension = Foundation::LoongPathUtils::GetFileExtension(fileName);
    std::string outputFileNameWithoutExt = fileName.substr(0, fileName.length() - extension.length());

    for (uint32_t i = 0; i < scene->mNumAnimations; ++i) {

        std::string outputPath = flags.outputDir + '/' + flags.modelPath + '/'
            + outputFileNameWithoutExt + "_" + scene->mAnimations[i]->mName.C_Str() + ".lganim";
        outputPath = Foundation::LoongPathUtils::Normalize(outputPath);

        LoongAnimation animation;
        if (!ParseAnimation(scene->mAnimations[i], &animation)) {
            return false;
        }

        FILE* ofs = fopen(outputPath.c_str(), "wb");
        if (ofs == nullptr) {
            LOONG_ERROR("Export animation to file '{}' failed: Can not open the output file");
            return false;
        }
        OnScopeExit { fclose(ofs); };

        Foundation::LoongArchiveFileOutputStream outputStream(ofs);

        if (!Foundation::Serialize(animation, outputStream)) {
            return false;
        }
    }
    return true;
}

}
