#pragma once

#include "REPlatform/Scene/Systems/EditorSceneSystem.h"
#include "REPlatform/Scene/Systems/SystemDelegates.h"

#include <TArc/Utils/QtConnections.h>

#include <Base/BaseTypes.h>
#include <Entity/SceneSystem.h>
#include <Scene3D/Components/SlotComponent.h>
#include <Reflection/Reflection.h>

namespace DAVA
{
class LoadedSlotItemComponent : public Component
{
public:
    Component* Clone(Entity* toEntity) override;

    SlotComponent* parentComponent = nullptr;

    DAVA_VIRTUAL_REFLECTION(LoadedSlotItemComponent, Component);
};

class ContextAccessor;
class EditorSlotSystem : public SceneSystem,
                         public EditorSceneSystem,
                         public EntityModificationSystemDelegate
{
public:
    static const FastName emptyItemName;

    EditorSlotSystem(Scene* scene, ContextAccessor* accessor);

    void AddEntity(Entity* entity) override;
    void RemoveEntity(Entity* entity) override;

    void AddComponent(Entity* entity, Component* component) override;
    void RemoveComponent(Entity* entity, Component* component) override;

    void PrepareForRemove() override;

    void Process(float32 timeElapsed) override;

    void WillClone(Entity* originalEntity) override;
    void DidCloned(Entity* originalEntity, Entity* newEntity) override;

    RefPtr<KeyedArchive> SaveSlotsPreset(Entity* entity);
    void LoadSlotsPreset(Entity* entity, RefPtr<KeyedArchive> archive);

    static FastName GenerateUniqueSlotName(SlotComponent* component);
    static FastName GenerateUniqueSlotName(SlotComponent* component, Entity* entity, const FastName& newTemplateName,
                                           const FastName& newEntityName, const Set<FastName>& reservedName);

protected:
    friend class AttachEntityToSlot;

    void DetachEntity(SlotComponent* component, Entity* entity);
    void AttachEntity(SlotComponent* component, Entity* entity, FastName itemName);
    RefPtr<Entity> AttachEntity(SlotComponent* component, FastName itemName);

    void AccumulateDependentCommands(REDependentCommandsHolder& holder) override;
    void ProcessCommand(const RECommandNotificationObject& commandNotification) override;
    FastName GetSuitableItemName(SlotComponent* component) const;

    void Draw() override;

protected:
    std::unique_ptr<Command> PrepareForSave(bool saveForGame) override;
    void SetScene(Scene* scene) override;

private:
    void LoadSlotsPresetImpl(Entity* entity, RefPtr<KeyedArchive> archive);
    Vector<Entity*> entities;
    Set<Entity*> pendingOnInitialize;
    QtConnections connections;
    ContextAccessor* accessor;

    struct AttachedItemInfo
    {
        SlotComponent* component = nullptr;
        RefPtr<Entity> entity;
        FastName itemName;
    };

    UnorderedMap<Entity*, Vector<AttachedItemInfo>> inClonedState;
};
} // namespace DAVA
