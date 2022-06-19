#include "FBXMeshImport.h"

#include "FBXMaterialImport.h"
#include "FBXSkeletonImport.h"

#include "Scene3D/Entity.h"
#include "Scene3D/Components/SkeletonComponent.h"
#include "Scene3D/Components/RenderComponent.h"

#include "Render/3D/MeshUtils.h"
#include "Render/3D/PolygonGroup.h"
#include "Render/Highlevel/Mesh.h"
#include "Render/Highlevel/SkinnedMesh.h"
#include "Render/Material/NMaterial.h"
#include "Render/Material/NMaterialNames.h"

#define FBX_IMPORT_CREATE_MATERIAL_INSTANCES 1
#define FBX_IMPORT_AUTO_SHADOW_VOLUME_CONVERT 1
#define FBX_IMPORT_AUTO_BUILD_TANGENT_SPACE 1

namespace DAVA
{
namespace FBXMeshImportDetails
{
//namespace declarations

using GeometrySet = Vector<std::pair<PolygonGroup*, NMaterial*>>;

struct FBXVertex
{
    struct JointWeightComparator
    {
        bool operator()(const FBXImporterDetails::VertexInfluence& l, const FBXImporterDetails::VertexInfluence& r) const
        {
            return l.second > r.second; //for sorting in descending order
        }
    };

    union
    {
        float32 data[20];
        struct
        {
            Vector3 position;
            Vector2 texCoord[4];
            Vector3 normal;
            Vector3 tangent;
            Vector3 binormal;
        };
    };

    FBXVertex();
    FBXVertex(const FBXVertex& other);

    bool operator<(const FBXVertex& other) const;

    Set<FBXImporterDetails::VertexInfluence, JointWeightComparator> joints;
};

#if FBX_IMPORT_CREATE_MATERIAL_INSTANCES
NMaterial* CreateMaterialInstance(NMaterial* parent)
{
    DVASSERT(parent != nullptr);

    NMaterial* materialInstance = new NMaterial();
    materialInstance->SetMaterialName(FastName(Format("Instance-%d", parent->GetRetainCount() - 1).c_str()));
    materialInstance->SetParent(parent);

    return materialInstance;
}
#endif

Map<const FbxMesh*, GeometrySet> meshCache; //in ProcessedMesh::GeometrySet materials isn't retained. It's owned by materialCache

#if FBX_IMPORT_AUTO_SHADOW_VOLUME_CONVERT
NMaterial* shadowVolumeMaterial = nullptr;
#endif

FbxSurfaceMaterial* GetPolygonMaterial(FbxMesh* fbxMesh, int32 polygonIndex);

}; //ns FBXMeshImportDetails

namespace FBXImporterDetails
{
void ImportMeshToEntity(FbxNode* fbxNode, Entity* entity)
{
    using namespace FBXMeshImportDetails;

    DVASSERT(fbxNode);
    FbxMesh* fbxMesh = fbxNode->GetMesh();

    auto found = meshCache.find(fbxMesh);
    if (found == meshCache.end())
    {
        bool hasNormal = fbxMesh->GetElementNormalCount() > 0;
        bool hasTangent = fbxMesh->GetElementTangentCount() > 0;
        bool hasBinormal = fbxMesh->GetElementBinormalCount() > 0;
        bool hasSkinning = fbxMesh->GetDeformerCount(FbxDeformer::eSkin) > 0;

        uint32 maxControlPointInfluence = 0;
        Vector<FbxControlPointInfluences> controlPointsInfluences;
        if (hasSkinning)
        {
            FbxSkin* fbxSkin = static_cast<FbxSkin*>(fbxMesh->GetDeformer(0, FbxDeformer::eSkin));
            GetControlPointInfluences(fbxSkin, &controlPointsInfluences, &maxControlPointInfluence);
        }
        maxControlPointInfluence = Min(maxControlPointInfluence, PolygonGroup::MAX_VERTEX_JOINTS_COUNT);

        FbxStringList uvNames;
        fbxMesh->GetUVSetNames(uvNames);
        int32 uvCount = Min(uvNames.GetCount(), 4);

        int32 meshFormat = EVF_VERTEX;
        if (uvCount > 0)
            meshFormat |= EVF_TEXCOORD0;
        if (uvCount > 1)
            meshFormat |= EVF_TEXCOORD1;
        if (uvCount > 2)
            meshFormat |= EVF_TEXCOORD2;
        if (uvCount > 3)
            meshFormat |= EVF_TEXCOORD3;
        if (hasNormal)
            meshFormat |= EVF_NORMAL;
        if (hasTangent)
            meshFormat |= EVF_TANGENT;
        if (hasBinormal)
            meshFormat |= EVF_BINORMAL;
        if (maxControlPointInfluence == 1)
            meshFormat |= EVF_HARD_JOINTINDEX;
        if (maxControlPointInfluence > 1)
            meshFormat |= EVF_JOINTINDEX | EVF_JOINTWEIGHT;

        using VerticesMap = std::pair<Map<FBXVertex, Vector<int32>>, int32>; //[[vertex, indices], polygonCount]
        using MaterialGeometryMap = Map<FbxSurfaceMaterial*, VerticesMap>;

        MaterialGeometryMap materialGeometry;
        int32 polygonCount = fbxMesh->GetPolygonCount();
        for (int32 p = 0; p < polygonCount; p++)
        {
            int32 polygonSize = fbxMesh->GetPolygonSize(p);
            DVASSERT(polygonSize == 3);

            FbxSurfaceMaterial* fbxMaterial = GetPolygonMaterial(fbxMesh, p);
            int32& materialPolygonIndex = materialGeometry[fbxMaterial].second;
            for (int32 v = 0; v < polygonSize; ++v)
            {
                FBXVertex vertex;

                int32 vIndex = fbxMesh->GetPolygonVertex(p, v);
                vertex.position = ToVector3(fbxMesh->GetControlPointAt(vIndex));

                if (hasNormal)
                    vertex.normal = ToVector3(GetFbxMeshLayerElementValue<FbxVector4>(fbxMesh->GetElementNormal(), vIndex, p, v));

                if (hasTangent)
                    vertex.tangent = ToVector3(GetFbxMeshLayerElementValue<FbxVector4>(fbxMesh->GetElementTangent(), vIndex, p, v));

                if (hasBinormal)
                    vertex.binormal = ToVector3(GetFbxMeshLayerElementValue<FbxVector4>(fbxMesh->GetElementBinormal(), vIndex, p, v));

                for (int32 t = 0; t < uvCount; ++t)
                {
                    vertex.texCoord[t] = ToVector2(GetFbxMeshLayerElementValue<FbxVector2>(fbxMesh->GetElementUV(uvNames[t]), vIndex, p, v));
                    vertex.texCoord[t].y = -vertex.texCoord[t].y;
                }

                if (hasSkinning)
                {
                    const FbxControlPointInfluences& vertexInfluences = controlPointsInfluences[vIndex];

                    float32 weightsSum = 0.f;
                    for (const VertexInfluence& vInf : vertexInfluences)
                        weightsSum += vInf.second;

                    for (const VertexInfluence& vInf : vertexInfluences)
                        vertex.joints.insert(VertexInfluence(vInf.first, vInf.second / weightsSum));
                }

                materialGeometry[fbxMaterial].first[vertex].push_back(materialPolygonIndex * 3 + v);
            }

            ++materialPolygonIndex;
        }

        GeometrySet geometrySet;
        Matrix4 meshTransform = ToMatrix4(fbxNode->EvaluateGlobalTransform());
        for (auto& gIt : materialGeometry)
        {
            FbxSurfaceMaterial* fbxMaterial = gIt.first;
            NMaterial* material = ImportMaterial(fbxMaterial, maxControlPointInfluence);

            const VerticesMap& vertices = gIt.second;
            int32 vxCount = int32(vertices.first.size());
            int32 indCount = int32(vertices.second * 3);

            PolygonGroup* polygonGroup = new PolygonGroup();
            polygonGroup->AllocateData(meshFormat, vxCount, indCount);

            int32 vertexIndex = 0;
            for (auto vIt = vertices.first.cbegin(); vIt != vertices.first.cend(); ++vIt)
            {
                const FBXVertex& fbxVertex = vIt->first;
                const Vector<int32>& indices = vIt->second;

                polygonGroup->SetCoord(vertexIndex, fbxVertex.position);

                for (int32 t = 0; t < uvCount; ++t)
                    polygonGroup->SetTexcoord(t, vertexIndex, fbxVertex.texCoord[t]);

                if (hasNormal)
                    polygonGroup->SetNormal(vertexIndex, fbxVertex.normal);

                if (hasTangent)
                    polygonGroup->SetTangent(vertexIndex, fbxVertex.tangent);

                if (hasBinormal)
                    polygonGroup->SetBinormal(vertexIndex, fbxVertex.binormal);

                if (hasSkinning)
                {
                    if (maxControlPointInfluence == 1) //hard-skinning
                    {
                        polygonGroup->SetHardJointIndex(vertexIndex, int32(fbxVertex.joints.cbegin()->first));
                    }
                    else
                    {
                        auto vInf = fbxVertex.joints.cbegin();
                        for (uint32 j = 0; j < PolygonGroup::MAX_VERTEX_JOINTS_COUNT; ++j)
                        {
                            if (vInf != fbxVertex.joints.end())
                            {
                                polygonGroup->SetJointIndex(vertexIndex, j, int32(vInf->first));
                                polygonGroup->SetJointWeight(vertexIndex, j, vInf->second);

                                ++vInf;
                            }
                            else
                            {
                                polygonGroup->SetJointIndex(vertexIndex, j, 0);
                                polygonGroup->SetJointWeight(vertexIndex, j, 0.f);
                            }
                        }
                    }
                }

                for (int32 index : indices)
                    polygonGroup->SetIndex(index, uint16(vertexIndex));

                ++vertexIndex;
            }

            polygonGroup->ApplyMatrix(meshTransform);

#if FBX_IMPORT_AUTO_BUILD_TANGENT_SPACE
            bool hasUV = uvCount != 0;
            bool completedTBN = hasNormal && hasTangent && hasBinormal;
            if (hasNormal && hasUV && !completedTBN)
            {
                MeshUtils::RebuildMeshTangentSpace(polygonGroup);
            }
#endif

#if FBX_IMPORT_AUTO_SHADOW_VOLUME_CONVERT
            if (strstr(fbxNode->GetName(), "_shadow"))
            {
                PolygonGroup* shadowPolygonGroup = MeshUtils::CreateShadowPolygonGroup(polygonGroup);
                SafeRelease(polygonGroup);
                polygonGroup = shadowPolygonGroup;

                if (shadowVolumeMaterial == nullptr)
                {
                    shadowVolumeMaterial = new NMaterial();
                    shadowVolumeMaterial->SetMaterialName(FastName("Shadow_Material"));
                    shadowVolumeMaterial->SetFXName(NMaterialName::SHADOW_VOLUME);
                }

                //Material is owned by materialCache. We don't need to release it
                material = shadowVolumeMaterial;
            }
#endif
            geometrySet.emplace_back(polygonGroup, material);
        }

        found = meshCache.emplace(fbxMesh, std::move(geometrySet)).first;
    }

    bool isSkinned = (found->first->GetDeformerCount(FbxDeformer::eSkin) > 0);
    if (isSkinned)
    {
        FbxSkin* fbxSkin = static_cast<FbxSkin*>(found->first->GetDeformer(0, FbxDeformer::eSkin));

        ScopedPtr<SkinnedMesh> mesh(new SkinnedMesh());

        uint32 maxVertexInfluenceCount = 0;
        GetControlPointInfluences(fbxSkin, nullptr, &maxVertexInfluenceCount);

        const GeometrySet& geometrySet = found->second;
        for (auto& geometry : geometrySet)
        {
            PolygonGroup* polygonGroup = geometry.first;
            NMaterial* material = geometry.second;

#if FBX_IMPORT_CREATE_MATERIAL_INSTANCES
            ScopedPtr<NMaterial> materialInstance(CreateMaterialInstance(material));
            material = materialInstance.get();
#endif

            auto splitedPolygons = MeshUtils::SplitSkinnedMeshGeometry(polygonGroup, SkinnedMesh::MAX_TARGET_JOINTS);
            for (auto& p : splitedPolygons)
            {
                PolygonGroup* pg = p.first;
                pg->RecalcAABBox();

                ScopedPtr<RenderBatch> renderBatch(new RenderBatch());
                renderBatch->SetPolygonGroup(pg);
                renderBatch->SetMaterial(material);

                mesh->AddRenderBatch(renderBatch);
                mesh->SetJointTargets(renderBatch, p.second);

                SafeRelease(pg);
            }
        }

        entity->AddComponent(new RenderComponent(mesh));
        entity->AddComponent(GetBuiltSkeletonComponent(fbxSkin)->Clone(entity));
    }
    else
    {
        ScopedPtr<Mesh> mesh(new Mesh());

        const GeometrySet& geometrySet = found->second;
        for (auto& geometry : geometrySet)
        {
            PolygonGroup* polygonGroup = geometry.first;
            NMaterial* material = geometry.second;

#if FBX_IMPORT_CREATE_MATERIAL_INSTANCES
            ScopedPtr<NMaterial> materialInstance(CreateMaterialInstance(material));
            material = materialInstance.get();
#endif

            mesh->AddPolygonGroup(polygonGroup, material);
        }

        entity->AddComponent(new RenderComponent(mesh));
    }
}

void ClearMeshCache()
{
    for (auto& it : FBXMeshImportDetails::meshCache)
    {
        for (auto& p : it.second)
            SafeRelease(p.first);
        //in geometry cache material isn't retained
    }
    FBXMeshImportDetails::meshCache.clear();
    
#if FBX_IMPORT_AUTO_SHADOW_VOLUME_CONVERT
    SafeRelease(FBXMeshImportDetails::shadowVolumeMaterial);
#endif
}

}; //ns FBXImporterDetails

//////////////////////////////////////////////////////////////////////////
//FBXMeshImportDetails namespace definitions

namespace FBXMeshImportDetails
{
FBXVertex::FBXVertex()
{
    Memset(data, 0, sizeof(data));
}

FBXVertex::FBXVertex(const FBXVertex& other)
    : joints(other.joints)
{
    Memcpy(data, other.data, sizeof(data));
}

bool FBXVertex::operator<(const FBXVertex& other) const
{
    for (int32 d = 0; d < 14; ++d)
    {
        if (!FLOAT_EQUAL(data[d], other.data[d]))
            return data[d] < other.data[d];
    }

    if (joints.size() != other.joints.size() || joints.size() == 0)
        return joints.size() < other.joints.size();

    auto i = joints.cbegin();
    auto j = other.joints.cbegin();
    while (i != joints.cend())
    {
        if (i->first != j->first)
            return i->first < j->first;

        if (!FLOAT_EQUAL(i->second, j->second))
            return i->second < j->second;

        ++i;
        ++j;
    }

    return false;
}

FbxSurfaceMaterial* GetPolygonMaterial(FbxMesh* fbxMesh, int32 polygonIndex)
{
    FbxSurfaceMaterial* fbxMaterial = nullptr;
    for (int32 me = 0; me < fbxMesh->GetElementMaterialCount(); me++)
    {
        fbxMaterial = fbxMesh->GetNode()->GetMaterial(fbxMesh->GetElementMaterial(me)->GetIndexArray().GetAt(polygonIndex));
        if (fbxMaterial != nullptr)
            break;
    }

    return fbxMaterial;
}

}; //ns FBXMeshImportDetails

}; //ns DAVA
