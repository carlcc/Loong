#include "Flags.h"
#include "LoongFoundation/LoongLogger.h"
#include "ModelExport.h"
#include "SkeletonExport.h"
#include "TextureExport.h"
#include "AnimationExport.h"
#include <assimp/Importer.hpp>
#include <assimp/matrix4x4.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <iostream>

namespace Loong::AssetConverter {

bool ExportMaterialFiles(const aiScene* scene)
{
    return true;
}

int Convert()
{
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
    auto inputFile = Flags::Get().inputFile;
    const aiScene* scene = import.ReadFile(inputFile, modelParserFlags);

    // Don't check `scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE`, e.g.
    // Those files which only contain animations is "incomplete"
    if (!scene || !scene->mRootNode) {
        LOONG_ERROR("Load asset '{}' failed!", inputFile);
        return false;
    }

    if (!ExportModelFiles(scene)) {
        LOONG_ERROR("Export models failed!");
        return 1;
    }

    if (!ExportSkeletonFiles(scene)) {
        LOONG_ERROR("Export skeletons failed!");
        return 1;
    }

    if (!ExportAnimationFiles(scene)) {
        LOONG_ERROR("Export animations failed!");
        return 2;
    }

    if (!ExportMaterialFiles(scene)) {
        LOONG_ERROR("Export materials failed!");
        return 3;
    }

    if (!ExportTextureFiles(scene)) {
        LOONG_ERROR("Export textures failed!");
        return 4;
    }

    return 0;
}
}

int main(int argc, char* argv[])
{
    using namespace Loong::Foundation;
    using namespace Loong::AssetConverter;

    auto listener = Logger::Get().SubscribeLog([](const LogItem& logItem) {
        std::cout << "[" << logItem.level << "][" << logItem.location << "]: " << logItem.message << std::endl;
    });

    if (!Flags::ParseCommandLine(argc, argv)) {
        return -1;
    }

    return Convert();
}