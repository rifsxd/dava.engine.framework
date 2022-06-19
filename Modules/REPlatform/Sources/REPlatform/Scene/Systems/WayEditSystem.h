#pragma once

#include "REPlatform/DataNodes/SelectableGroup.h"

#include "REPlatform/Scene/Systems/EditorSceneSystem.h"
#include "REPlatform/Scene/Systems/SystemDelegates.h"

#include <Entity/SceneSystem.h>
#include <Render/RenderHelper.h>
#include <Scene3D/Components/Waypoint/EdgeComponent.h>
#include <UI/UIEvent.h>

namespace DAVA
{
class RECommandNotificationObject;
class SceneEditor2;
class WayEditSystem : public SceneSystem,
                      public EntityModificationSystemDelegate,
                      public StructureSystemDelegate,
                      public SelectionSystemDelegate,
                      public EditorSceneSystem

{
    friend class SceneEditor2;

public:
    WayEditSystem(Scene* scene);
    ~WayEditSystem() = default;

    void EnableWayEdit(bool enable);
    bool IsWayEditEnabled() const;

    void Process(float32 timeElapsed) override;
    bool Input(UIEvent* event) override;

    void AddEntity(Entity* entity) override;
    void RemoveEntity(Entity* entity) override;
    void PrepareForRemove() override;

    bool HasCustomClonedAddading(Entity* entityToClone) const override;
    void PerformAdding(Entity* sourceEntity, Entity* clonedEntity) override;

    bool HasCustomRemovingForEntity(Entity* entityToRemove) const override;
    void PerformRemoving(Entity* entityToRemove) override;

protected:
    void Draw() override;
    void ProcessCommand(const RECommandNotificationObject& commandNotification) override;

    void DefineAddOrRemoveEdges(const Vector<PathComponent::Waypoint*>& srcPoints, PathComponent::Waypoint* dstPoint,
                                Vector<PathComponent::Waypoint*>& toAddEdge, Vector<PathComponent::Waypoint*>& toRemoveEdge);
    void AddEdges(PathComponent* path, const Vector<PathComponent::Waypoint*>& srcPoints, PathComponent::Waypoint* nextWaypoint);
    void RemoveEdges(PathComponent* path, const Vector<PathComponent::Waypoint*>& srcPoints, PathComponent::Waypoint* nextWaypoint);

    void ResetSelection();
    void ProcessSelection(const SelectableGroup& selection);
    void UpdateSelectionMask();
    void FilterSelection(PathComponent* path, const Vector<PathComponent::Waypoint*>& srcPoints, Vector<PathComponent::Waypoint*>& validPoints);

    bool AllowPerformSelectionHavingCurrent(const SelectableGroup& currentSelection) override;
    bool AllowChangeSelectionReplacingCurrent(const SelectableGroup& currentSelection, const SelectableGroup& newSelection) override;

    SceneEditor2* GetSceneEditor() const;

private:
    SelectableGroup currentSelection;
    Vector<PathComponent::Waypoint*> selectedWaypoints;
    Vector<PathComponent::Waypoint*> prevSelectedWaypoints;
    Vector<Entity*> waypointEntities;
    PathComponent::Waypoint* underCursorPathEntity = nullptr;
    bool inCloneState = false;
    bool isEnabled = false;
};
} // namespace DAVA
