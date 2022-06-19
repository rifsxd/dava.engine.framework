#include "REPlatform/Commands/SlotCommands.h"
#include "REPlatform/Scene/Systems/EditorSlotSystem.h"

#include <Reflection/ReflectionRegistrator.h>
#include <Scene3D/Components/SlotComponent.h>
#include <Scene3D/Entity.h>
#include <Scene3D/Scene.h>
#include <Scene3D/Systems/SlotSystem.h>
#include <Logger/Logger.h>

namespace DAVA
{
AttachEntityToSlot::AttachEntityToSlot(Scene* scene_, SlotComponent* slotComponent, Entity* entity, FastName itemName)
    : RECommand("Add item to slot")
    , scene(scene_)
    , component(slotComponent)
    , redoEntity(RefPtr<Entity>::ConstructWithRetain(entity))
    , redoItemName(itemName)
    , redoEntityInited(true)
{
}

AttachEntityToSlot::AttachEntityToSlot(Scene* scene_, SlotComponent* slotComponent, FastName itemName)
    : RECommand("Add item to slot")
    , scene(scene_)
    , component(slotComponent)
    , redoItemName(itemName)
    , redoEntityInited(false)
{
    DVASSERT(redoItemName.IsValid());
}

void AttachEntityToSlot::Redo()
{
    EditorSlotSystem* system = scene->GetSystem<EditorSlotSystem>();
    DVASSERT(system != nullptr);

    if (executed == false)
    {
        undoEntity = RefPtr<Entity>::ConstructWithRetain(scene->slotSystem->LookUpLoadedEntity(component));
        undoItemName = component->GetLoadedItemName();
        executed = true;
    }

    if (undoEntity.Get() != nullptr)
    {
        system->DetachEntity(component, undoEntity.Get());
    }

    if (redoEntityInited == false)
    {
        redoEntity = system->AttachEntity(component, redoItemName);
        if (redoEntity.Get() == nullptr)
        {
            Logger::Error("Couldn't load item %s to slot %s", redoItemName.c_str(), component->GetSlotName().c_str());
            redoEntity.ConstructInplace();
            redoItemName = EditorSlotSystem::emptyItemName;
        }
        redoEntityInited = true;
    }
    else if (redoEntity.Get() != nullptr)
    {
        system->AttachEntity(component, redoEntity.Get(), redoItemName);
    }
}

void AttachEntityToSlot::Undo()
{
    EditorSlotSystem* system = scene->GetSystem<EditorSlotSystem>();
    DVASSERT(system != nullptr);
    if (redoEntity.Get() != nullptr)
    {
        system->DetachEntity(component, redoEntity.Get());
    }

    if (undoEntity.Get() != nullptr)
    {
        system->AttachEntity(component, undoEntity.Get(), undoItemName);
    }
}

bool AttachEntityToSlot::IsClean() const
{
    return true;
}

DAVA_VIRTUAL_REFLECTION_IMPL(AttachEntityToSlot)
{
    ReflectionRegistrator<AttachEntityToSlot>::Begin()
    .End();
}

SlotTypeFilterEdit::SlotTypeFilterEdit(SlotComponent* slotComponent_, FastName typeFilter_, bool isAddCommand_)
    : RECommand("Edit slot filters")
    , slotComponent(slotComponent_)
    , typeFilter(typeFilter_)
    , isAddCommand(isAddCommand_)
{
    if (isAddCommand == false)
    {
        bool filterFound = false;
        for (uint32 i = 0; i < slotComponent->GetTypeFiltersCount(); ++i)
        {
            if (slotComponent->GetTypeFilter(i) == typeFilter)
            {
                filterFound = true;
                break;
            }
        }

        DVASSERT(filterFound == true);
    }
}

void SlotTypeFilterEdit::Redo()
{
    if (isAddCommand == true)
    {
        slotComponent->AddTypeFilter(typeFilter);
    }
    else
    {
        slotComponent->RemoveTypeFilter(typeFilter);
    }
}

void SlotTypeFilterEdit::Undo()
{
    if (isAddCommand == false)
    {
        slotComponent->AddTypeFilter(typeFilter);
    }
    else
    {
        slotComponent->RemoveTypeFilter(typeFilter);
    }
}

DAVA_VIRTUAL_REFLECTION_IMPL(SlotTypeFilterEdit)
{
    ReflectionRegistrator<SlotTypeFilterEdit>::Begin()
    .End();
}
} // namespace DAVA
