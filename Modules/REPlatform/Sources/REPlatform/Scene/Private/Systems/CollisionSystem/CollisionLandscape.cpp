#include "REPlatform/Scene/Private/Systems/CollisionSystem/CollisionLandscape.h"

#include <Base/AlignedAllocator.h>
#include <Render/Highlevel/Heightmap.h>
#include <Render/Highlevel/Landscape.h>
#include <Scene3D/Entity.h>

namespace DAVA
{
CollisionLandscape::CollisionLandscape(Entity* entity, btCollisionWorld* word, Landscape* landscape)
    : CollisionBaseObject(entity, word)
{
    if ((landscape != nullptr) && (word != nullptr))
    {
        Heightmap* heightmap = landscape->GetHeightmap();
        if ((heightmap != nullptr) && (heightmap->Size() > 0))
        {
            Vector3 landSize;
            AABBox3 landBox = landscape->GetBoundingBox();
            landSize = landBox.max - landBox.min;

            float32 landWidth = landSize.x;
            float32 landScaleW = landWidth / heightmap->Size();
            float32 landHeight = landSize.z;
            float32 landScaleH = landHeight / 65535.f;

            btHMap.resize(heightmap->Size() * heightmap->Size());

            for (int32 y = 0; y < heightmap->Size(); ++y)
            {
                for (int32 x = 0; x < heightmap->Size(); ++x)
                {
                    int32 heightIndex = x + y * heightmap->Size();
                    btHMap[heightIndex] = heightmap->GetHeight(x, y) * landScaleH;
                }
            }

            btTransform landTransform;
            landTransform.setIdentity();
            landTransform.setOrigin(btVector3(0, 0, landHeight / 2.0f));

            btTerrain = CreateObjectAligned<btHeightfieldTerrainShape, 16>(heightmap->Size(), heightmap->Size(), &btHMap.front(), landScaleH, 0.0f, landHeight, 2, PHY_FLOAT, true);
            btTerrain->setLocalScaling(btVector3(landScaleW, landScaleW, 1.0f));
            btObject = new btCollisionObject();
            btObject->setWorldTransform(landTransform);
            btObject->setCollisionShape(btTerrain);
            btWord->addCollisionObject(btObject);

            object.SetBoundingBox(landBox);
        }
    }
}

CollisionLandscape::~CollisionLandscape()
{
    if (NULL != btObject)
    {
        btWord->removeCollisionObject(btObject);
        SafeDelete(btObject);
        DestroyObjectAligned(btTerrain);
    }
}

CollisionBaseObject::ClassifyPlaneResult CollisionLandscape::ClassifyToPlane(const Plane& plane)
{
    return ClassifyPlaneResult::Behind;
}

CollisionBaseObject::ClassifyPlanesResult CollisionLandscape::ClassifyToPlanes(const Vector<Plane>& planes)
{
    return ClassifyPlanesResult::Outside;
}
} // namespace DAVA
