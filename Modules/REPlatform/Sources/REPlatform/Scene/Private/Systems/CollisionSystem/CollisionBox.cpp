#include "REPlatform/Scene/Private/Systems/CollisionSystem/CollisionBox.h"

namespace DAVA
{
CollisionBox::CollisionBox(const Any& object, btCollisionWorld* word, const Vector3& position, float32 boxSize)
    : CollisionBaseObject(object, word)
{
    Initialize(object, word, position, Vector3(boxSize, boxSize, boxSize));
}

CollisionBox::CollisionBox(const Any& object, btCollisionWorld* word, const Vector3& position, const Vector3& boxSize)
    : CollisionBaseObject(object, word)
{
    Initialize(object, word, position, boxSize);
}

CollisionBox::~CollisionBox()
{
    if (btObject != nullptr)
    {
        btWord->removeCollisionObject(btObject);
        SafeDelete(btObject);
        SafeDelete(btShape);
    }
}

void CollisionBox::Initialize(const Any& object_, btCollisionWorld* world_, const Vector3& position_, const Vector3& boxSize_)
{
    if (world_ != nullptr)
    {
        Vector3 halfSize = 0.5f * boxSize_;

        btTransform trans;
        trans.setIdentity();
        trans.setOrigin(btVector3(position_.x, position_.y, position_.z));

        btShape = new btBoxShape(btVector3(halfSize.x, halfSize.y, halfSize.z));
        btObject = new btCollisionObject();
        btObject->setCollisionShape(btShape);
        btObject->setWorldTransform(trans);
        btWord->addCollisionObject(btObject);

        object.SetBoundingBox(AABBox3(-halfSize, halfSize));
    }
}

CollisionBaseObject::ClassifyPlaneResult CollisionBox::ClassifyToPlane(const Plane& plane)
{
    return ClassifyBoundingBoxToPlane(object.GetBoundingBox(), TransformPlaneToLocalSpace(plane));
}

CollisionBaseObject::ClassifyPlanesResult CollisionBox::ClassifyToPlanes(const Vector<Plane>& planes)
{
    for (const Plane& plane : planes)
    {
        if (ClassifyToPlane(plane) == CollisionBaseObject::ClassifyPlaneResult::Behind)
        {
            return CollisionBaseObject::ClassifyPlanesResult::Outside;
        }
    }
    return CollisionBaseObject::ClassifyPlanesResult::ContainsOrIntersects;
}
} // namespace DAVA
