//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#include "ModelExport.h"
#include "Flags.h"
#include "LoongFoundation/LoongAchiver.h"
#include "LoongFoundation/LoongDefer.h"
#include "LoongFoundation/LoongPathUtils.h"
#include <LoongAsset/LoongMesh.h>
#include <LoongAsset/LoongModel.h>
#include <LoongFoundation/LoongMath.h>
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

static void ProcessMesh(const void* transform, const struct aiMesh* mesh, const struct aiScene* scene, std::vector<Asset::LoongVertex>& outVertices, std::vector<uint32_t>& outIndices)
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

Math::AABB CalculateAABB(const std::vector<Asset::LoongMesh*>& meshes, const std::vector<std::string>& materials)
{
    Math::AABB aabb;
    if (meshes.empty()) {
        aabb = {
            { 0.F, 0.F, 0.F },
            { 0.F, 0.F, 0.F },
        };
        return aabb;
    }

    aabb = meshes[0]->GetAABB();
    for (size_t i = 1; i < meshes.size(); ++i) {
        auto& mesh = *meshes[i];
        auto& meshAabb = mesh.GetAABB();
        aabb.min.x = std::min(aabb.min.x, meshAabb.min.x);
        aabb.min.y = std::min(aabb.min.y, meshAabb.min.y);
        aabb.min.z = std::min(aabb.min.z, meshAabb.min.z);
        aabb.max.x = std::min(aabb.max.x, meshAabb.max.x);
        aabb.max.y = std::min(aabb.max.y, meshAabb.max.y);
        aabb.max.z = std::min(aabb.max.z, meshAabb.max.z);
    }
    return aabb;
}

bool ExportModelFiles(const aiScene* scene)
{
    auto& flags = Flags::Get();

    std::string fileName(Foundation::LoongPathUtils::GetFileName(flags.inputFile));
    std::string_view extension = Foundation::LoongPathUtils::GetFileExtension(fileName);
    std::string outputFileName = fileName.substr(0, fileName.length() - extension.length()) + ".lgmdl";

    std::string outputPath = flags.outputDir + '/' + outputFileName;

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

    Foundation::LoongArchiveOutputStream outputStream(ofs);

    Foundation::LoongArchive<Foundation::LoongArchiveOutputStream> archive(outputStream);

    return model.Serialize(archive);
}

}
