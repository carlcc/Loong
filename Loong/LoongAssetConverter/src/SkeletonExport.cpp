//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//
#ifdef _MSC_VER
// e.g. This function or variable may be unsafe. Consider using fopen_s instead.
#pragma warning(disable : 4996)
#endif

#include "SkeletonExport.h"
#include "Flags.h"
#include "LoongAsset/LoongMesh.h"
#include "LoongAsset/LoongModel.h"
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

class LoongSkeleton {
public:
    struct Node {
        ~Node()
        {
            for (auto* node : children) {
                delete node;
            }
        }
        std::string name {};
        std::vector<Node*> children {};
        Math::Matrix4 transform {};

        template <class Archive>
        bool Serialize(Archive& archive) { return archive(name, children, transform); }
    };

    LoongSkeleton() = default;

    // Note: this function will take over the ownship
    LoongSkeleton(Node* root)
        : root_(root)
        , inverseMatrix_()
    {
    }

    ~LoongSkeleton()
    {
        delete root_;
    }

    const Math::Matrix4& GetInverseMatrix() const { return inverseMatrix_; }

    template <class Archive>
    bool Serialize(Archive& archive) { return archive(root_, inverseMatrix_); }

private:
    Node* root_ { nullptr };
    // This is the inverse of the root node's transform, used to transform the skeleton to model space
    Math::Matrix4 inverseMatrix_ {};
};

class LoongAnimation {
public:
};

class LoongAnimationEvaluator {
public:
};

bool ParseSkeleton(const aiNode* aiRootNode, LoongSkeleton::Node* rootNode)
{
    rootNode->name = aiRootNode->mName.C_Str();
    rootNode->transform = AiMatrix2LoongMatrix(aiRootNode->mTransformation);

    rootNode->children.resize(aiRootNode->mNumChildren);
    for (size_t i = 0; i < rootNode->children.size(); ++i) {
        auto& child = rootNode->children[i];
        child = new LoongSkeleton::Node;

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

    LoongSkeleton::Node* root = new LoongSkeleton::Node;
    LoongSkeleton skeleton(root);
    if (!ParseSkeleton(scene->mRootNode, root)) {
        return false;
    }

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
