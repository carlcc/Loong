//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//
#ifdef _MSC_VER
// e.g. This function or variable may be unsafe. Consider using fopen_s instead.
#pragma warning(disable : 4996)
#endif

#include "SkeletonExport.h"
#include "Flags.h"
#include "LoongAsset/LoongSkeleton.h"
#include "LoongFoundation/LoongDefer.h"
#include "LoongFoundation/LoongLogger.h"
#include "LoongFoundation/LoongMath.h"
#include "LoongFoundation/LoongPathUtils.h"
#include "LoongFoundation/LoongSerializer.h"
#include "Utils.h"
#include <assimp/matrix4x4.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <string>
#include <vector>

namespace Loong::AssetConverter {

bool ParseSkeleton(const aiNode* aiRootNode, Asset::LoongSkeleton::Node& rootNode)
{
    rootNode.name = aiRootNode->mName.C_Str();
    rootNode.transform = AiMatrix2LoongMatrix(aiRootNode->mTransformation);

    rootNode.children.resize(aiRootNode->mNumChildren);
    for (size_t i = 0; i < rootNode.children.size(); ++i) {
        auto& child = rootNode.children[i];

        if (!ParseSkeleton(aiRootNode->mChildren[i], child)) {
            return false;
        }
    }
    return true;
}

bool ExportSkeletonFiles(const aiScene* scene)
{
    auto& flags = Flags::Get();

    std::string fileName(Foundation::LoongPathUtils::GetFileName(flags.inputFile));
    std::string_view extension = Foundation::LoongPathUtils::GetFileExtension(fileName);
    std::string outputFileName = fileName.substr(0, fileName.length() - extension.length()) + ".lgskl";

    std::string outputPath = flags.outputDir + '/' + flags.modelPath + '/' + outputFileName;
    outputPath = Foundation::LoongPathUtils::Normalize(outputPath);

    Asset::LoongSkeleton::Node root;
    if (!ParseSkeleton(scene->mRootNode, root)) {
        return false;
    }
    Asset::LoongSkeleton skeleton(std::move(root));

    FILE* ofs = fopen(outputPath.c_str(), "wb");
    if (ofs == nullptr) {
        LOONG_ERROR("Export skeleton to file '{}' failed: Can not open the output file");
        return false;
    }
    OnScopeExit { fclose(ofs); };

    Foundation::LoongArchiveFileOutputStream outputStream(ofs);

    return Foundation::Serialize(skeleton, outputStream);
}

}
