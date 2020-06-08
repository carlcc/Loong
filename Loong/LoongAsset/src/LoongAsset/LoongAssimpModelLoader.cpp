//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#include "LoongAssimpModelLoader.h"
#include "LoongAsset/LoongMesh.h"
#include "LoongFileSystem/LoongFileSystem.h"
#include "LoongFoundation/LoongLogger.h"
#include <assimp/Importer.hpp>
#include <assimp/matrix4x4.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <vector>

namespace Loong::Asset {

bool LoongAssimpModelLoader::LoadModel(const std::string& fileName, std::vector<LoongMesh*>& meshes, std::vector<std::string>& materials)
{
    int64_t fileSize = FS::LoongFileSystem::GetFileSize(fileName);
    if (fileSize <= 0) {
        LOONG_ERROR("Failed to load model '{}': Wrong file size", fileName);
        return false;
    }
    std::vector<uint8_t> buffer(fileSize);
    assert(FS::LoongFileSystem::LoadFileContent(fileName, buffer.data(), fileSize) == fileSize);

    Assimp::Importer import;
    unsigned int modelParserFlags = 0;

    modelParserFlags |= aiProcess_Triangulate;
    modelParserFlags |= aiProcess_GenSmoothNormals;
    modelParserFlags |= aiProcess_OptimizeMeshes;
    modelParserFlags |= aiProcess_OptimizeGraph;
    modelParserFlags |= aiProcess_FindInstances;
    modelParserFlags |= aiProcess_CalcTangentSpace;
    modelParserFlags |= aiProcess_JoinIdenticalVertices;
    modelParserFlags |= aiProcess_Debone;
    modelParserFlags |= aiProcess_FindInvalidData;
    modelParserFlags |= aiProcess_ImproveCacheLocality;
    modelParserFlags |= aiProcess_GenUVCoords;
    // modelParserFlags |= aiProcess_PreTransformVertices; // incompatible with aiProcess_OptimizeGraph
    const aiScene* scene = import.ReadFileFromMemory(buffer.data(), buffer.size(), modelParserFlags);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        LOONG_ERROR("Load model '{}' failed!", fileName);
        return false;
    }

    ProcessMaterials(scene, materials);

    aiMatrix4x4 identity;

    ProcessNode(&identity, scene->mRootNode, scene, meshes);

    return true;
}

void LoongAssimpModelLoader::ProcessNode(const void* transform, const struct aiNode* node, const struct aiScene* scene, std::vector<LoongMesh*>& meshes)
{
    aiMatrix4x4 nodeTransformation = *reinterpret_cast<const aiMatrix4x4*>(transform) * node->mTransformation;

    // Process all the node's meshes (if any)
    for (uint32_t i = 0; i < node->mNumMeshes; ++i) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];

        std::vector<LoongVertex> vertices;
        std::vector<uint32_t> indices;
        ProcessMesh(&nodeTransformation, mesh, scene, vertices, indices);

        meshes.push_back(new LoongMesh(std::move(vertices), std::move(indices), mesh->mMaterialIndex)); // The model will handle mesh destruction
    }

    // Then do the same for each of its children
    for (uint32_t i = 0; i < node->mNumChildren; ++i) {
        ProcessNode(&nodeTransformation, node->mChildren[i], scene, meshes);
    }
}

void LoongAssimpModelLoader::ProcessMaterials(const struct aiScene* scene, std::vector<std::string>& materials)
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

void LoongAssimpModelLoader::ProcessMesh(const void* transform, const struct aiMesh* mesh, const struct aiScene* scene, std::vector<LoongVertex>& outVertices, std::vector<uint32_t>& outIndices)
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

}
