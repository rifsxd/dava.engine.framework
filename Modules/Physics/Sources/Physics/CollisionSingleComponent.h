#pragma once

#include "Base/Vector.h"
#include "Math/Vector.h"

namespace DAVA
{
class Entity;

/** Specifies information about collision point */
struct CollisionPoint
{
    /** Collision point coordinates */
    Vector3 position;

    /** Impulse applied at the point */
    Vector3 impulse;
};

/** Specifies information about collision */
struct CollisionInfo
{
    /** First collision object */
    Entity* first = nullptr;

    /** Second collision object */
    Entity* second = nullptr;

    /** Vector of collision points */
    Vector<CollisionPoint> points;
};

/** Single component providing information about current collisions */
class CollisionSingleComponent
{
public:
    /** Vector of current collisions */
    Vector<CollisionInfo> collisions;

    /** Remove collision from current collisions vector with specified `entity` */
    void RemoveCollisionsWithEntity(Entity* entity);
};
}
