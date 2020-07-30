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
#include "ModelExport.h"
#include "Utils.h"
#include <assimp/matrix4x4.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <string>
#include <vector>

namespace Loong::AssetConverter {

static void ProcessMaterials(const struct aiScene* scene, std::vector<std::string>& materials)
{
    for (uint32_t i = 0; i < scene->mNumMaterials; ++i) {
        aiMaterial* material = scene->mMaterials[i];
        if (material) {
            aiString name;
            aiGetMaterialString(material, AI_MATKEY_NAME, &name);
            materials.emplace_back(name.C_Str());
        }
    }
}

static void ProcessMesh(const void* transform, const struct aiMesh* mesh, const struct aiScene* scene,
    std::vector<Asset::LoongVertex>& outVertices, std::vector<uint32_t>& outIndices,
    std::vector<Asset::LoongMesh::BoneBinding>& boneInfos, Asset::LoongMesh::BoneInfoMap& boneInfoMap)
{
    aiMatrix4x4 meshTransformation = *reinterpret_cast<const aiMatrix4x4*>(transform);

    for (uint32_t i = 0; i < mesh->mNumVertices; ++i) {
        aiVector3D position = meshTransformation * mesh->mVertices[i];
        aiVector3D normal = meshTransformation * (mesh->mNormals ? mesh->mNormals[i] : aiVector3D(0.0f, 0.0f, 0.0f));
        aiVector3D texCoords = mesh->mTextureCoords[0] ? mesh->mTextureCoords[0][i] : aiVector3D(0.0f, 0.0f, 0.0f);
        aiVector3D tangent = mesh->mTangents ? meshTransformation * mesh->mTangents[i] : aiVector3D(0.0f, 0.0f, 0.0f);
        aiVector3D bitangent = mesh->mBitangents ? meshTransformation * mesh->mBitangents[i] : aiVector3D(0.0f, 0.0f, 0.0f);

        outVertices.push_back(
            {
                { position.x, position.y, position.z },
                { texCoords.x, texCoords.y },
                { normal.x, normal.y, normal.z },
                { tangent.x, tangent.y, tangent.z },
                { bitangent.x, bitangent.y, bitangent.z },
            });
    }

    for (uint32_t faceID = 0; faceID < mesh->mNumFaces; ++faceID) {
        auto& face = mesh->mFaces[faceID];

        for (size_t indexID = 0; indexID < 3; ++indexID)
            outIndices.push_back(face.mIndices[indexID]);
    }

    if (mesh->mNumBones > 0) {
        auto AddWeigth = [](Asset::LoongMesh::BoneBinding& boneInfo, uint32_t boneId, float weight) -> bool {
            for (int i = 0; i < 4; ++i) {
                if (boneInfo.boneWeights[i] == 0.0F) {
                    boneInfo.boneIndices[i] = boneId;
                    boneInfo.boneWeights[i] = weight;
                    return true;
                }
            }
            LOONG_ERROR("LoongModel support at most 4 bones for a certain vertex, but more than 4 bones were found");
            return false;
        };

        assert(boneInfoMap.empty());
        uint32_t boneIndexCounter = 0;
        uint32_t boneId = 0;

        boneInfos.resize(outVertices.size());
        for (uint32_t i = 0; i < mesh->mNumBones; ++i) {
            auto* aiBone_ = mesh->mBones[i];
            std::string boneName = aiBone_->mName.C_Str();
            auto it = boneInfoMap.find(boneName);
            if (it == boneInfoMap.end()) {
                boneId = boneIndexCounter++;
                boneInfoMap.insert({ boneName, { boneId, AiMatrix2LoongMatrix(aiBone_->mOffsetMatrix) } });
            } else {
                boneId = it->second.index;
            }

            for (uint32_t j = 0; j < aiBone_->mNumWeights; ++j) {
                auto vertexId = aiBone_->mWeights[j].mVertexId;
                float weight = aiBone_->mWeights[j].mWeight;
                // TODO: Should we abort if it returns false?
                AddWeigth(boneInfos[vertexId], boneId, weight);
            }
        }
    }
}

static void ProcessNode(const void* transform, const struct aiNode* node, const struct aiScene* scene, std::vector<Asset::LoongMesh*>& meshes)
{
    aiMatrix4x4 nodeTransformation = *reinterpret_cast<const aiMatrix4x4*>(transform) * node->mTransformation;

    // Process all the node's meshes (if any)
    for (uint32_t i = 0; i < node->mNumMeshes; ++i) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];

        std::vector<Asset::LoongVertex> vertices;
        std::vector<uint32_t> indices;
        std::vector<Asset::LoongMesh::BoneBinding> boneInfos;
        Asset::LoongMesh::BoneInfoMap boneInfoMap;
        ProcessMesh(&nodeTransformation, mesh, scene, vertices, indices, boneInfos, boneInfoMap);

        // The model will handle mesh destruction
        meshes.push_back(new Asset::LoongMesh(std::move(vertices), std::move(indices), std::move(boneInfos), std::move(boneInfoMap), mesh->mMaterialIndex));
    }

    // Then do the same for each of its children
    for (uint32_t i = 0; i < node->mNumChildren; ++i) {
        ProcessNode(&nodeTransformation, node->mChildren[i], scene, meshes);
    }
}

bool ExportModelFiles(const aiScene* scene)
{
    auto& flags = Flags::Get();

    std::string fileName(Foundation::LoongPathUtils::GetFileName(flags.inputFile));
    std::string_view extension = Foundation::LoongPathUtils::GetFileExtension(fileName);
    std::string outputFileName = fileName.substr(0, fileName.length() - extension.length()) + ".lgmdl";

    std::string outputPath = flags.outputDir + '/' + flags.modelPath + '/' + outputFileName;
    outputPath = Foundation::LoongPathUtils::Normalize(outputPath);

    FILE* ofs = fopen(outputPath.c_str(), "wb");
    if (ofs == nullptr) {
        LOONG_ERROR("Export model to file '{}' failed: Can not open the output file");
        return false;
    }
    OnScopeExit { fclose(ofs); };

    std::vector<Asset::LoongMesh*> meshes;
    std::vector<std::string> materials;

    ProcessMaterials(scene, materials);

    aiMatrix4x4 identity;
    ProcessNode(&identity, scene->mRootNode, scene, meshes);

    Asset::LoongModel model(std::move(meshes), std::move(materials));

    Foundation::LoongArchiveFileOutputStream outputStream(ofs);

    return Foundation::Serialize(model, outputStream);
}

}
