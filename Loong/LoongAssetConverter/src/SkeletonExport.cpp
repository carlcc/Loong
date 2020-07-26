//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#include "SkeletonExport.h"
#include "Flags.h"
#include "LoongAsset/LoongMesh.h"
#include "LoongAsset/LoongModel.h"
#include "LoongFoundation/LoongDefer.h"
#include "LoongFoundation/LoongLogger.h"
#include "LoongFoundation/LoongMath.h"
#include "LoongFoundation/LoongPathUtils.h"
#include "LoongFoundation/LoongSerializer.h"
#include <assimp/matrix4x4.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <string>
#include <vector>

namespace Loong::AssetConverter {
//
//class LoongSkeleton {
//public:
//    struct VertexWeight {
//        uint32_t vertexId { 0 };
//        float weight { 0.0F };
//    };
//    // Bone struct defines who each vertex belongs to
//    // NOTE: A vertex may belongs to more than 1 bone
//    struct Bone {
//        std::string name {};
//        Math::Matrix4 offset {};
//        std::vector<VertexWeight> verticesWeights {};
//    };
//
//    // Node defines the skeleton hierachy
//    struct Node {
//        std::string name; // If name is not empty, this node is a bone, else this node is just used to manage transform
//        Node* parent { nullptr };
//        Math::Matrix4 transform {};
//        std::vector<Node*> children {};
//    };
//
//
//    struct VertexBoneBinding {
//        Math::IVector4 boneIds { 0};
//        Math::Vector4 weights { 0.0F };
//    };
//
//    std::vector<Math::Matrix4> finalBoneTransforms_;
//    std::vector<VertexBoneBinding> vertices_;
//};

class LoongSkeleton {
public:
    struct Node {
        Node* parent;
        std::vector<Node*> children;
        Math::Matrix4 offset;
    };

private:
    Node* root_ { nullptr };
};

class LoongAnimation {
public:
};

class LoongAnimationEvaluator {
public:
};

bool ExportSkeletonFiles(const aiScene* scene)
{
    auto& flags = Flags::Get();

    std::string fileName(Foundation::LoongPathUtils::GetFileName(flags.inputFile));
    std::string_view extension = Foundation::LoongPathUtils::GetFileExtension(fileName);
    std::string outputFileName = fileName.substr(0, fileName.length() - extension.length()) + ".lgskl";

    std::string outputPath = flags.outputDir + '/' + flags.modelPath + '/' + outputFileName;
    outputPath = Foundation::LoongPathUtils::Normalize(outputPath);

    FILE* ofs = fopen(outputPath.c_str(), "wb");
    if (ofs == nullptr) {
        LOONG_ERROR("Export model to file '{}' failed: Can not open the output file");
        return false;
    }
    OnScopeExit { fclose(ofs); };

    struct FileOutputStream : public Foundation::LoongArchiveOutputStream {
        explicit FileOutputStream(FILE* fout)
            : fout_(fout)
        {
        }
        bool operator()(void* d, size_t l)
        {
            return fwrite(d, l, 1, fout_) == 1;
        }

    private:
        FILE* fout_ { nullptr };
    };
    FileOutputStream outputStream(ofs);

    return true;
}

}
