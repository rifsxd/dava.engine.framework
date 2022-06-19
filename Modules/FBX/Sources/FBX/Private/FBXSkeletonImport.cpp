#include "FBXSkeletonImport.h"

#include "Debug/DVAssert.h"
#include "Logger/Logger.h"
#include "Scene3D/Components/SkeletonComponent.h"

namespace DAVA
{
namespace FBXSkeletonImportDetails
{
//namespace declarations

struct FBXJoint
{
    FbxAMatrix bindTransformInv;
    const FbxSkeleton* joint = nullptr;
    const FbxSkeleton* parentJoint = nullptr;

    uint32 hierarchyDepth = 0;
    uint32 parentIndex = SkeletonComponent::INVALID_JOINT_INDEX;
};

void ProcessSkinsRecursive(FbxNode* fbxNode);
void ProcessSkin(FbxSkin* fbxSkin);

void ProcessSkeletonsHierarchyRecursive(FbxNode* fbxNode);
void ProcessSkeletonHierarchy(const FbxSkeleton* fbxSkeleton);

void BuildSkeletonComponents();

void CollectSkeletonNodes(const FbxSkeleton* fbxSkeleton, Vector<FBXJoint>* fbxJoints, const FbxSkeleton* fbxParentSkeleton, uint32 depth);

const FbxSkeleton* FindSkeletonRoot(FbxSkin* fbxSkin);
const FbxSkeleton* FindSkeletonRoot(const FbxSkeleton* fbxSkeleton);

//namespace members

Map<const FbxSkeleton*, Vector<FBXJoint>> linkedSkeletons; // [rootJoint, jointsArray]
Map<FbxSkin*, std::pair<Vector<FBXImporterDetails::FbxControlPointInfluences>, uint32>> processedSkins; //[skin, [ControlPointInfluences, maxInfluence]];
Map<const FbxSkeleton*, SkeletonComponent*> builtSkeletonComponents;

}; //ns FBXSkeletonImportDetails

//////////////////////////////////////////////////////////////////////////

namespace FBXImporterDetails
{
void ProcessSceneSkeletons(FbxScene* scene)
{
    FBXSkeletonImportDetails::ProcessSkeletonsHierarchyRecursive(scene->GetRootNode());
    FBXSkeletonImportDetails::ProcessSkinsRecursive(scene->GetRootNode());
    FBXSkeletonImportDetails::BuildSkeletonComponents();
}

void GetControlPointInfluences(FbxSkin* fbxSkin, Vector<FbxControlPointInfluences>* outControlPointsInfluences, uint32* outMaxInfluenceCount)
{
    DVASSERT(FBXSkeletonImportDetails::processedSkins.count(fbxSkin) != 0);

    if (outControlPointsInfluences != nullptr)
        *outControlPointsInfluences = FBXSkeletonImportDetails::processedSkins[fbxSkin].first;

    if (outMaxInfluenceCount != nullptr)
        *outMaxInfluenceCount = FBXSkeletonImportDetails::processedSkins[fbxSkin].second;
}

SkeletonComponent* GetBuiltSkeletonComponent(FbxSkin* fbxSkin)
{
    const FbxSkeleton* fbxSkeleton = FBXSkeletonImportDetails::FindSkeletonRoot(fbxSkin);

    DVASSERT(fbxSkeleton != nullptr);
    DVASSERT(FBXSkeletonImportDetails::builtSkeletonComponents.count(fbxSkeleton) != 0);

    return FBXSkeletonImportDetails::builtSkeletonComponents[fbxSkeleton];
}

void ClearSkeletonCache()
{
    using namespace FBXSkeletonImportDetails;

    linkedSkeletons.clear();
    processedSkins.clear();

    for (auto& it : builtSkeletonComponents)
        SafeDelete(it.second);
    builtSkeletonComponents.clear();
}

}; //ns FBXImporterDetails

//////////////////////////////////////////////////////////////////////////
//FBXSkeletonImportDetails namespace definitions

namespace FBXSkeletonImportDetails
{
void ProcessSkinsRecursive(FbxNode* fbxNode)
{
    const FbxMesh* fbxMesh = fbxNode->GetMesh();
    if (fbxMesh != nullptr && fbxMesh->GetDeformerCount(FbxDeformer::eSkin) > 0)
        ProcessSkin(static_cast<FbxSkin*>(fbxMesh->GetDeformer(0, FbxDeformer::eSkin)));

    int32 childCount = fbxNode->GetChildCount();
    for (int32 c = 0; c < childCount; ++c)
        ProcessSkinsRecursive(fbxNode->GetChild(c));
}

void ProcessSkin(FbxSkin* fbxSkin)
{
    using namespace FBXImporterDetails;

    DVASSERT(fbxSkin != nullptr);
    DVASSERT(processedSkins.count(fbxSkin) == 0);

    const FbxMesh* fbxMesh = static_cast<const FbxMesh*>(fbxSkin->GetGeometry());
    const FbxSkeleton* skeletonRoot = FindSkeletonRoot(fbxSkin);
    FbxNode* fbxMeshNode = fbxMesh->GetNode();

    DVASSERT(skeletonRoot != nullptr);
    DVASSERT(linkedSkeletons.count(skeletonRoot) != 0);

    std::pair<Vector<FbxControlPointInfluences>, uint32>& skinData = processedSkins[fbxSkin];
    Vector<FbxControlPointInfluences>& controlPointsInfluences = skinData.first;
    uint32& maxInfluenceCount = skinData.second;

    maxInfluenceCount = 0;
    controlPointsInfluences.resize(fbxMesh->GetControlPointsCount());

    Vector<FBXJoint>& fbxJoints = linkedSkeletons[skeletonRoot];
    FbxAMatrix meshTransformInv = fbxMeshNode->EvaluateGlobalTransform().Inverse();

    FbxAMatrix linkTransform, nodeTransform;
    int32 clusterCount = fbxSkin->GetClusterCount();
    for (int32 c = 0; c < clusterCount; ++c)
    {
        FbxCluster* cluster = fbxSkin->GetCluster(c);
        FbxNode* linkNode = cluster->GetLink();

        if (cluster->GetLinkMode() != FbxCluster::eNormalize)
        {
            static const char* linkModes[] = { "Normalize", "Additive", "Total1" };
            Logger::Warning("[FBXImporter] Skin cluster linked with %s mode (node: %s)!", linkModes[cluster->GetLinkMode()], linkNode->GetName());
        }

        const FbxSkeleton* linkedJoint = linkNode->GetSkeleton();
        auto foundFbxJoint = std::find_if(fbxJoints.begin(), fbxJoints.end(), [&linkedJoint](const FBXJoint& item) {
            return (item.joint == linkedJoint);
        });
        DVASSERT(foundFbxJoint != fbxJoints.end());

        uint32 jointIndex = uint32(std::distance(fbxJoints.begin(), foundFbxJoint));

        cluster->GetTransformLinkMatrix(linkTransform);
        cluster->GetTransformMatrix(nodeTransform);

        linkTransform *= GetGeometricTransform(linkNode);
        nodeTransform *= GetGeometricTransform(fbxMeshNode);

        foundFbxJoint->bindTransformInv = linkTransform.Inverse() * nodeTransform * meshTransformInv;

        int32 indicesCount = cluster->GetControlPointIndicesCount();
        for (int32 i = 0; i < indicesCount; ++i)
        {
            int32 controlPointIndex = cluster->GetControlPointIndices()[i];
            float32 controlPointWeight = float32(cluster->GetControlPointWeights()[i]);

            controlPointsInfluences[controlPointIndex].emplace_back(jointIndex, controlPointWeight);
            maxInfluenceCount = Max(maxInfluenceCount, uint32(controlPointsInfluences[controlPointIndex].size()));
        }
    }
}

void ProcessSkeletonsHierarchyRecursive(FbxNode* fbxNode)
{
    int32 attrCount = fbxNode->GetNodeAttributeCount();
    for (int32 a = 0; a < attrCount; ++a)
    {
        const FbxSkeleton* fbxSkeleton = fbxNode->GetSkeleton();
        if (fbxSkeleton != nullptr && fbxSkeleton->IsSkeletonRoot())
        {
            FBXSkeletonImportDetails::ProcessSkeletonHierarchy(fbxSkeleton);
            return;
        }
    }

    int32 childCount = fbxNode->GetChildCount();
    for (int32 c = 0; c < childCount; ++c)
        ProcessSkeletonsHierarchyRecursive(fbxNode->GetChild(c));
}

void ProcessSkeletonHierarchy(const FbxSkeleton* fbxSkeleton)
{
    DVASSERT(linkedSkeletons.count(fbxSkeleton) == 0);

    Vector<FBXJoint> fbxJoints; //[node, hierarchy-depth]
    CollectSkeletonNodes(fbxSkeleton, &fbxJoints, nullptr, 0);

    //sort nodes by hierarchy depth, and by uid inside hierarchy-level
    std::sort(fbxJoints.begin(), fbxJoints.end(), [](const FBXJoint& l, const FBXJoint& r)
              {
                  if (l.hierarchyDepth == r.hierarchyDepth)
                      return (l.joint->GetUniqueID() < r.joint->GetUniqueID());
                  else
                      return (l.hierarchyDepth < r.hierarchyDepth);
              });

    for (FBXJoint& fbxJoint : fbxJoints)
    {
        auto found = std::find_if(fbxJoints.begin(), fbxJoints.end(), [&fbxJoint](const FBXJoint& item) {
            return (item.joint == fbxJoint.parentJoint);
        });

        fbxJoint.parentIndex = (found == fbxJoints.end()) ? SkeletonComponent::INVALID_JOINT_INDEX : uint32(std::distance(fbxJoints.begin(), found));
    }

    linkedSkeletons.emplace(fbxSkeleton, std::move(fbxJoints));
}

void BuildSkeletonComponents()
{
    using namespace FBXImporterDetails;

    Map<const FbxSkeleton*, Vector<SkeletonComponent::Joint>> skeletonJointsMap;

    for (auto& linkedSkeleton : linkedSkeletons)
    {
        const FbxSkeleton* fbxSkeleton = linkedSkeleton.first;
        const Vector<FBXJoint>& fbxJoints = linkedSkeleton.second;

        Vector<SkeletonComponent::Joint>& joints = skeletonJointsMap[fbxSkeleton];

        size_t jointCount = fbxJoints.size();
        joints.reserve(jointCount);
        for (const FBXJoint& fbxJoint : fbxJoints)
        {
            FbxNode* jointNode = fbxJoint.joint->GetNode();

            joints.emplace_back();
            SkeletonComponent::Joint& joint = joints.back();
            joint.parentIndex = fbxJoint.parentIndex;
            joint.uid = GenerateNodeUID(jointNode);
            joint.name = FastName(jointNode->GetName());
            joint.bbox = AABBox3();
            joint.bindTransform = EvaluateNodeTransform(jointNode);
            joint.bindTransformInv = ToMatrix4(fbxJoint.bindTransformInv);
        }
    }

    for (auto& processedSkin : processedSkins)
    {
        FbxSkin* fbxSkin = processedSkin.first;
        const FbxSkeleton* fbxSkeleton = FindSkeletonRoot(fbxSkin);
        const FbxMesh* fbxMesh = static_cast<const FbxMesh*>(fbxSkin->GetGeometry());
        Matrix4 meshTransform = ToMatrix4(fbxMesh->GetNode()->EvaluateGlobalTransform());

        DVASSERT(fbxSkeleton != nullptr);
        DVASSERT(fbxSkeleton->IsSkeletonRoot());
        DVASSERT(linkedSkeletons.count(fbxSkeleton) != 0);
        DVASSERT(skeletonJointsMap.count(fbxSkeleton) != 0);

        Vector<SkeletonComponent::Joint>& skeletonJoints = skeletonJointsMap[fbxSkeleton];
        const Vector<FbxControlPointInfluences>& controlPointsInfluences = processedSkin.second.first;
        for (int32 controlPointIndex = 0; controlPointIndex < int32(controlPointsInfluences.size()); ++controlPointIndex)
        {
            for (const VertexInfluence& influence : controlPointsInfluences[controlPointIndex])
            {
                SkeletonComponent::Joint& joint = skeletonJoints[influence.first];
                float32 jointWeight = influence.second;

                if (jointWeight > EPSILON)
                {
                    Vector3 vertexPosition = ToVector3(fbxMesh->GetControlPointAt(controlPointIndex)) * meshTransform;
                    joint.bbox.AddPoint(vertexPosition * joint.bindTransformInv);
                }
            }
        }
    }

    for (auto& skeletonJoints : skeletonJointsMap)
    {
        Vector<SkeletonComponent::Joint>& joints = skeletonJoints.second;
        for (SkeletonComponent::Joint& joint : joints)
        {
            if (joint.bbox.IsEmpty())
                joint.bbox = AABBox3(Vector3(), 0.f);
        }

        SkeletonComponent* skeletonComponent = new SkeletonComponent();
        skeletonComponent->SetJoints(skeletonJoints.second);
        builtSkeletonComponents[skeletonJoints.first] = skeletonComponent;
    }
}

void CollectSkeletonNodes(const FbxSkeleton* fbxSkeleton, Vector<FBXJoint>* fbxJoints, const FbxSkeleton* fbxParentSkeleton, uint32 depth)
{
    if (fbxSkeleton != nullptr && fbxSkeleton->GetSkeletonType())
    {
        fbxJoints->emplace_back();
        fbxJoints->back().joint = fbxSkeleton;
        fbxJoints->back().parentJoint = fbxParentSkeleton;
        fbxJoints->back().hierarchyDepth = depth;

        FbxNode* fbxNode = fbxSkeleton->GetNode();
        int32 childCount = fbxNode->GetChildCount();
        for (int32 c = 0; c < childCount; ++c)
        {
            const FbxSkeleton* childJoint = fbxNode->GetChild(c)->GetSkeleton();
            if (childJoint != nullptr)
            {
                CollectSkeletonNodes(childJoint, fbxJoints, fbxSkeleton, depth + 1);
            }
        }
    }
}

const FbxSkeleton* FindSkeletonRoot(FbxSkin* fbxSkin)
{
    const FbxSkeleton* skeletonRoot = nullptr;

    int32 clusterCount = fbxSkin->GetClusterCount();
    for (int32 c = 0; c < clusterCount; ++c)
    {
        FbxCluster* cluster = fbxSkin->GetCluster(c);
        const FbxSkeleton* linkedJoint = cluster->GetLink()->GetSkeleton();

        skeletonRoot = FindSkeletonRoot(linkedJoint);
        if (skeletonRoot != nullptr)
            break;
    }

    return skeletonRoot;
}

const FbxSkeleton* FindSkeletonRoot(const FbxSkeleton* fbxSkeleton)
{
    if (fbxSkeleton == nullptr)
        return nullptr;

    while (fbxSkeleton != nullptr && !fbxSkeleton->IsSkeletonRoot())
    {
        fbxSkeleton = fbxSkeleton->GetNode()->GetParent()->GetSkeleton();
    }

    return fbxSkeleton;
}

}; //ns FBXSkeletonImportDetails

}; //ns DAVA