#include "FBXUtils.h"
#include "Logger/Logger.h"

namespace DAVA
{
namespace FBXImporterDetails
{
namespace FBXImporterUtilsDetails
{
UnorderedMap<FastName, uint32> nodeIDcache;
UnorderedMap<const FbxNode*, FastName> nodeUIDMap;
}

Matrix4 ToMatrix4(const FbxAMatrix& fbxMatrix)
{
    Matrix4 mx;

    for (int32 r = 0; r < 4; ++r)
        for (int32 c = 0; c < 4; ++c)
            mx._data[r][c] = float32(fbxMatrix.Get(r, c));

    return mx;
}

Vector3 ToVector3(const FbxVector4& fbxVector)
{
    return Vector3(float32(fbxVector[0]), float32(fbxVector[1]), float32(fbxVector[2]));
}

Vector2 ToVector2(const FbxVector2& fbxVector)
{
    return Vector2(float32(fbxVector[0]), float32(fbxVector[1]));
}

const char* GetFBXTexturePath(const FbxProperty& textureProperty)
{
    FbxTexture* fbxTexture = nullptr;
    if (textureProperty.GetSrcObjectCount<FbxLayeredTexture>() > 0)
    {
        FbxLayeredTexture* layeredTexture = textureProperty.GetSrcObject<FbxLayeredTexture>(0);
        if (layeredTexture->GetSrcObjectCount<FbxTexture>() > 0)
            fbxTexture = layeredTexture->GetSrcObject<FbxTexture>(0);
    }
    else
    {
        if (textureProperty.GetSrcObjectCount<FbxTexture>() > 0)
            textureProperty.GetSrcObject<FbxTexture>(0);
    }

    FbxFileTexture* fbxFileTexture = FbxCast<FbxFileTexture>(fbxTexture);
    if (fbxFileTexture)
        return fbxFileTexture->GetFileName();

    return nullptr;
}

FastName GenerateNodeUID(const FbxNode* node)
{
    using namespace FBXImporterUtilsDetails;

    auto foundNode = nodeUIDMap.find(node);
    if (foundNode != nodeUIDMap.end())
    {
        return foundNode->second;
    }

    FastName nodeID = FastName(Format("node-%s", node->GetName()).c_str());

    auto foundID = nodeIDcache.find(nodeID);
    if (foundID != nodeIDcache.end())
    {
        foundID->second++;
        nodeID = FastName(Format("%s-%d", nodeID.c_str(), foundID->second).c_str());
        Logger::Warning("[FBXImport] ID for node '%s' is duplicate. Generated ID: '%s'", node->GetName(), nodeID.c_str());
    }
    else
    {
        nodeIDcache.insert(std::make_pair(nodeID, 0));
    }

    nodeUIDMap[node] = nodeID;

    return nodeID;
}

void ClearNodeUIDCache()
{
    FBXImporterUtilsDetails::nodeIDcache.clear();
    FBXImporterUtilsDetails::nodeUIDMap.clear();
}

FbxAMatrix GetGeometricTransform(const FbxNode* fbxNode)
{
    const FbxVector4 gT = fbxNode->GetGeometricTranslation(FbxNode::eSourcePivot);
    const FbxVector4 gR = fbxNode->GetGeometricRotation(FbxNode::eSourcePivot);
    const FbxVector4 gS = fbxNode->GetGeometricScaling(FbxNode::eSourcePivot);

    return FbxAMatrix(gT, gR, gS);
}

Matrix4 EvaluateNodeTransform(FbxNode* fbxNode, const FbxTime& time)
{
    bool isSkeletonRoot = (fbxNode->GetSkeleton() != nullptr && fbxNode->GetSkeleton()->IsSkeletonRoot());
    if (isSkeletonRoot)
    {
        return ToMatrix4(fbxNode->EvaluateGlobalTransform(time));
    }
    else if (fbxNode->GetParent() != nullptr)
    {
        //To use EvaluateLocalTransform() incorrect in cases when node-transform inheritance is RrSs or Rrs
        return ToMatrix4(fbxNode->GetParent()->EvaluateGlobalTransform(time).Inverse() * fbxNode->EvaluateGlobalTransform(time));
    }
    else
    {
        return ToMatrix4(fbxNode->EvaluateLocalTransform(time));
    }
}
}
};