#include "REPlatform/Scene/Systems/WayEditSystem.h"
#include "REPlatform/Scene/Systems/CollisionSystem.h"
#include "REPlatform/Scene/Systems/ModifSystem.h"
#include "REPlatform/Scene/Systems/PathSystem.h"
#include "REPlatform/Scene/Systems/SelectionSystem.h"

#include "REPlatform/Commands/RECommandNotificationObject.h"
#include "REPlatform/Commands/WayEditCommands.h"
#include "REPlatform/Scene/SceneEditor2.h"

#include <Debug/DVAssert.h>
#include <DeviceManager/DeviceManager.h>
#include <Engine/Engine.h>
#include <Engine/EngineContext.h>
#include <Entity/ComponentUtils.h>
#include <Input/Keyboard.h>
#include <Math/AABBox3.h>
#include <Scene3D/Components/ComponentHelpers.h>
#include <Scene3D/Components/SingleComponents/TransformSingleComponent.h>
#include <Scene3D/Components/TransformComponent.h>
#include <Scene3D/Components/Waypoint/PathComponent.h>
#include <Scene3D/Components/Waypoint/WaypointComponent.h>
#include <Utils/Utils.h>

namespace DAVA
{
namespace WayEditSystemDetail
{
struct AccessibleQueryParams
{
    PathComponent::Waypoint* startPoint = nullptr;
    PathComponent::Waypoint* destinationPoint = nullptr;
    PathComponent::Waypoint* excludePoint = nullptr;
    PathComponent::Edge* excludeEdge = nullptr;
};

bool IsAccessibleImpl(const AccessibleQueryParams& params, Set<PathComponent::Waypoint*>& passedPoints)
{
    DVASSERT(params.startPoint != nullptr);
    DVASSERT(params.destinationPoint != nullptr);
    if (params.startPoint == params.destinationPoint)
    {
        return true;
    }

    for (PathComponent::Edge* edge : params.startPoint->edges)
    {
        if (edge == params.excludeEdge || edge->destination == params.excludePoint)
        {
            continue;
        }

        if (passedPoints.insert(edge->destination).second == false)
        {
            continue;
        }

        AccessibleQueryParams deeperParams = params;
        deeperParams.startPoint = edge->destination;
        if (IsAccessibleImpl(deeperParams, passedPoints) == true)
        {
            return true;
        }
    }

    return false;
}

bool IsAccessible(const AccessibleQueryParams& params)
{
    Set<PathComponent::Waypoint*> passedPoints;
    return IsAccessibleImpl(params, passedPoints);
}
} // namespace WayEditSystemDetail

WayEditSystem::WayEditSystem(Scene* scene)
    : SceneSystem(scene)
{
}

void WayEditSystem::AddEntity(Entity* newWaypoint)
{
    waypointEntities.push_back(newWaypoint);
}

void WayEditSystem::RemoveEntity(Entity* removedPoint)
{
    FindAndRemoveExchangingWithLast(waypointEntities, removedPoint);
    WaypointComponent* waypointComponent = GetWaypointComponent(removedPoint);
    if (waypointComponent == nullptr)
    {
        return;
    }

    PathComponent::Waypoint* waypoint = waypointComponent->GetWaypoint();

    if (currentSelection.ContainsObject(removedPoint))
    {
        currentSelection.Remove(removedPoint);
    }

    FindAndRemoveExchangingWithLast(selectedWaypoints, waypoint);
    FindAndRemoveExchangingWithLast(prevSelectedWaypoints, waypoint);
}

void WayEditSystem::PrepareForRemove()
{
    waypointEntities.clear();
    selectedWaypoints.clear();
    prevSelectedWaypoints.clear();
    currentSelection.Clear();
}

bool WayEditSystem::HasCustomRemovingForEntity(Entity* entityToRemove) const
{
    if (IsWayEditEnabled() == false)
    {
        DVASSERT(entityToRemove->GetComponentCount<WaypointComponent>() == 0);
        DVASSERT(entityToRemove->GetComponentCount<EdgeComponent>() == 0);
        return false;
    }

    return GetWaypointComponent(entityToRemove) != nullptr;
}

void WayEditSystem::PerformRemoving(Entity* entityToRemove)
{
    using namespace DAVA;
    DVASSERT(IsWayEditEnabled() == true);

    WaypointComponent* waypointComponent = GetWaypointComponent(entityToRemove);
    DVASSERT(waypointComponent != nullptr);
    DVASSERT(waypointComponent->GetWaypoint()->IsStarting() == false);

    PathComponent* path = waypointComponent->GetPath();
    PathComponent::Waypoint* waypointToRemove = waypointComponent->GetWaypoint();
// lookup path that point was removed from
#if defined(__DAVAENGINE_DEBUG__)
    Entity* parentEntity = entityToRemove->GetParent();
    bool pathFound = false;
    DVASSERT(path == GetPathComponent(parentEntity));

    bool waypointFound = false;
    for (PathComponent::Waypoint* waypoint : path->GetPoints())
    {
        if (waypoint == waypointToRemove)
        {
            waypointFound = true;
            break;
        }
    }
    DVASSERT(waypointFound);
#endif

    Vector<std::pair<PathComponent::Waypoint*, PathComponent::Edge*>> edgesToRemovedPoint;

    const Vector<PathComponent::Waypoint*>& pathPoints = path->GetPoints();
    for (PathComponent::Waypoint* checkWaypoint : pathPoints)
    {
        if (checkWaypoint == waypointToRemove)
        {
            continue;
        }

        for (PathComponent::Edge* edge : checkWaypoint->edges)
        {
            if (edge->destination == waypointToRemove)
            {
                edgesToRemovedPoint.push_back(std::make_pair(checkWaypoint, edge));
            }
        }
    }

    PathComponent::Waypoint* startPoint = path->GetStartWaypoint();
    DVASSERT(startPoint != nullptr);

    WayEditSystemDetail::AccessibleQueryParams params;
    params.startPoint = startPoint;
    params.excludePoint = waypointToRemove;

    Vector<PathComponent::Waypoint*> inaccessiblePoints;
    for (PathComponent::Edge* edge : waypointToRemove->edges)
    {
        params.destinationPoint = edge->destination;
        if (WayEditSystemDetail::IsAccessible(params) == false)
        {
            inaccessiblePoints.push_back(edge->destination);
        }
    }

    SceneEditor2* sceneEditor = GetSceneEditor();
    for (auto& edgeNode : edgesToRemovedPoint)
    {
        sceneEditor->Exec(std::make_unique<RemoveEdgeCommand>(sceneEditor, path, edgeNode.first, edgeNode.second));
    }

    Vector<PathComponent::Edge*> edgesToRemove = waypointToRemove->edges;
    for (PathComponent::Edge* e : edgesToRemove)
    {
        sceneEditor->Exec(std::make_unique<RemoveEdgeCommand>(sceneEditor, path, waypointToRemove, e));
    }

    sceneEditor->Exec(std::make_unique<RemoveWaypointCommand>(sceneEditor, path, waypointToRemove));

    for (PathComponent::Waypoint* inaccessiblePoint : inaccessiblePoints)
    {
        for (auto& srcNode : edgesToRemovedPoint)
        {
            if (srcNode.first != inaccessiblePoint)
            {
                PathComponent::Edge* newEdge = new PathComponent::Edge();
                newEdge->destination = inaccessiblePoint;
                sceneEditor->Exec(std::make_unique<AddEdgeCommand>(sceneEditor, path, srcNode.first, newEdge));
            }
        }
    }
}

void WayEditSystem::Process(float32 timeElapsed)
{
    TransformSingleComponent* tsc = GetScene()->transformSingleComponent;
    for (auto& pair : tsc->worldTransformChanged.map)
    {
        if (pair.first->GetComponentsCount(Type::Instance<WaypointComponent>()) > 0)
        {
            for (Entity* entity : pair.second)
            {
                WaypointComponent* waypoint = entity->GetComponent<WaypointComponent>();
                if (waypoint != nullptr && waypoint->GetWaypoint() != nullptr)
                {
                    TransformComponent* tc = entity->GetComponent<TransformComponent>();
                    waypoint->GetWaypoint()->position = tc->GetLocalTransform().GetTranslation();
                }
            }
        }
    }
}

void WayEditSystem::ResetSelection()
{
    selectedWaypoints.clear();
    prevSelectedWaypoints.clear();
    underCursorPathEntity = nullptr;
}

void WayEditSystem::ProcessSelection(const SelectableGroup& selection)
{
    prevSelectedWaypoints = selectedWaypoints;
    selectedWaypoints.clear();

    if (currentSelection != selection)
    {
        currentSelection = selection;

        for (auto entity : currentSelection.ObjectsOfType<Entity>())
        {
            WaypointComponent* waypointComponent = GetWaypointComponent(entity);
            if (waypointComponent != nullptr && GetPathComponent(entity->GetParent()))
            {
                selectedWaypoints.push_back(waypointComponent->GetWaypoint());
            }
        }
    }
}

bool WayEditSystem::Input(UIEvent* event)
{
    SceneEditor2* sceneEditor = GetSceneEditor();
    if (isEnabled && (eMouseButtons::LEFT == event->mouseButton))
    {
        EntityModificationSystem* modifSystem = sceneEditor->GetSystem<EntityModificationSystem>();
        if (UIEvent::Phase::MOVE == event->phase)
        {
            underCursorPathEntity = nullptr;
            SceneCollisionSystem* collisionSystem = sceneEditor->GetSystem<SceneCollisionSystem>();
            SelectableGroup::CollectionType collObjects;
            collisionSystem->ObjectsRayTestFromCamera(collObjects);
            if (collObjects.size() == 1)
            {
                Entity* firstEntity = collObjects.front().AsEntity();
                if ((firstEntity != nullptr) && GetWaypointComponent(firstEntity) && GetPathComponent(firstEntity->GetParent()))
                {
                    underCursorPathEntity = GetWaypointComponent(firstEntity)->GetWaypoint();
                }
            }
        }
        else if (UIEvent::Phase::BEGAN == event->phase)
        {
            inCloneState = modifSystem->InCloneState();
        }
        else if (UIEvent::Phase::ENDED == event->phase)
        {
            bool cloneJustDone = false;
            if (inCloneState && !modifSystem->InCloneState())
            {
                inCloneState = false;
                cloneJustDone = true;
            }

            SelectionSystem* selectionSystem = sceneEditor->GetSystem<SelectionSystem>();
            ProcessSelection(selectionSystem->GetSelection());

            Keyboard* kb = GetEngineContext()->deviceManager->GetKeyboard();
            bool shiftPressed = (kb != nullptr) && (kb->GetKeyState(eInputElements::KB_LSHIFT).IsPressed() || kb->GetKeyState(eInputElements::KB_RSHIFT).IsPressed());

            if (!shiftPressed)
            {
                // we need to use shift key to add waypoint or edge
                return false;
            }

            PathSystem* pathSystem = sceneEditor->GetSystem<PathSystem>();
            Entity* currentWayParent = pathSystem->GetCurrrentPath();
            if (currentWayParent == nullptr)
            {
                // we need to have entity with path component
                return false;
            }

            PathComponent* pathComponent = GetPathComponent(currentWayParent);
            Vector<PathComponent::Waypoint*> validPrevPoints;
            FilterSelection(pathComponent, prevSelectedWaypoints, validPrevPoints);

            if (selectedWaypoints.empty() && cloneJustDone == false)
            {
                Vector3 lanscapeIntersectionPos;
                SceneCollisionSystem* collisionSystem = sceneEditor->GetSystem<SceneCollisionSystem>();
                bool lanscapeIntersected = collisionSystem->LandRayTestFromCamera(lanscapeIntersectionPos);

                // add new waypoint on the landscape
                if (lanscapeIntersected)
                {
                    if (validPrevPoints.empty())
                    {
                        if (pathComponent->GetPoints().size() > 0)
                        {
                            // current path has waypoints but none of them was selected. Point adding is denied
                            return false;
                        }
                    }

                    TransformComponent* tc = currentWayParent->GetComponent<TransformComponent>();
                    Matrix4 parentTransform = tc->GetWorldMatrix();
                    parentTransform.Inverse();

                    Matrix4 waypointTransform;
                    waypointTransform.SetTranslationVector(lanscapeIntersectionPos);
                    waypointTransform = waypointTransform * parentTransform;

                    int32 waypointsCount = static_cast<int32>(pathComponent->GetPoints().size());
                    PathComponent::Waypoint* waypoint = new PathComponent::Waypoint();
                    waypoint->name = FastName(Format("Waypoint_%d", waypointsCount));
                    waypoint->SetStarting(waypointsCount == 0);
                    waypoint->position = waypointTransform.GetTranslationVector();

                    bool prevIsLocked = selectionSystem->IsLocked();
                    selectionSystem->SetLocked(true);
                    sceneEditor->BeginBatch("Add Waypoint", static_cast<uint32>(1 + validPrevPoints.size()));
                    sceneEditor->Exec(std::make_unique<AddWaypointCommand>(sceneEditor, pathComponent, waypoint));
                    if (!validPrevPoints.empty())
                    {
                        AddEdges(pathComponent, validPrevPoints, waypoint);
                    }
                    sceneEditor->EndBatch();

                    selectedWaypoints.clear();
                    selectedWaypoints.push_back(waypoint);
                    selectionSystem->SetLocked(prevIsLocked);
                }
            }
            else if ((selectedWaypoints.size() == 1) && (cloneJustDone == false))
            {
                Vector<PathComponent::Waypoint*> filteredSelection;
                FilterSelection(pathComponent, selectedWaypoints, filteredSelection);
                if (filteredSelection.empty() == false)
                {
                    PathComponent::Waypoint* nextWaypoint = selectedWaypoints.front();
                    Vector<PathComponent::Waypoint*> srcPointToAddEdges;
                    Vector<PathComponent::Waypoint*> srcPointToRemoveEdges;
                    DefineAddOrRemoveEdges(validPrevPoints, nextWaypoint, srcPointToAddEdges, srcPointToRemoveEdges);
                    size_t totalOperations = srcPointToAddEdges.size() + srcPointToRemoveEdges.size();
                    if (totalOperations > 0)
                    {
                        sceneEditor->BeginBatch(Format("Add/Remove edges pointed on entity %s", nextWaypoint->name.c_str()), static_cast<uint32>(totalOperations));
                        AddEdges(pathComponent, srcPointToAddEdges, nextWaypoint);
                        RemoveEdges(pathComponent, srcPointToRemoveEdges, nextWaypoint);
                        sceneEditor->EndBatch();
                    }
                }
            }
        }
    }
    return false;
}

void WayEditSystem::FilterSelection(PathComponent* path, const Vector<PathComponent::Waypoint*>& srcPoints, Vector<PathComponent::Waypoint*>& validPoints)
{
    const Vector<PathComponent::Waypoint*>& points = path->GetPoints();
    Set<PathComponent::Waypoint*> pathPoints(points.begin(), points.end());

    validPoints.reserve(srcPoints.size());
    for (PathComponent::Waypoint* waypoint : srcPoints)
    {
        if (pathPoints.count(waypoint))
        {
            validPoints.push_back(waypoint);
        }
    }
}

void WayEditSystem::DefineAddOrRemoveEdges(const Vector<PathComponent::Waypoint*>& srcPoints, PathComponent::Waypoint* dstPoint,
                                           Vector<PathComponent::Waypoint*>& toAddEdge, Vector<PathComponent::Waypoint*>& toRemoveEdge)
{
    using namespace DAVA;
    for (PathComponent::Waypoint* srcPoint : srcPoints)
    {
        if (srcPoint == dstPoint)
        {
            continue;
        }

        bool edgeFound = false;
        for (PathComponent::Edge* edge : srcPoint->edges)
        {
            if (edge->destination == dstPoint)
            {
                edgeFound = true;
                break;
            }
        }

        if (edgeFound)
        {
            toRemoveEdge.push_back(srcPoint);
        }
        else
        {
            toAddEdge.push_back(srcPoint);
        }
    }
}

void WayEditSystem::AddEdges(PathComponent* path, const Vector<PathComponent::Waypoint*>& srcPoints, PathComponent::Waypoint* nextWaypoint)
{
    DVASSERT(path);
    DVASSERT(nextWaypoint);

    SceneEditor2* sceneEditor = GetSceneEditor();

    for (PathComponent::Waypoint* waypoint : srcPoints)
    {
        PathComponent::Edge* newEdge = new PathComponent::Edge();
        newEdge->destination = nextWaypoint;
        sceneEditor->Exec(std::unique_ptr<Command>(new AddEdgeCommand(sceneEditor, path, waypoint, newEdge)));
    }
}

void WayEditSystem::RemoveEdges(PathComponent* path, const Vector<PathComponent::Waypoint*>& srcPoints, PathComponent::Waypoint* nextWaypoint)
{
    using namespace DAVA;
    DVASSERT(path);
    DVASSERT(nextWaypoint);

    SceneEditor2* sceneEditor = GetSceneEditor();
    Vector<std::pair<PathComponent::Waypoint*, PathComponent::Edge*>> edgesToRemove;
    edgesToRemove.reserve(srcPoints.size());

    for (PathComponent::Waypoint* srcPoint : srcPoints)
    {
        for (PathComponent::Edge* edge : srcPoint->edges)
        {
            if (edge->destination == nextWaypoint)
            {
                edgesToRemove.push_back(std::make_pair(srcPoint, edge));
            }
        }
    }

    for (auto& node : edgesToRemove)
    {
        WayEditSystemDetail::AccessibleQueryParams params;
        params.startPoint = path->GetStartWaypoint();
        params.destinationPoint = node.second->destination;
        params.excludeEdge = node.second;
        if (WayEditSystemDetail::IsAccessible(params) == true)
        {
            sceneEditor->Exec(std::make_unique<RemoveEdgeCommand>(sceneEditor, path, node.first, node.second));
        }
    }
}

void WayEditSystem::ProcessCommand(const RECommandNotificationObject& commandNotification)
{
    if (commandNotification.MatchCommandTypes<EnableWayEditCommand>())
    {
        DVASSERT(commandNotification.MatchCommandTypes<DisableWayEditCommand>() == false);
        EnableWayEdit(commandNotification.IsRedo());
    }
    else if (commandNotification.MatchCommandTypes<DisableWayEditCommand>())
    {
        DVASSERT(commandNotification.MatchCommandTypes<EnableWayEditCommand>() == false);
        EnableWayEdit(!commandNotification.IsRedo());
    }
}

void WayEditSystem::Draw()
{
    SceneEditor2* editorScene = static_cast<SceneEditor2*>(GetScene());

    Set<PathComponent::Waypoint*> selected;
    if (currentSelection.IsEmpty())
    {
        selected.insert(selectedWaypoints.begin(), selectedWaypoints.end());
    }
    else
    {
        for (auto item : currentSelection.ObjectsOfType<Entity>())
        {
            WaypointComponent* waypointComponent = GetWaypointComponent(item);
            if (waypointComponent != nullptr)
            {
                selected.insert(waypointComponent->GetWaypoint());
            }
        }
    }

    const uint32 count = static_cast<uint32>(waypointEntities.size());
    SceneCollisionSystem* collisionSystem = editorScene->GetSystem<SceneCollisionSystem>();
    for (uint32 i = 0; i < count; ++i)
    {
        Entity* e = waypointEntities[i];
        Entity* path = e->GetParent();
        DVASSERT(path);

        if (!e->GetVisible() || !path->GetVisible())
        {
            continue;
        }

        WaypointComponent* wpComponent = GetWaypointComponent(e);
        DVASSERT(wpComponent);
        PathComponent::Waypoint* waypoint = wpComponent->GetWaypoint();

        float32 redValue = 0.0f;
        float32 greenValue = 0.0f;
        float32 blueValue = wpComponent->IsStartingPoint() ? 1.0f : 0.0f;

        if (wpComponent->GetWaypoint() == underCursorPathEntity)
        {
            redValue = 0.6f;
            greenValue = 0.6f;
        }
        else if (selected.count(waypoint))
        {
            redValue = 1.0f;
        }
        else
        {
            greenValue = 1.0f;
        }

        AABBox3 localBox = collisionSystem->GetUntransformedBoundingBox(e);
        // localBox.IsEmpty() == true, means that "e" was added into system on this frame
        // and collision has not been calculated yet. We will draw this entity on next frame
        if (localBox.IsEmpty() == false)
        {
            TransformComponent* tc = e->GetComponent<TransformComponent>();
            editorScene->GetRenderSystem()->GetDebugDrawer()->DrawAABoxTransformed(localBox, tc->GetWorldMatrix(), Color(redValue, greenValue, blueValue, 0.3f), RenderHelper::DRAW_SOLID_DEPTH);
            editorScene->GetRenderSystem()->GetDebugDrawer()->DrawAABoxTransformed(localBox, tc->GetWorldMatrix(), Color(redValue, greenValue, blueValue, 1.0f), RenderHelper::DRAW_WIRE_DEPTH);
        }
    }
}

void WayEditSystem::EnableWayEdit(bool enable)
{
    ResetSelection();

    isEnabled = enable;
    UpdateSelectionMask();
}

bool WayEditSystem::IsWayEditEnabled() const
{
    return isEnabled;
}

void WayEditSystem::UpdateSelectionMask()
{
    SelectionSystem* selectionSystem = GetScene()->GetSystem<SelectionSystem>();

    if (isEnabled)
    {
        selectionSystem->SetSelectionComponentMask(ComponentUtils::MakeMask<WaypointComponent>() | ComponentUtils::MakeMask<PathComponent>());
    }
    else
    {
        selectionSystem->ResetSelectionComponentMask();
    }
}

bool WayEditSystem::HasCustomClonedAddading(Entity* entityToClone) const
{
    if (isEnabled == true && GetWaypointComponent(entityToClone))
    {
        return true;
    }

    return false;
}

void WayEditSystem::PerformAdding(Entity* sourceEntity, Entity* clonedEntity)
{
    using namespace DAVA;

    DVASSERT(isEnabled == true);
    WaypointComponent* sourceComponent = GetWaypointComponent(sourceEntity);
    WaypointComponent* clonedComponent = GetWaypointComponent(clonedEntity);
    DVASSERT(sourceComponent);
    DVASSERT(clonedComponent);

    PathComponent::Waypoint* sourceWayPoint = sourceComponent->GetWaypoint();
    DVASSERT(sourceWayPoint != nullptr);

    TransformComponent* sourceTC = sourceEntity->GetComponent<TransformComponent>();
    sourceWayPoint->position = sourceTC->GetLocalTransform().GetTranslation();

    PathComponent::Waypoint* clonedWayPoint = clonedComponent->GetWaypoint();
    DVASSERT(clonedWayPoint == sourceWayPoint);

    PathComponent* path = sourceComponent->GetPath();

    clonedWayPoint = new PathComponent::Waypoint();
    clonedWayPoint->name = sourceWayPoint->name;
    TransformComponent* clonedTC = clonedEntity->GetComponent<TransformComponent>();
    clonedWayPoint->position = clonedTC->GetLocalTransform().GetTranslation();
    KeyedArchive* propertiesCopy = new KeyedArchive(*sourceWayPoint->GetProperties());
    clonedWayPoint->SetProperties(propertiesCopy);
    SafeRelease(propertiesCopy);

    clonedComponent->Init(path, clonedWayPoint);
    clonedEntity->SetNotRemovable(false);
    sourceEntity->GetParent()->AddNode(clonedEntity);

    UnorderedMap<PathComponent::Edge*, EdgeComponent*> edgeMap;
    for (uint32 i = 0; i < clonedEntity->GetComponentCount<EdgeComponent>(); ++i)
    {
        EdgeComponent* edgeComponent = static_cast<EdgeComponent*>(clonedEntity->GetComponent<EdgeComponent>(i));
        edgeMap[edgeComponent->GetEdge()] = edgeComponent;
    }

    // add new waypoint
    SceneEditor2* sceneEditor = GetSceneEditor();
    sceneEditor->Exec(std::make_unique<AddWaypointCommand>(sceneEditor, path, clonedWayPoint));

    // add copy of edges from source point to cloned
    for (PathComponent::Edge* edge : sourceWayPoint->edges)
    {
        PathComponent::Edge* newEdge = new PathComponent::Edge();
        newEdge->destination = edge->destination;
        auto edgeMappingIter = edgeMap.find(edge);
        DVASSERT(edgeMappingIter != edgeMap.end());
        edgeMappingIter->second->Init(path, newEdge);
        sceneEditor->Exec(std::make_unique<AddEdgeCommand>(sceneEditor, path, clonedWayPoint, newEdge));
    }

    // add edge from source point to cloned
    PathComponent::Edge* newEdge = new PathComponent::Edge();
    newEdge->destination = clonedWayPoint;
    sceneEditor->Exec(std::make_unique<AddEdgeCommand>(sceneEditor, path, sourceWayPoint, newEdge));
}

bool WayEditSystem::AllowPerformSelectionHavingCurrent(const SelectableGroup& currentSelection)
{
    Keyboard* kb = GetEngineContext()->deviceManager->GetKeyboard();
    bool shiftPressed = (kb != nullptr) && (kb->GetKeyState(eInputElements::KB_LSHIFT).IsPressed() || kb->GetKeyState(eInputElements::KB_RSHIFT).IsPressed());

    if (isEnabled && shiftPressed)
    {
        return (selectedWaypoints.size() > 0);
    }

    return true;
}

bool WayEditSystem::AllowChangeSelectionReplacingCurrent(const SelectableGroup& currentSelection, const SelectableGroup& newSelection)
{
    Engine* engine = Engine::Instance();
    DVASSERT(engine != nullptr);
    const EngineContext* engineContext = engine->GetContext();
    if (engineContext->inputSystem == nullptr)
    {
        return true;
    }

    Keyboard* kb = GetEngineContext()->deviceManager->GetKeyboard();
    bool shiftPressed = (kb != nullptr) && (kb->GetKeyState(eInputElements::KB_LSHIFT).IsPressed() || kb->GetKeyState(eInputElements::KB_RSHIFT).IsPressed());

    if (isEnabled && shiftPressed)
    {
        // no waypoints selected or no new objects are selected
        // will attempt to create new waypoint in input handler
        if (newSelection.IsEmpty())
            return true;

        // do not allow multi selection here
        if (newSelection.GetSize() > 1)
        {
            return false;
        }

        // only allow to select waypoints in this mode
        auto entity = newSelection.GetFirst().AsEntity();
        if ((entity == nullptr) || (GetWaypointComponent(entity) == nullptr))
        {
            return false;
        }

        // only allow to select waypoints withing same path
        PathSystem* pathSystem = GetScene()->GetSystem<PathSystem>();
        if (entity->GetParent() != pathSystem->GetCurrrentPath())
        {
            return false;
        }
    }

    return true;
}

SceneEditor2* WayEditSystem::GetSceneEditor() const
{
    return static_cast<SceneEditor2*>(GetScene());
}

} // namespace DAVA
