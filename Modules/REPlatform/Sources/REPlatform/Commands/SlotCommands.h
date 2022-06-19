#pragma once

#include "REPlatform/Commands/RECommand.h"

#include <Base/FastName.h>
#include <Base/RefPtr.h>
#include <Reflection/Reflection.h>

namespace DAVA
{
class Entity;
class SlotComponent;
class Scene;

class AttachEntityToSlot : public RECommand
{
public:
    explicit AttachEntityToSlot(Scene* scene, SlotComponent* slotComponent, Entity* entity, FastName itemName);
    explicit AttachEntityToSlot(Scene* scene, SlotComponent* slotComponent, FastName itemName);

    void Redo() override;
    void Undo() override;

    bool IsClean() const override;

private:
    Scene* scene = nullptr;
    SlotComponent* component = nullptr;
    RefPtr<Entity> redoEntity;
    RefPtr<Entity> undoEntity;
    FastName undoItemName;
    FastName redoItemName;
    bool redoEntityInited = false;
    bool executed = false;

    DAVA_VIRTUAL_REFLECTION(AttachEntityToSlot, RECommand);
};

class SlotTypeFilterEdit : public RECommand
{
public:
    explicit SlotTypeFilterEdit(SlotComponent* slotComponent, FastName typeFilter, bool isAddCommand);

    void Redo() override;
    void Undo() override;

private:
    SlotComponent* slotComponent = nullptr;
    FastName typeFilter;
    bool isAddCommand = true;

    DAVA_VIRTUAL_REFLECTION(SlotTypeFilterEdit, RECommand);
};
} // namespace DAVA
