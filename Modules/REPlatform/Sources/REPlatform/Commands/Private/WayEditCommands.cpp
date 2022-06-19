#include "REPlatform/Commands/WayEditCommands.h"
#include "REPlatform/Scene/Systems/SelectionSystem.h"
#include "REPlatform/Scene/Systems/PathSystem.h"
#include "REPlatform/Scene/Systems/WayEditSystem.h"

#include <Reflection/ReflectionRegistrator.h>
#include <Debug/DVAssert.h>

namespace DAVA
{
ToggleWayEditCommand::ToggleWayEditCommand(const String& description, Scene* scene_)
    : RECommand(description)
    , scene(scene_)
{
}

void ToggleWayEditCommand::EnableWayEdit(bool enable)
{
    PathSystem* pathSystem = scene->GetSystem<PathSystem>();
    bool isEditorEnabled = pathSystem->IsPathEditEnabled();
    DVASSERT(isEditorEnabled != enable);

    SelectionSystem* selectionSystem = scene->GetSystem<SelectionSystem>();
    bool wasLocked = selectionSystem->IsLocked();
    selectionSystem->SetLocked(true);
    pathSystem->EnablePathEdit(enable);
    selectionSystem->SetLocked(wasLocked);
}

DAVA_VIRTUAL_REFLECTION_IMPL(ToggleWayEditCommand)
{
    ReflectionRegistrator<ToggleWayEditCommand>::Begin()
    .End();
}

EnableWayEditCommand::EnableWayEditCommand(Scene* scene)
    : ToggleWayEditCommand("Enable waypoint edit mode", scene)
{
}

void EnableWayEditCommand::Undo()
{
    EnableWayEdit(false);
}

void EnableWayEditCommand::Redo()
{
    EnableWayEdit(true);
}

DAVA_VIRTUAL_REFLECTION_IMPL(EnableWayEditCommand)
{
    ReflectionRegistrator<EnableWayEditCommand>::Begin()
    .End();
}

DisableWayEditCommand::DisableWayEditCommand(Scene* scene)
    : ToggleWayEditCommand("Disable waypoint edit mode", scene)
{
}

void DisableWayEditCommand::Undo()
{
    EnableWayEdit(true);
}

void DisableWayEditCommand::Redo()
{
    EnableWayEdit(false);
}

DAVA_VIRTUAL_REFLECTION_IMPL(DisableWayEditCommand)
{
    ReflectionRegistrator<DisableWayEditCommand>::Begin()
    .End();
}

WaypointEditCommand::WaypointEditCommand(Scene* scene_, PathComponent* path_, PathComponent::Waypoint* waypoint_,
                                         const String& description, bool isRemovingCommand)
    : RECommand(description)
    , scene(scene_)
    , path(path_)
    , waypoint(waypoint_)
{
    DVASSERT(path);
    DVASSERT(waypoint);

    isWaypointAdded = false;
    const Vector<PathComponent::Waypoint*>& points = path->GetPoints();
    for (size_t i = 0; i < points.size(); ++i)
    {
        if (points[i] == waypoint)
        {
            isWaypointAdded = true;
            waypointIndex = static_cast<uint32>(i);
            break;
        }
    }

    DVASSERT(isWaypointAdded == isRemovingCommand);
    if (isWaypointAdded == false)
    {
        waypointIndex = static_cast<uint32>(points.size());
    }
}

WaypointEditCommand::~WaypointEditCommand()
{
    if (isWaypointAdded == false)
    {
        scene->GetSystem<PathSystem>()->OnWaypointDeleted(path, waypoint);
        SafeDelete(waypoint);
    }
}

Entity* WaypointEditCommand::GetEntity() const
{
    return path->GetEntity();
}

void WaypointEditCommand::AddWaypoint()
{
    DVASSERT(isWaypointAdded == false);

    path->InsertPoint(waypoint, waypointIndex);
    scene->GetSystem<PathSystem>()->OnWaypointAdded(path, waypoint);
    isWaypointAdded = true;
}

void WaypointEditCommand::RemoveWaypoint()
{
    DVASSERT(isWaypointAdded == true);
    path->ExtractPoint(waypoint);
    scene->GetSystem<PathSystem>()->OnWaypointRemoved(path, waypoint);
    isWaypointAdded = false;
}

DAVA_VIRTUAL_REFLECTION_IMPL(WaypointEditCommand)
{
    ReflectionRegistrator<WaypointEditCommand>::Begin()
    .End();
}

AddWaypointCommand::AddWaypointCommand(Scene* scene, PathComponent* path, PathComponent::Waypoint* waypoint)
    : WaypointEditCommand(scene, path, waypoint, "Add waypoint", false)
{
}

void AddWaypointCommand::Redo()
{
    AddWaypoint();
}

void AddWaypointCommand::Undo()
{
    RemoveWaypoint();
}

DAVA_VIRTUAL_REFLECTION_IMPL(AddWaypointCommand)
{
    ReflectionRegistrator<AddWaypointCommand>::Begin()
    .End();
}

RemoveWaypointCommand::RemoveWaypointCommand(Scene* scene, PathComponent* path, PathComponent::Waypoint* waypoint)
    : WaypointEditCommand(scene, path, waypoint, "Remove waypoint", true)
{
}

void RemoveWaypointCommand::Redo()
{
    RemoveWaypoint();
}

void RemoveWaypointCommand::Undo()
{
    AddWaypoint();
}

DAVA_VIRTUAL_REFLECTION_IMPL(RemoveWaypointCommand)
{
    ReflectionRegistrator<RemoveWaypointCommand>::Begin()
    .End();
}

EdgeEditCommand::EdgeEditCommand(Scene* scene_, PathComponent* path_, PathComponent::Waypoint* waypoint_,
                                 PathComponent::Edge* edge_, const String& description, bool isRemovingCommand)
    : RECommand(description)
    , scene(scene_)
    , path(path_)
    , waypoint(waypoint_)
    , edge(edge_)
{
    DVASSERT(edge->destination != nullptr);
    DVASSERT(edge->destination != waypoint);
    destination = edge->destination;

    isEdgeAdded = false;
    for (size_t i = 0; i < waypoint->edges.size(); ++i)
    {
        if (waypoint->edges[i] == edge)
        {
            isEdgeAdded = true;
            edgeIndex = i;
            break;
        }
    }

    DVASSERT(isEdgeAdded == isRemovingCommand);
    if (isEdgeAdded == false)
    {
        edgeIndex = waypoint->edges.size();
    }
}

EdgeEditCommand::~EdgeEditCommand()
{
    if (isEdgeAdded == false)
    {
        scene->GetSystem<PathSystem>()->OnEdgeDeleted(path, waypoint, edge);
        SafeDelete(edge);
    }
}

Entity* EdgeEditCommand::GetEntity() const
{
    return path->GetEntity();
}

void EdgeEditCommand::AddEdge()
{
    DVASSERT(isEdgeAdded == false);

#if defined(__DAVAENGINE_DEBUG__)
    bool destinationFound = false;
    for (PathComponent::Waypoint* wp : path->GetPoints())
    {
        if (wp == destination)
        {
            destinationFound = true;
            break;
        }
    }
    DVASSERT(destinationFound);
#endif

    edge->destination = destination;
    if (edgeIndex < waypoint->edges.size())
    {
        waypoint->edges.insert(waypoint->edges.begin() + edgeIndex, edge);
    }
    else
    {
        waypoint->edges.push_back(edge);
    }

    scene->GetSystem<PathSystem>()->OnEdgeAdded(path, waypoint, edge);
    isEdgeAdded = true;
}

void EdgeEditCommand::RemoveEdge()
{
    DVASSERT(isEdgeAdded == true);
    DVASSERT(waypoint->edges[edgeIndex] == edge);
    DVASSERT(edge->destination == destination);

    waypoint->edges.erase(waypoint->edges.begin() + edgeIndex);
    scene->GetSystem<PathSystem>()->OnEdgeRemoved(path, waypoint, edge);
    isEdgeAdded = false;
}

DAVA_VIRTUAL_REFLECTION_IMPL(EdgeEditCommand)
{
    ReflectionRegistrator<EdgeEditCommand>::Begin()
    .End();
}

AddEdgeCommand::AddEdgeCommand(Scene* scene, PathComponent* path, PathComponent::Waypoint* waypoint, PathComponent::Edge* edge)
    : EdgeEditCommand(scene, path, waypoint, edge, "Add edge", false)
{
}

void AddEdgeCommand::Redo()
{
    AddEdge();
}

void AddEdgeCommand::Undo()
{
    RemoveEdge();
}

DAVA_VIRTUAL_REFLECTION_IMPL(AddEdgeCommand)
{
    ReflectionRegistrator<AddEdgeCommand>::Begin()
    .End();
}

RemoveEdgeCommand::RemoveEdgeCommand(Scene* scene, PathComponent* path, PathComponent::Waypoint* waypoint, PathComponent::Edge* edge)
    : EdgeEditCommand(scene, path, waypoint, edge, "Remove edge", true)
{
}

void RemoveEdgeCommand::Redo()
{
    RemoveEdge();
}

void RemoveEdgeCommand::Undo()
{
    AddEdge();
}

DAVA_VIRTUAL_REFLECTION_IMPL(RemoveEdgeCommand)
{
    ReflectionRegistrator<RemoveEdgeCommand>::Begin()
    .End();
}

} // namespace DAVA
