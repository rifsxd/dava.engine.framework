#pragma once

#include "REPlatform/Scene/Systems/EditorSceneSystem.h"
#include "REPlatform/Scene/Systems/ModifSystem.h"
#include "REPlatform/Scene/Systems/SystemDelegates.h"

#include <Base/BaseTypes.h>
#include <Base/FastName.h>
#include <Entity/SceneSystem.h>
#include <Scene3D/Components/Waypoint/EdgeComponent.h>
#include <Scene3D/Components/Waypoint/PathComponent.h>
#include <Scene3D/Entity.h>

namespace DAVA
{
static const uint32 WAYPOINTS_DRAW_LIFTING = 1;

class RECommandNotificationObject;
class PathSystem : public SceneSystem, public EntityModificationSystemDelegate, public EditorSceneSystem
{
public:
    PathSystem(Scene* scene);
    ~PathSystem() override;

    void EnablePathEdit(bool enable);
    bool IsPathEditEnabled() const;

    void AddEntity(Entity* entity) override;
    void RemoveEntity(Entity* entity) override;
    void PrepareForRemove() override;

    void Process(float32 timeElapsed) override;

    Entity* GetCurrrentPath() const;
    const Vector<Entity*>& GetPathes() const;

    void WillClone(Entity* originalEntity) override;
    void DidCloned(Entity* originalEntity, Entity* newEntity) override;

    void OnWaypointAdded(PathComponent* path, PathComponent::Waypoint* waypoint);
    void OnWaypointRemoved(PathComponent* path, PathComponent::Waypoint* waypoint);
    void OnEdgeAdded(PathComponent* path, PathComponent::Waypoint* waypoint, PathComponent::Edge* edge);
    void OnEdgeRemoved(PathComponent* path, PathComponent::Waypoint* waypoint, PathComponent::Edge* edge);
    void OnWaypointDeleted(PathComponent* path, PathComponent::Waypoint* waypoint);
    void OnEdgeDeleted(PathComponent* path, PathComponent::Waypoint* waypoint, PathComponent::Edge* edge);

    void BakeWaypoints();

protected:
    void Draw() override;

    void DrawInEditableMode();
    void DrawInViewOnlyMode();
    void DrawArrow(const Vector3& start, const Vector3& finish, const Color& color);

    void InitPathComponent(PathComponent* component);

    FastName GeneratePathName() const;
    const Color& GetNextPathColor() const;

    void ExpandPathEntity(Entity* entity);
    void CollapsePathEntity(Entity* entity, FastName pathName);

    Vector<Entity*> pathes;
    Set<std::pair<Entity*, FastName>> entitiesForCollapse;
    Set<Entity*> entitiesForExpand;
    UnorderedMap<PathComponent::Waypoint*, RefPtr<Entity>> entityCache;
    UnorderedMap<PathComponent::Edge*, EdgeComponent*> edgeComponentCache;

    SelectableGroup currentSelection;
    Entity* currentPath;

    bool isEditingEnabled;
};

inline const Vector<Entity*>& PathSystem::GetPathes() const
{
    return pathes;
}

inline Entity* PathSystem::GetCurrrentPath() const
{
    return currentPath;
}

inline bool PathSystem::IsPathEditEnabled() const
{
    return isEditingEnabled;
}
} // namespace DAVA
