#include "FBXSceneImport.h"
#include "FBXMeshImport.h"

#include <Logger/Logger.h>
#include <FileSystem/FilePath.h>
#include <Scene3D/Entity.h>
#include <Scene3D/Components/TransformComponent.h>

namespace DAVA
{
namespace FBXImporterDetails
{
FbxScene* ImportFbxScene(FbxManager* fbxManager, const FilePath& fbxPath)
{
    FbxIOSettings* fbxIOSettings = FbxIOSettings::Create(fbxManager, IOSROOT);
    FbxImporter* importer = FbxImporter::Create(fbxManager, "fbxImporter");

    bool initSuccess = importer->Initialize(fbxPath.GetAbsolutePathname().c_str());
    if (!initSuccess)
    {
        Logger::Error("FbxImporter Initialization error: %s", importer->GetStatus().GetErrorString());
        return nullptr;
    }

    FbxScene* fbxScene = FbxScene::Create(fbxManager, "importedScene");
    bool importSuccess = importer->Import(fbxScene);
    if (!importSuccess)
    {
        Logger::Error("FBX Import error: %s", importer->GetStatus().GetErrorString());
        return nullptr;
    }
    importer->Destroy();

    FbxAxisSystem::MayaZUp.ConvertScene(fbxScene); // UpVector = ZAxis, CoordSystem = RightHanded

    //FbxSystemUnit::ConvertScene() doesn't work properly in some cases, so we scale scene manually
    double conversionFactor = fbxScene->GetGlobalSettings().GetSystemUnit().GetConversionFactorTo(FbxSystemUnit::m);
    FbxVector4 rootNodeScaling = FbxVector4(fbxScene->GetRootNode()->LclScaling.Get()) * conversionFactor;
    fbxScene->GetRootNode()->LclScaling.Set(rootNodeScaling);

    return fbxScene;
}

void ProcessSceneHierarchyRecursive(FbxNode* fbxNode, Entity* entity)
{
    entity->SetName(fbxNode->GetName());

    const FbxMesh* fbxMesh = fbxNode->GetMesh();
    if (fbxMesh != nullptr)
    {
        ImportMeshToEntity(fbxNode, entity);
    }
    else
    {
        Matrix4 transform = ToMatrix4(fbxNode->EvaluateLocalTransform());
        TransformComponent* tc = entity->GetComponent<TransformComponent>();
        tc->SetLocalTransform(transform);
    }

    int32 childCount = fbxNode->GetChildCount();
    for (int32 c = 0; c < childCount; ++c)
    {
        ScopedPtr<Entity> childEntity(new Entity());
        entity->AddNode(childEntity);

        ProcessSceneHierarchyRecursive(fbxNode->GetChild(c), childEntity);
    }
}

}; //ns FBXImporterDetails
}; //ns DAVA
