#pragma once

#include "FBXUtils.h"

namespace DAVA
{
class NMaterial;

namespace FBXImporterDetails
{
NMaterial* ImportMaterial(const FbxSurfaceMaterial* fbxMaterial, uint32 maxVertexInfluence);
void ClearMaterialCache();
}
};