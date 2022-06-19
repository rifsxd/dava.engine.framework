#pragma once

#include "FBXUtils.h"

namespace DAVA
{
class Entity;

namespace FBXImporterDetails
{
void ImportMeshToEntity(FbxNode* fbxNode, Entity* entity);
void ClearMeshCache();
};
};