#include "REPlatform/Commands/EntityParentChangeCommand.h"

#include <Math/Transform.h>
#include <Reflection/ReflectionRegistrator.h>
#include <Scene3D/Components/TransformComponent.h>
#include <Scene3D/Entity.h>

namespace DAVA
{
namespace EntityParentChangeCommandDetails
{
Transform ConvertLocalTransform(Entity* entity, Entity* parent)
{
    TransformComponent* parentTransform = parent->GetComponent<TransformComponent>();
    TransformComponent* entityTransform = entity->GetComponent<TransformComponent>();

    Transform parentInverse = parentTransform->GetWorldTransform();
    parentInverse.Inverse();
    return entityTransform->GetWorldTransform() * parentInverse;
}
}

EntityParentChangeCommand::EntityParentChangeCommand(Entity* _entity, Entity* _newParent, bool saveEntityPosition, Entity* _newBefore /* = nullptr */)
    : RECommand("Move entity")
    , entity(_entity)
    , newParent(_newParent)
    , newBefore(_newBefore)
    , saveEntityPosition(saveEntityPosition)
{
    SafeRetain(entity);

    DVASSERT(entity != nullptr);
    DVASSERT(newParent != nullptr);

    oldParent = entity->GetParent();

    DVASSERT(oldParent != nullptr);
    oldBefore = oldParent->GetNextChild(entity);

    TransformComponent* entityTransform = entity->GetComponent<TransformComponent>();
    oldTransform = entityTransform->GetLocalTransform();
    newTransform = EntityParentChangeCommandDetails::ConvertLocalTransform(entity, newParent);
}

EntityParentChangeCommand::~EntityParentChangeCommand()
{
    SafeRelease(entity);
}

void EntityParentChangeCommand::Undo()
{
    if (oldBefore == nullptr)
    {
        oldParent->AddNode(entity);
    }
    else
    {
        oldParent->InsertBeforeNode(entity, oldBefore);
    }

    if (saveEntityPosition == true)
    {
        TransformComponent* entityTransform = entity->GetComponent<TransformComponent>();
        entityTransform->SetLocalTransform(oldTransform);
    }
}

void EntityParentChangeCommand::Redo()
{
    if (newBefore == nullptr)
    {
        newParent->AddNode(entity);
    }
    else
    {
        newParent->InsertBeforeNode(entity, newBefore);
    }

    if (saveEntityPosition == true)
    {
        TransformComponent* entityTransform = entity->GetComponent<TransformComponent>();
        entityTransform->SetLocalTransform(newTransform);
    }
}

Entity* EntityParentChangeCommand::GetEntity() const
{
    return entity;
}

DAVA_VIRTUAL_REFLECTION_IMPL(EntityParentChangeCommand)
{
    ReflectionRegistrator<EntityParentChangeCommand>::Begin()
    .End();
}
} // namespace DAVA
