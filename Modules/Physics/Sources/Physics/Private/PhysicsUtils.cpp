#include "Physics/PhysicsUtils.h"
#include "Physics/PhysicsModule.h"
#include "Physics/CollisionShapeComponent.h"
#include "Physics/CharacterControllerComponent.h"

#include <Engine/Engine.h>
#include <Scene3D/Entity.h>
#include <ModuleManager/ModuleManager.h>

namespace DAVA
{
namespace PhysicsUtils
{
Vector<CollisionShapeComponent*> GetShapeComponents(Entity* entity)
{
    const PhysicsModule* module = GetEngineContext()->moduleManager->GetModule<PhysicsModule>();
    const Vector<const Type*>& shapeComponents = module->GetShapeComponentTypes();

    Vector<CollisionShapeComponent*> shapes;
    for (const Type* shapeType : shapeComponents)
    {
        const uint32 shapesCount = entity->GetComponentCount(shapeType);
        if (shapesCount > 0)
        {
            for (uint32 i = 0; i < shapesCount; ++i)
            {
                CollisionShapeComponent* component = static_cast<CollisionShapeComponent*>(entity->GetComponent(shapeType, i));
                DVASSERT(component != nullptr);

                shapes.push_back(component);
            }
        }
    }

    return shapes;
}

CharacterControllerComponent* GetCharacterControllerComponent(Entity* entity)
{
    DVASSERT(entity != nullptr);

    const PhysicsModule* module = GetEngineContext()->moduleManager->GetModule<PhysicsModule>();
    DVASSERT(module != nullptr);

    const Vector<const Type*>& characterControllerComponents = module->GetCharacterControllerComponentTypes();

    for (const Type* controllerType : characterControllerComponents)
    {
        CharacterControllerComponent* component = static_cast<CharacterControllerComponent*>(entity->GetComponent(controllerType));
        if (component != nullptr)
        {
            return component;
        }
    }

    return nullptr;
}
}
}