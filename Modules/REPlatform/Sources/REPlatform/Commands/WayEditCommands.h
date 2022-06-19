#pragma once

#include "REPlatform/Commands/RECommand.h"

#include <Base/BaseTypes.h>
#include <Reflection/Reflection.h>
#include <Scene3D/Components/Waypoint/PathComponent.h>
#include <Scene3D/Scene.h>

namespace DAVA
{
class ToggleWayEditCommand : public RECommand
{
public:
    ToggleWayEditCommand(const String& description, Scene* scene);

protected:
    void EnableWayEdit(bool enable);

private:
    Scene* scene = nullptr;

    DAVA_VIRTUAL_REFLECTION(ToggleWayEditCommand, RECommand);
};

class EnableWayEditCommand : public ToggleWayEditCommand
{
public:
    EnableWayEditCommand(Scene* scene);
    void Undo() override;
    void Redo() override;

private:
    DAVA_VIRTUAL_REFLECTION(EnableWayEditCommand, ToggleWayEditCommand);
};

class DisableWayEditCommand : public ToggleWayEditCommand
{
public:
    DisableWayEditCommand(Scene* scene);
    void Undo() override;
    void Redo() override;

private:
    DAVA_VIRTUAL_REFLECTION(DisableWayEditCommand, ToggleWayEditCommand);
};

class WaypointEditCommand : public RECommand
{
public:
    WaypointEditCommand(Scene* scene, PathComponent* path, PathComponent::Waypoint* waypoint,
                        const String& description, bool isRemovingCommand);
    ~WaypointEditCommand() override;

    Entity* GetEntity() const;

protected:
    void AddWaypoint();
    void RemoveWaypoint();

private:
    Scene* scene = nullptr;
    PathComponent* path = nullptr;
    PathComponent::Waypoint* waypoint = nullptr;

    uint32 waypointIndex = 0;
    bool isWaypointAdded = false;

    DAVA_VIRTUAL_REFLECTION(WaypointEditCommand, RECommand);
};

class AddWaypointCommand : public WaypointEditCommand
{
public:
    AddWaypointCommand(Scene* scene, PathComponent* path, PathComponent::Waypoint* waypoint);

    void Redo() override;
    void Undo() override;

private:
    DAVA_VIRTUAL_REFLECTION(AddWaypointCommand, WaypointEditCommand);
};

class RemoveWaypointCommand : public WaypointEditCommand
{
public:
    RemoveWaypointCommand(Scene* scene, PathComponent* path, PathComponent::Waypoint* waypoint);

    void Redo() override;
    void Undo() override;

private:
    DAVA_VIRTUAL_REFLECTION(RemoveWaypointCommand, WaypointEditCommand);
};

class EdgeEditCommand : public RECommand
{
public:
    EdgeEditCommand(Scene* scene, PathComponent* path,
                    PathComponent::Waypoint* waypoint, PathComponent::Edge* edge,
                    const String& description, bool isRemovingCommand);
    ~EdgeEditCommand() override;

    Entity* GetEntity() const;

protected:
    void AddEdge();
    void RemoveEdge();

private:
    Scene* scene = nullptr;
    PathComponent* path = nullptr;
    PathComponent::Waypoint* waypoint = nullptr;
    PathComponent::Waypoint* destination = nullptr;
    PathComponent::Edge* edge = nullptr;

    size_t edgeIndex = 0;
    bool isEdgeAdded = false;

    DAVA_VIRTUAL_REFLECTION(EdgeEditCommand, RECommand);
};

class AddEdgeCommand : public EdgeEditCommand
{
public:
    AddEdgeCommand(Scene* scene, PathComponent* path, PathComponent::Waypoint* waypoint, PathComponent::Edge* edge);

    void Redo() override;
    void Undo() override;

private:
    DAVA_VIRTUAL_REFLECTION(AddEdgeCommand, EdgeEditCommand);
};

class RemoveEdgeCommand : public EdgeEditCommand
{
public:
    RemoveEdgeCommand(Scene* scene, PathComponent* path, PathComponent::Waypoint* waypoint, PathComponent::Edge* edge);

    void Redo() override;
    void Undo() override;

private:
    DAVA_VIRTUAL_REFLECTION(RemoveEdgeCommand, EdgeEditCommand);
};
} // namespace DAVA
