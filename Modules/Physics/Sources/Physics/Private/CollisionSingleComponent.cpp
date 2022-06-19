#include "Physics/CollisionSingleComponent.h"

#include <algorithm>

namespace DAVA
{
void CollisionSingleComponent::RemoveCollisionsWithEntity(Entity* entity)
{
    collisions.erase(std::remove_if(collisions.begin(),
                                    collisions.end(),
                                    [entity](const CollisionInfo& i) { return i.first == entity || i.second == entity; }),
                     collisions.end());
}
}