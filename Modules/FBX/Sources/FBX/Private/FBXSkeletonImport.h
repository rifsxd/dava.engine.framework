#pragma once

#include "FBXUtils.h"

namespace DAVA
{
class SkeletonComponent;

namespace FBXImporterDetails
{
using VertexInfluence = std::pair<uint32, float32>; //[jointIndex, jointWeight]
using FbxControlPointInfluences = Vector<VertexInfluence>;

void ProcessSceneSkeletons(FbxScene* scene);
void GetControlPointInfluences(FbxSkin* fbxSkin, Vector<FbxControlPointInfluences>* outControlPointsInfluences, uint32* outMaxInfluenceCount);
SkeletonComponent* GetBuiltSkeletonComponent(FbxSkin* fbxSkin);

void ClearSkeletonCache();
};
};