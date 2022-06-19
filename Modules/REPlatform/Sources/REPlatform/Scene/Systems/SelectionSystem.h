#pragma once

#include "REPlatform/Scene/SceneTypes.h"
#include "REPlatform/DataNodes/SelectableGroup.h"

#include "REPlatform/Scene/Systems/SystemDelegates.h"
#include "REPlatform/Scene/Systems/EditorSceneSystem.h"

#include <Entity/SceneSystem.h>
#include <Render/RenderHelper.h>
#include <Render/UniqueStateSet.h>
#include <Scene3D/Entity.h>
#include <UI/UIEvent.h>

#include "Render/UniqueStateSet.h"
#include "Render/RenderHelper.h"

namespace DAVA
{
class RECommandNotificationObject;
class SceneCollisionSystem;
class HoodSystem;
class EntityModificationSystem;

class SelectionSystem : public SceneSystem, public EditorSceneSystem
{
public:
    SelectionSystem(Scene* scene);
    ~SelectionSystem();

    void Clear();

    bool IsEntitySelectable(Entity* entity) const;

    /*
     * SetSelection could remove not selectable items from provided group
     */
    void SetSelection(SelectableGroup& newSelection);
    const SelectableGroup& GetSelection() const;
    const AABBox3& GetSelectionBox() const;

    void ResetSelectionComponentMask();
    void SetSelectionComponentMask(const ComponentMask& mask);
    const ComponentMask& GetSelectionComponentMask() const;

    void SetSelectionAllowed(bool allowed);
    bool IsSelectionAllowed() const;

    void SetLocked(bool lock) override;

    void Process(float32 timeElapsed) override;
    bool Input(UIEvent* event) override;

    void AddEntity(Entity* entity) override;
    void RemoveEntity(Entity* entity) override;
    void PrepareForRemove() override;

    void Activate() override;
    void Deactivate() override;

    void CancelSelection();

    void AddDelegate(SelectionSystemDelegate* delegate);
    void RemoveDelegate(SelectionSystemDelegate* delegate);

protected:
    void Draw() override;
    void ProcessCommand(const RECommandNotificationObject& commandNotification) override;

private:
    void ImmediateEvent(Component* component, uint32 event) override;

    void UpdateHoodPos() const;

    void PerformSelectionAtPoint(const Vector2&);
    void PerformSelectionInCurrentBox();
    void ProcessSelectedGroup(const SelectableGroup::CollectionType&);

    void UpdateGroupSelectionMode();
    void UpdateSelectionGroup(const SelectableGroup& newSelection);
    void FinishSelection();

    void ExcludeSingleItem(const Any& object);

    void DrawItem(const AABBox3& bbox, const Matrix4& transform, int32 drawMode,
                  RenderHelper::eDrawType wireDrawType, RenderHelper::eDrawType solidDrawType,
                  const Color& color);

    enum class GroupSelectionMode
    {
        Replace,
        Add,
        Remove
    };

    SceneCollisionSystem* collisionSystem = nullptr;
    HoodSystem* hoodSystem = nullptr;
    EntityModificationSystem* modificationSystem = nullptr;
    SelectableGroup currentSelection;
    SelectableGroup lastGroupSelection;
    SelectableGroup objectsToSelect;
    List<Entity*> entitiesForSelection;
    Vector2 selectionStartPoint;
    Vector2 selectionEndPoint;
    ComponentMask componentMaskForSelection;
    Vector<SelectionSystemDelegate*> selectionDelegates;
    GroupSelectionMode groupSelectionMode = GroupSelectionMode::Replace;
    bool selectionAllowed = true;
    bool applyOnPhaseEnd = false;

    AABBox3 selectionBox;
    bool invalidSelectionBoxes = false;

    bool selecting = false;

    bool wasLockedInActiveMode = false;
};

inline void SelectionSystem::ResetSelectionComponentMask()
{
    componentMaskForSelection.set();
}

inline const ComponentMask& SelectionSystem::GetSelectionComponentMask() const
{
    return componentMaskForSelection;
}

inline void SelectionSystem::SetSelectionAllowed(bool allowed)
{
    selectionAllowed = allowed;
}

inline bool SelectionSystem::IsSelectionAllowed() const
{
    return selectionAllowed;
}

Entity* GetSelectableEntity(Entity* selectionCandidate);
Selectable GetSelectableObject(const Selectable& object);
} // namespace DAVA
