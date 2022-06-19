#include "PhysicsDebug/PhysicsDebugDrawSystem.h"

#include <Physics/CollisionShapeComponent.h>
#include <Physics/Private/PhysicsMath.h>
#include <Physics/PhysicsModule.h>

#include <Entity/Component.h>
#include <Scene3D/Entity.h>
#include <Scene3D/Scene.h>
#include <Scene3D/Components/TransformComponent.h>
#include <Scene3D/Components/SingleComponents/TransformSingleComponent.h>
#include <Engine/Engine.h>
#include <Engine/EngineContext.h>
#include <Render/Highlevel/RenderSystem.h>
#include <Render/Highlevel/RenderObject.h>
#include <Render/Material/NMaterial.h>
#include <Render/3D/PolygonGroup.h>
#include <Math/Color.h>

#include <physx/PxRigidBody.h>
#include <physx/PxShape.h>
#include <physx/geometry/PxBoxGeometry.h>
#include <physx/geometry/PxCapsuleGeometry.h>
#include <physx/geometry/PxSphereGeometry.h>
#include <physx/geometry/PxTriangleMeshGeometry.h>
#include <physx/geometry/PxTriangleMesh.h>
#include <physx/geometry/PxConvexMeshGeometry.h>
#include <physx/geometry/PxConvexMesh.h>
#include <physx/geometry/PxHeightFieldGeometry.h>
#include <physx/geometry/PxHeightField.h>
#include <PxShared/foundation/PxVec3.h>

namespace DAVA
{
namespace PhysicsDebugDrawSystemDetail
{
const uint16 VERTEX_PER_CIRCLE = 32;

bool IsGeometryEqual(const physx::PxGeometryHolder& holder1, const physx::PxGeometryHolder& holder2)
{
    if (holder1.getType() != holder2.getType())
    {
        return false;
    }

    bool result = false;
    switch (holder1.getType())
    {
    case physx::PxGeometryType::eBOX:
    {
        physx::PxBoxGeometry box1 = holder1.box();
        physx::PxBoxGeometry box2 = holder2.box();

        if (box1.halfExtents == box2.halfExtents)
        {
            result = true;
        }
    }
    break;
    case physx::PxGeometryType::eSPHERE:
    {
        physx::PxSphereGeometry sphere1 = holder1.sphere();
        physx::PxSphereGeometry sphere2 = holder2.sphere();

        if (sphere1.radius == sphere2.radius)
        {
            result = true;
        }
    }
    break;
    case physx::PxGeometryType::eCAPSULE:
    {
        physx::PxCapsuleGeometry capsule1 = holder1.capsule();
        physx::PxCapsuleGeometry capsule2 = holder2.capsule();

        if (capsule1.radius == capsule2.radius &&
            capsule1.halfHeight == capsule2.halfHeight)
        {
            result = true;
        }
    }
    break;
    case physx::PxGeometryType::eCONVEXMESH:
    {
        physx::PxConvexMeshGeometry mesh1 = holder1.convexMesh();
        physx::PxConvexMeshGeometry mesh2 = holder2.convexMesh();

        if (mesh1.convexMesh == mesh2.convexMesh)
        {
            result = true;
        }
    }
    break;
    case physx::PxGeometryType::eTRIANGLEMESH:
    {
        physx::PxTriangleMeshGeometry mesh1 = holder1.triangleMesh();
        physx::PxTriangleMeshGeometry mesh2 = holder2.triangleMesh();

        if (mesh1.triangleMesh == mesh2.triangleMesh)
        {
            result = true;
        }
    }
    break;
    case physx::PxGeometryType::eHEIGHTFIELD:
    {
        physx::PxHeightFieldGeometry heightfield1 = holder1.heightField();
        physx::PxHeightFieldGeometry heightfield2 = holder2.heightField();

        if (heightfield1.heightField == heightfield2.heightField &&
            heightfield1.columnScale == heightfield2.columnScale &&
            heightfield1.rowScale == heightfield2.rowScale &&
            heightfield1.heightScale == heightfield2.heightScale)
        {
            result = true;
        }
    }
    break;
    case physx::PxGeometryType::ePLANE:
        result = true;
        break;
    default:
        DVASSERT(false);
        break;
    }

    return result;
}

Color GetColor(physx::PxShape* shape)
{
    physx::PxActor* actor = shape->getActor();
    Color result = Color::White;
    if (actor != nullptr)
    {
        if (actor->is<physx::PxRigidStatic>())
        {
            result = Color::Red;
        }
        else
        {
            physx::PxRigidDynamic* dynamic = actor->is<physx::PxRigidDynamic>();
            if (dynamic != nullptr)
            {
                if (dynamic->isSleeping())
                {
                    result = Color::Yellow;
                }
                else
                {
                    result = Color::Green;
                }
            }
        }

        if (shape->getGeometryType() == physx::PxGeometryType::eHEIGHTFIELD)
        {
            result = Color(0.0f, 0.0f, 0.5f, 1.0f);
        }
    }

    return result;
}

bool IsCollisionComponent(Component* component)
{
    const Type* componentType = component->GetType();
    for (const Type* type : GetEngineContext()->moduleManager->GetModule<PhysicsModule>()->GetShapeComponentTypes())
    {
        if (type == componentType)
        {
            return true;
        }
    }

    return false;
}

void GenerateArc(float32 startRad, float32 endRad, float32 radius, const Vector3& center,
                 const Vector3& direction, bool closeArc, const Matrix4& localPose,
                 Vector<Vector3>& vertices, Vector<uint16>& indices)
{
    float32 deltaRad = endRad - startRad;
    uint16 pointsCount = std::min(static_cast<uint16>(128), static_cast<uint16>(VERTEX_PER_CIRCLE * deltaRad * radius / PI_2));

    const Vector3 dir = Normalize(direction);
    const Vector3 ortho = Abs(dir.x) < Abs(dir.y) ? dir.CrossProduct(Vector3(1.f, 0.f, 0.f)) : dir.CrossProduct(Vector3(0.f, 1.f, 0.f));

    uint16 startVertex = static_cast<uint16>(vertices.size());

    Matrix4 rotationMx;
    float32 angleDelta = deltaRad / pointsCount;
    float32 currentAngle = startRad;
    while (currentAngle < endRad + 0.01)
    {
        rotationMx.BuildRotation(direction, -currentAngle);
        uint16 vertexIndex = static_cast<uint16>(vertices.size());
        vertices.push_back(center + ((ortho * radius) * rotationMx) * localPose);

        if (currentAngle > startRad)
        {
            indices.push_back(vertexIndex - 1);
            indices.push_back(vertexIndex);
        }
        currentAngle += angleDelta;
    }

    if (closeArc == true)
    {
        uint16 endVertexIndex = static_cast<uint16>(vertices.size() - 1);
        indices.push_back(endVertexIndex);
        indices.push_back(startVertex);
    }
}

template <typename TVertices, typename TIndices>
RefPtr<PolygonGroup> AllocatePolygonGroup(const TVertices& vertices, const TIndices& indices, rhi::PrimitiveType primitiveType, int32 primitiveCount)
{
    RefPtr<PolygonGroup> polygonGroup(new PolygonGroup());
    polygonGroup->SetPrimitiveType(primitiveType);
    polygonGroup->AllocateData(eVertexFormat::EVF_VERTEX, static_cast<int32>(vertices.size()), static_cast<int32>(indices.size()), primitiveCount);
    memcpy(polygonGroup->vertexArray, vertices.data(), vertices.size() * sizeof(Vector3));
    memcpy(polygonGroup->indexArray, indices.data(), indices.size() * sizeof(uint16));
    polygonGroup->BuildBuffers();
    polygonGroup->RecalcAABBox();

    return polygonGroup;
}

RefPtr<PolygonGroup> CreateBoxPolygonGroup(const physx::PxBoxGeometry& geom, const Matrix4& localPose)
{
    Vector3 halfExtents = PhysicsMath::PxVec3ToVector3(geom.halfExtents);
    Array<Vector3, 8> vertices =
    {
      Vector3(-halfExtents.x, -halfExtents.y, -halfExtents.z) * localPose,
      Vector3(halfExtents.x, -halfExtents.y, -halfExtents.z) * localPose,
      Vector3(-halfExtents.x, halfExtents.y, -halfExtents.z) * localPose,
      Vector3(halfExtents.x, halfExtents.y, -halfExtents.z) * localPose,
      Vector3(-halfExtents.x, -halfExtents.y, halfExtents.z) * localPose,
      Vector3(halfExtents.x, -halfExtents.y, halfExtents.z) * localPose,
      Vector3(-halfExtents.x, halfExtents.y, halfExtents.z) * localPose,
      Vector3(halfExtents.x, halfExtents.y, halfExtents.z) * localPose,
    };

    Array<uint16, 24> indices =
    {
      0, 1,
      1, 3,
      3, 2,
      2, 0,
      4, 5,
      5, 7,
      7, 6,
      6, 4,
      0, 4,
      2, 6,
      3, 7,
      1, 5
    };

    return AllocatePolygonGroup(vertices, indices, rhi::PRIMITIVE_LINELIST, 12);
}

RefPtr<PolygonGroup> CreateSpherePolygonGroup(const physx::PxSphereGeometry& geom, const Matrix4& localPose)
{
    uint16 pointsCount = std::min(128u, static_cast<uint32>(geom.radius * VERTEX_PER_CIRCLE));
    Vector<Vector3> vertices;
    Vector<uint16> indices;
    vertices.reserve(pointsCount * 3); // 3 circles per sphere
    indices.reserve(pointsCount * 2 * 3); // 3 circles per sphere, 2 indices per single line

    Array<Vector3, 3> directions = {
        Vector3(1.0f, 0.0f, 0.0f),
        Vector3(0.0f, 1.0f, 0.0f),
        Vector3(0.0f, 0.0f, 1.0f)
    };

    for (size_t dirIndex = 0; dirIndex < directions.size(); ++dirIndex)
    {
        GenerateArc(0.0, PI_2, geom.radius, Vector3(0.0f, 0.0f, 0.0f), directions[dirIndex], true, localPose, vertices, indices);
    }

    return AllocatePolygonGroup(vertices, indices, rhi::PRIMITIVE_LINELIST, static_cast<int32>(indices.size() >> 1));
}

RefPtr<PolygonGroup> CreateCapsulePolygonGroup(const physx::PxCapsuleGeometry& geom, const Matrix4& localPose)
{
    uint16 pointsCount = std::min(128u, static_cast<uint32>(geom.radius * 32));
    Vector<Vector3> vertices;
    Vector<uint16> indices;
    vertices.reserve(8 + 2 * pointsCount);
    indices.reserve(8 + 2 * 2 * pointsCount);

    Vector3 xAxisOffset = Vector3(1.0f, 0.0f, 0.0f) * geom.halfHeight;
    Vector3 yAxisOffset = Vector3(0.0f, geom.radius, 0.0f);
    Vector3 zAxisOffset = Vector3(0.0f, 0.0f, geom.radius);

    GenerateArc(0.0f, PI_2, geom.radius, xAxisOffset, Vector3(1.0f, 0.0f, 0.0f), true, localPose, vertices, indices);
    GenerateArc(0.0f, PI, geom.radius, xAxisOffset, Vector3(0.0f, 1.0f, 0.0f), false, localPose, vertices, indices);
    GenerateArc(PI_05, PI + PI_05, geom.radius, xAxisOffset, Vector3(0.0f, 0.0f, 1.0f), false, localPose, vertices, indices);

    GenerateArc(0.0f, PI_2, geom.radius, -xAxisOffset, Vector3(1.0f, 0.0f, 0.0f), false, localPose, vertices, indices);
    GenerateArc(PI, PI_2, geom.radius, -xAxisOffset, Vector3(0.0f, 1.0f, 0.0f), false, localPose, vertices, indices);
    GenerateArc(PI + PI_05, PI_2 + PI_05, geom.radius, -xAxisOffset, Vector3(0.0f, 0.0f, 1.0f), false, localPose, vertices, indices);

    uint16 firstLineVertex = static_cast<uint16>(vertices.size());

    vertices.push_back(-xAxisOffset + yAxisOffset);
    vertices.push_back(xAxisOffset + yAxisOffset);
    vertices.push_back(-xAxisOffset - yAxisOffset);
    vertices.push_back(xAxisOffset - yAxisOffset);
    vertices.push_back(-xAxisOffset + zAxisOffset);
    vertices.push_back(xAxisOffset + zAxisOffset);
    vertices.push_back(-xAxisOffset - zAxisOffset);
    vertices.push_back(xAxisOffset - zAxisOffset);

    for (uint16 i = 0; i < 8; ++i)
    {
        indices.push_back(firstLineVertex + i);
    }

    return AllocatePolygonGroup(vertices, indices, rhi::PRIMITIVE_LINELIST, static_cast<int32>(indices.size() >> 1));
}

RefPtr<PolygonGroup> CreateConvexMeshPolygonGroup(const physx::PxConvexMeshGeometry& geom, const Matrix4& localPose)
{
    physx::PxConvexMesh* mesh = geom.convexMesh;
    physx::PxU32 polygonCount = mesh->getNbPolygons();
    const physx::PxU8* indicesPtr = mesh->getIndexBuffer();
    const physx::PxVec3* verticesPtr = mesh->getVertices();

    Vector<Vector3> vertices;
    vertices.reserve(mesh->getNbVertices());
    Vector<uint16> indices;
    indices.reserve(vertices.capacity() * 2);

    for (physx::PxU32 i = 0; i < polygonCount; ++i)
    {
        physx::PxHullPolygon data;
        mesh->getPolygonData(i, data);

        physx::PxU16 vertexCount = data.mNbVerts;
        for (physx::PxU16 j = 1; j < vertexCount; j++)
        {
            indices.push_back(static_cast<uint16>(vertices.size()));
            vertices.push_back(PhysicsMath::PxVec3ToVector3(verticesPtr[indicesPtr[j - 1]]));
            indices.push_back(static_cast<uint16>(vertices.size()));
            vertices.push_back(PhysicsMath::PxVec3ToVector3(verticesPtr[indicesPtr[j]]));
        }

        {
            indices.push_back(static_cast<uint16>(vertices.size()));
            vertices.push_back(PhysicsMath::PxVec3ToVector3(verticesPtr[indicesPtr[0]]));
            indices.push_back(static_cast<uint16>(vertices.size()));
            vertices.push_back(PhysicsMath::PxVec3ToVector3(verticesPtr[indicesPtr[vertexCount - 1]]));
        }

        indicesPtr += vertexCount;
    }

    return AllocatePolygonGroup(vertices, indices, rhi::PRIMITIVE_LINELIST, static_cast<int32>(indices.size() >> 1));
}

RefPtr<PolygonGroup> CreateTriangleMeshPolygonGroup(const physx::PxTriangleMeshGeometry& geom, const Matrix4& localPose)
{
    physx::PxTriangleMesh* mesh = geom.triangleMesh;

    const physx::PxU32 triangleCount = mesh->getNbTriangles();
    const physx::PxVec3* verticesPtr = mesh->getVertices();
    const void* indicesPtr = mesh->getTriangles();

    const bool indices16Bits = mesh->getTriangleMeshFlags() & physx::PxTriangleMeshFlag::e16_BIT_INDICES;

    Vector<Vector3> vertices;
    vertices.reserve(triangleCount * 3);
    Vector<uint16> indices;
    indices.reserve(vertices.capacity());

    for (physx::PxU32 i = 0; i < triangleCount; ++i)
    {
        physx::PxU32 i0 = indices16Bits ? static_cast<const physx::PxU16*>(indicesPtr)[3 * i] : static_cast<const physx::PxU32*>(indicesPtr)[3 * i];
        physx::PxU32 i1 = indices16Bits ? static_cast<const physx::PxU16*>(indicesPtr)[3 * i + 1] : static_cast<const physx::PxU32*>(indicesPtr)[3 * i + 1];
        physx::PxU32 i2 = indices16Bits ? static_cast<const physx::PxU16*>(indicesPtr)[3 * i + 2] : static_cast<const physx::PxU32*>(indicesPtr)[3 * i + 2];

        Vector3 v0 = PhysicsMath::PxVec3ToVector3(verticesPtr[i0]);
        Vector3 v1 = PhysicsMath::PxVec3ToVector3(verticesPtr[i1]);
        Vector3 v2 = PhysicsMath::PxVec3ToVector3(verticesPtr[i2]);

        indices.push_back(static_cast<uint16>(vertices.size()));
        vertices.push_back(v0);
        indices.push_back(static_cast<uint16>(vertices.size()));
        vertices.push_back(v1);
        indices.push_back(static_cast<uint16>(vertices.size()));
        vertices.push_back(v2);
    }

    return AllocatePolygonGroup(vertices, indices, rhi::PRIMITIVE_TRIANGLELIST, triangleCount);
}

Vector<RefPtr<PolygonGroup>> CreateHeightfieldPolygonGroup(const physx::PxHeightFieldGeometry& geom, const Matrix4& localPose)
{
    Vector<RefPtr<PolygonGroup>> result;

    physx::PxHeightField* heightfield = geom.heightField;
    physx::PxU32 columnCount = heightfield->getNbColumns();
    physx::PxU32 rowCount = heightfield->getNbRows();

    uint32 squadCount = 4 * (columnCount - 1) * (rowCount - 1);

    Vector<Vector3> vertices;
    vertices.reserve(squadCount);
    Vector<uint16> indices;
    indices.reserve(6 * vertices.capacity() / 4);

    float32 xOffset = (rowCount >> 1) * geom.rowScale;
    float32 yOffset = (columnCount >> 1) * geom.columnScale;

    for (physx::PxU32 i = 0; i < rowCount - 1; ++i)
    {
        for (physx::PxU32 j = 0; j < columnCount - 1; ++j)
        {
            uint32 currentIndex = static_cast<uint32>(vertices.size());
            if (currentIndex + 6 > std::numeric_limits<uint16>::max())
            {
                result.push_back(AllocatePolygonGroup(vertices, indices, rhi::PRIMITIVE_TRIANGLELIST, static_cast<int32>(indices.size() / 3)));

                vertices.clear();
                indices.clear();
                currentIndex = 0;
            }

            uint16 startIndex = static_cast<uint16>(currentIndex);
            float32 rowIndex = static_cast<float32>(i);
            float32 columnIndex = static_cast<float32>(j);

            float32 height0 = heightfield->getHeight(rowIndex, columnIndex);
            float32 height1 = heightfield->getHeight(rowIndex + 1, columnIndex);
            float32 height2 = heightfield->getHeight(rowIndex, columnIndex + 1);
            float32 height3 = heightfield->getHeight(rowIndex + 1, columnIndex + 1);

            float32 xPosition = columnIndex * geom.rowScale - xOffset;
            float32 yPosition = rowIndex * geom.columnScale - yOffset;
            vertices.push_back(Vector3(xPosition, yPosition, height0 * geom.heightScale));
            vertices.push_back(Vector3(xPosition + geom.columnScale, yPosition, height2 * geom.heightScale));
            vertices.push_back(Vector3(xPosition, yPosition + geom.rowScale, height1 * geom.heightScale));
            vertices.push_back(Vector3(xPosition + geom.columnScale, yPosition + geom.rowScale, height3 * geom.heightScale));

            indices.push_back(startIndex);
            indices.push_back(startIndex + 1);
            indices.push_back(startIndex + 2);

            indices.push_back(startIndex + 1);
            indices.push_back(startIndex + 2);
            indices.push_back(startIndex + 3);
        }
    }

    result.push_back(AllocatePolygonGroup(vertices, indices, rhi::PRIMITIVE_TRIANGLELIST, static_cast<int32>(indices.size() / 3)));
    return result;
}

Vector<RefPtr<PolygonGroup>> CreatePolygonGroup(const physx::PxGeometryHolder& geom, const Matrix4& localPose)
{
    Vector<RefPtr<PolygonGroup>> result;
    switch (geom.getType())
    {
    case physx::PxGeometryType::eBOX:
        result.push_back(CreateBoxPolygonGroup(geom.box(), localPose));
        break;
    case physx::PxGeometryType::eSPHERE:
        result.push_back(CreateSpherePolygonGroup(geom.sphere(), localPose));
        break;
    case physx::PxGeometryType::eCAPSULE:
        result.push_back(CreateCapsulePolygonGroup(geom.capsule(), localPose));
        break;
    case physx::PxGeometryType::eCONVEXMESH:
        result.push_back(CreateConvexMeshPolygonGroup(geom.convexMesh(), localPose));
        break;
    case physx::PxGeometryType::eTRIANGLEMESH:
        result.push_back(CreateTriangleMeshPolygonGroup(geom.triangleMesh(), localPose));
        break;
    case physx::PxGeometryType::eHEIGHTFIELD:
        result = CreateHeightfieldPolygonGroup(geom.heightField(), localPose);
        break;
    default:
        break;
    }

    return result;
}

RefPtr<NMaterial> CreateMaterial()
{
    RefPtr<NMaterial> material(new NMaterial());
    material->SetMaterialName(FastName("DebugPhysxMaterial"));
    material->SetFXName(NMaterialName::DEBUG_DRAW_WIREFRAME);
    material->AddProperty(FastName("color"), Color::White.color, rhi::ShaderProp::TYPE_FLOAT4);

    return material;
}

RenderObject* CreateRenderObject(const physx::PxGeometryHolder& geometry, uint32 vertexLayoutId, const Matrix4& localPose)
{
    Vector<RefPtr<PolygonGroup>> groups = CreatePolygonGroup(geometry, localPose);
    if (groups.empty())
    {
        return nullptr;
    }

    RenderObject* ro = new RenderObject();
    for (RefPtr<PolygonGroup>& group : groups)
    {
        ScopedPtr<RenderBatch> batch(new RenderBatch());
        batch->SetMaterial(CreateMaterial().Get());
        batch->SetPolygonGroup(group.Get());
        batch->vertexLayoutId = vertexLayoutId;

        ro->AddRenderBatch(batch);
    }

    return ro;
}

void MarkRenderObjectsForUpdateRecursively(Scene* scene, Entity* root, const UnorderedMap<Entity*, RenderObject*>& entityToRoMap)
{
    auto roIter = entityToRoMap.find(root);
    if (roIter != entityToRoMap.end() && roIter->second != nullptr)
    {
        Matrix4* worldTransformPointer = root->GetComponent<TransformComponent>()->GetWorldMatrixPtr();
        roIter->second->SetWorldMatrixPtr(worldTransformPointer);
        scene->renderSystem->MarkForUpdate(roIter->second);
    }

    const int32 childrenCount = root->GetChildrenCount();
    for (int32 i = 0; i < childrenCount; ++i)
    {
        Entity* child = root->GetChild(i);
        DVASSERT(child != nullptr);

        MarkRenderObjectsForUpdateRecursively(scene, child, entityToRoMap);
    }
}

} // namespace PhysicsDebugDrawSystemDetail

uint32 PhysicsDebugDrawSystem::vertexLayoutId = static_cast<uint32>(-1);

PhysicsDebugDrawSystem::PhysicsDebugDrawSystem(Scene* scene)
    : SceneSystem(scene)
{
    if (vertexLayoutId == static_cast<uint32>(-1))
    {
        rhi::VertexLayout vertexLayout;
        vertexLayout.AddElement(rhi::VS_POSITION, 0, rhi::VDT_FLOAT, 3);
        vertexLayoutId = rhi::VertexLayout::UniqueId(vertexLayout);
    }
}

void PhysicsDebugDrawSystem::RegisterEntity(Entity* entity)
{
    using namespace PhysicsDebugDrawSystemDetail;

    for (const Type* type : GetEngineContext()->moduleManager->GetModule<PhysicsModule>()->GetShapeComponentTypes())
    {
        for (uint32 i = 0; i < entity->GetComponentCount(type); ++i)
        {
            RegisterComponent(entity, entity->GetComponent(type, i));
        }
    }
}

void PhysicsDebugDrawSystem::UnregisterEntity(Entity* entity)
{
    using namespace PhysicsDebugDrawSystemDetail;

    for (const Type* type : GetEngineContext()->moduleManager->GetModule<PhysicsModule>()->GetShapeComponentTypes())
    {
        for (uint32 i = 0; i < entity->GetComponentCount(type); ++i)
        {
            UnregisterComponent(entity, entity->GetComponent(type, i));
        }
    }
}

void PhysicsDebugDrawSystem::RegisterComponent(Entity* entity, Component* component)
{
    using namespace PhysicsDebugDrawSystemDetail;

    if (IsCollisionComponent(component))
    {
        pendingComponents.insert(static_cast<CollisionShapeComponent*>(component));
    }
}

void PhysicsDebugDrawSystem::UnregisterComponent(Entity* entity, Component* component)
{
    using namespace PhysicsDebugDrawSystemDetail;

    if (IsCollisionComponent(component))
    {
        auto iter = renderObjects.find(static_cast<CollisionShapeComponent*>(component));
        if (iter != renderObjects.end())
        {
            RenderObjectInfo& roInfo = iter->second;
            if (roInfo.ro != nullptr)
            {
                GetScene()->GetRenderSystem()->RemoveFromRender(roInfo.ro);
                SafeRelease(roInfo.ro);
            }
            renderObjects.erase(iter);
        }

        pendingComponents.erase(static_cast<CollisionShapeComponent*>(component));
    }
}

void PhysicsDebugDrawSystem::PrepareForRemove()
{
    pendingComponents.clear();
    for (const auto& roItem : renderObjects)
    {
        RenderObject* ro = roItem.second.ro;
        if (ro != nullptr)
        {
            GetScene()->GetRenderSystem()->RemoveFromRender(ro);
            SafeRelease(ro);
        }
    }

    renderObjects.clear();
}

void PhysicsDebugDrawSystem::Process(float32 timeElapsed)
{
    using namespace PhysicsDebugDrawSystemDetail;
    auto iter = pendingComponents.begin();
    while (iter != pendingComponents.end())
    {
        RenderObject* ro = nullptr;
        physx::PxShape* shape = (*iter)->GetPxShape();
        if (shape != nullptr)
        {
            physx::PxGeometryHolder holder = shape->getGeometry();
            ro = CreateRenderObject(holder, vertexLayoutId, PhysicsMath::PxMat44ToMatrix4(physx::PxMat44(shape->getLocalPose())));
            RenderObjectInfo roInfo;
            roInfo.ro = ro;
            roInfo.geomHolder = holder;
            roInfo.localPose = shape->getLocalPose();
            renderObjects[*iter] = roInfo;

            if (ro != nullptr)
            {
                Entity* entity = (*iter)->GetEntity();
                Matrix4* worldTransformPointer = entity->GetComponent<TransformComponent>()->GetWorldMatrixPtr();
                ro->SetWorldMatrixPtr(worldTransformPointer);
                GetScene()->GetRenderSystem()->RenderPermanent(ro);
            }

            iter = pendingComponents.erase(iter);
        }
        else
        {
            ++iter;
        }
    }

    Cleanup();
    for (auto& node : renderObjects)
    {
        if (node.second.ro == nullptr)
        {
            continue;
        }

        CollisionShapeComponent* shapeComponent = node.first;
        physx::PxShape* shape = shapeComponent->GetPxShape();
        DVASSERT(shape != nullptr);
        Color currentColor = GetColor(shape);

        for (uint32 i = 0; i < node.second.ro->GetRenderBatchCount(); ++i)
        {
            NMaterial* material = node.second.ro->GetRenderBatch(i)->GetMaterial();
            material->SetPropertyValue(FastName("color"), currentColor.color);
        }
    }

    UnorderedMap<Entity*, RenderObject*> mapping;
    for (auto& node : renderObjects)
    {
        mapping.emplace(node.first->GetEntity(), node.second.ro);
    }

    TransformSingleComponent* trSingle = GetScene()->transformSingleComponent;
    if (trSingle != nullptr)
    {
        for (auto& pair : trSingle->worldTransformChanged.map)
        {
            for (Entity* e : pair.second)
            {
                MarkRenderObjectsForUpdateRecursively(GetScene(), e, mapping);
            }
        }

        for (DAVA::Entity* e : trSingle->localTransformChanged)
        {
            MarkRenderObjectsForUpdateRecursively(GetScene(), e, mapping);
        }
    }
}

void PhysicsDebugDrawSystem::Cleanup()
{
    Vector<CollisionShapeComponent*> reinitComponents;
    for (auto& node : renderObjects)
    {
        const physx::PxGeometryHolder& holder1 = node.second.geomHolder;
        physx::PxShape* shape = node.first->GetPxShape();
        if (shape == nullptr)
        {
            reinitComponents.push_back(node.first);
        }
        else
        {
            const physx::PxGeometryHolder& holder1 = node.second.geomHolder;
            physx::PxShape* shape = node.first->GetPxShape();
            physx::PxTransform localPose = shape->getLocalPose();
            const physx::PxGeometryHolder& holder2 = shape->getGeometry();
            using namespace PhysicsDebugDrawSystemDetail;
            if (IsGeometryEqual(holder1, holder2) == false ||
                !(node.second.localPose.p == localPose.p &&
                  node.second.localPose.q == localPose.q))
            {
                reinitComponents.push_back(node.first);
            }
        }
    }

    for (CollisionShapeComponent* component : reinitComponents)
    {
        UnregisterComponent(component->GetEntity(), component);
        RegisterComponent(component->GetEntity(), component);
    }
}

} // namespace DAVA
