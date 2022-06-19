#pragma once

#include <Base/Vector.h>

namespace DAVA
{
class Entity;
class CollisionShapeComponent;
class CharacterControllerComponent;
namespace PhysicsUtils
{
/** Get vector of collision components attached to the entity */
Vector<CollisionShapeComponent*> GetShapeComponents(Entity* entity);

/** Get character controller component attached to the entity. Return nullptr if there is none */
CharacterControllerComponent* GetCharacterControllerComponent(Entity* entity);
}
}