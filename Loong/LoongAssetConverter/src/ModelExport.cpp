//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#ifdef _MSC_VER
// e.g. This function or variable may be unsafe. Consider using fopen_s instead.
#pragma warning(disable : 4996)
#endif

#include "ModelExport.h"
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

inline Math::Matrix4 AiMatrix2LoongMatrix(const aiMatrix4x4& matrix)
{
    // aiMatrix4x4 is row major, while our Matrix4 is column major
    return Math::Matrix4(
        matrix.a1, matrix.b1, matrix.c1, matrix.d1,
        matrix.a2, matrix.b2, matrix.c2, matrix.d2,
        matrix.a3, matrix.b3, matrix.c3, matrix.d3,
        matrix.a4, matrix.b4, matrix.c4, matrix.d4);
}

inline Math::Vector3 AiVector2LoongVector(const aiVector3D& v)
{
    return { v.x, v.y, v.z };
}

static void ProcessMesh(const void* transform, const struct aiMesh* mesh, const struct aiScene* scene, std::vector<Asset::LoongVertex>& outVertices, std::vector<uint32_t>& outIndices)
{
    aiMatrix4x4 meshTransformation = *reinterpret_cast<const aiMatrix4x4*>(transform);
    Math::Matrix3 rotTransform(Math::Transpose(Math::Inverse(AiMatrix2LoongMatrix(meshTransformation))));
    for (uint32_t i = 0; i < mesh->mNumVertices; ++i) {
        aiVector3D position = meshTransformation * mesh->mVertices[i];
        Math::Vector3 normal = rotTransform * AiVector2LoongVector(mesh->mNormals ? mesh->mNormals[i] : aiVector3D(0.0f, 0.0f, 0.0f));
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
}

static void ProcessNode(const void* transform, const struct aiNode* node, const struct aiScene* scene, std::vector<Asset::LoongMesh*>& meshes)
{
    aiMatrix4x4 nodeTransformation = *reinterpret_cast<const aiMatrix4x4*>(transform) * node->mTransformation;

    // Process all the node's meshes (if any)
    for (uint32_t i = 0; i < node->mNumMeshes; ++i) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];

        std::vector<Asset::LoongVertex> vertices;
        std::vector<uint32_t> indices;
        ProcessMesh(&nodeTransformation, mesh, scene, vertices, indices);

        meshes.push_back(new Asset::LoongMesh(std::move(vertices), std::move(indices), mesh->mMaterialIndex)); // The model will handle mesh destruction
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

    return Foundation::Serialize(model, outputStream);
}

}
