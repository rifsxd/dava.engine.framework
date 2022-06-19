#pragma once

#include "FBXUtils.h"

namespace DAVA
{
class FilePath;

namespace FBXImporterDetails
{
FbxScene* ImportFbxScene(FbxManager* fbxManager, const FilePath& fbxPath);
void ProcessSceneHierarchyRecursive(FbxNode* fbxNode, Entity* entity);
};
};