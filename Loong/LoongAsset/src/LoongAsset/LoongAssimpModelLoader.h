//
// Copyright (c) 2020 Carl Chen. All rights reserved.
//

#pragma once

#include "ILoongModelLoader.h"
#include "LoongAsset/LoongVertex.h"

struct aiScene;
struct aiNode;
struct aiMesh;

namespace Loong::Asset {

class LoongAssimpModelLoader : public ILoongModelLoader {
public:
    bool LoadModel(const std::string& fileName, std::vector<LoongMesh*>& meshes, std::vector<std::string>& materials) override;

private:
    static void ProcessMaterials(const struct aiScene* scene, std::vector<std::string>& materials);

    static void ProcessNode(const void* transform, const struct aiNode* node, const struct aiScene* scene, std::vector<LoongMesh*>& meshes);

    static void ProcessMesh(const void* transform, const struct aiMesh* mesh, const struct aiScene* scene, std::vector<LoongVertex>& outVertices, std::vector<uint32_t>& outIndices);
};

}