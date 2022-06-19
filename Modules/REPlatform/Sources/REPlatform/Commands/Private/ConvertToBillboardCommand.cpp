#include "REPlatform/Commands/ConvertToBillboardCommand.h"

#include <Base/ScopedPtr.h>
#include <Reflection/ReflectionRegistrator.h>
#include <Render/Highlevel/BillboardRenderObject.h>
#include <Scene3D/Components/ComponentHelpers.h>
#include <Scene3D/Components/RenderComponent.h>

namespace DAVA
{
ConvertToBillboardCommand::ConvertToBillboardCommand(RenderObject* ro, Entity* entity_)
    : RECommand("Convert to billboard")
    , entity(entity_)
    , oldRenderComponent(GetRenderComponent(entity))
    , newRenderComponent(new RenderComponent())
{
    ScopedPtr<RenderObject> newRenderObject(new BillboardRenderObject());
    oldRenderComponent->GetRenderObject()->Clone(newRenderObject);
    newRenderObject->AddFlag(RenderObject::eFlags::CUSTOM_PREPARE_TO_RENDER);
    newRenderObject->RecalcBoundingBox();

    newRenderComponent->SetRenderObject(newRenderObject);

    detachedComponent = newRenderComponent;
}

ConvertToBillboardCommand::~ConvertToBillboardCommand()
{
    DVASSERT(detachedComponent->GetEntity() == nullptr);
    SafeDelete(detachedComponent);
}

void ConvertToBillboardCommand::Redo()
{
    entity->DetachComponent(oldRenderComponent);
    entity->AddComponent(newRenderComponent);

    entity->GetScene()->GetRenderSystem()->MarkForUpdate(newRenderComponent->GetRenderObject());

    detachedComponent = oldRenderComponent;
}

void ConvertToBillboardCommand::Undo()
{
    entity->DetachComponent(newRenderComponent);
    entity->AddComponent(oldRenderComponent);

    detachedComponent = newRenderComponent;
}

DAVA_VIRTUAL_REFLECTION_IMPL(ConvertToBillboardCommand)
{
    ReflectionRegistrator<ConvertToBillboardCommand>::Begin()
    .End();
}
} // namespace DAVA
