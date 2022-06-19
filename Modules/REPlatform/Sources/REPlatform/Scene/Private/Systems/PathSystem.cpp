#include "REPlatform/Scene/Systems/PathSystem.h"
#include "REPlatform/Scene/Systems/SelectionSystem.h"
#include "REPlatform/Scene/SceneEditor2.h"

#include "REPlatform/DataNodes/Settings/GlobalSceneSettings.h"

#include "REPlatform/Commands/RECommandNotificationObject.h"
#include "REPlatform/Commands/InspMemberModifyCommand.h"
#include "REPlatform/Commands/SetFieldValueCommand.h"
#include "REPlatform/Commands/TransformCommand.h"
#include "REPlatform/Commands/WayEditCommands.h"

#include <TArc/Core/Deprecated.h>

#include <FileSystem/KeyedArchive.h>
#include <Math/Matrix4.h>
#include <Scene3D/Components/ComponentHelpers.h>
#include <Scene3D/Components/TransformComponent.h>
#include <Scene3D/Components/Waypoint/EdgeComponent.h>
#include <Scene3D/Components/Waypoint/PathComponent.h>
#include <Scene3D/Components/Waypoint/WaypointComponent.h>
#include <Scene3D/Entity.h>
#include <Utils/Utils.h>

namespace DAVA
{
namespace PathSystemDetail
{
const Array<Color, 16> PathColorPallete =
{ { Color(0x00ffffff),
    Color(0x000000ff),
    Color(0x0000ffff),
    Color(0xff00ffff),

    Color(0x808080ff),
    Color(0x008000ff),
    Color(0x00ff00ff),
    Color(0x80000ff),

    Color(0x000080ff),
    Color(0x808000ff),
    Color(0x800080ff),
    Color(0xff0000ff),

    Color(0xc0c0c0ff),
    Color(0x008080ff),
    Color(0xffffffff),
    Color(0xffff00ff) } };

const String PATH_COLOR_PROP_NAME = "pathColor";

struct WaypointKey
{
    PathComponent* path = nullptr;
    PathComponent::Waypoint* waypoint = nullptr;

    bool operator==(const WaypointKey& other) const
    {
        return path == other.path && waypoint == other.waypoint;
    }
};

class WaypointHash
{
public:
    size_t operator()(const WaypointKey& s) const
    {
        size_t h1 = std::hash<const PathComponent*>()(s.path);
        size_t h2 = std::hash<const PathComponent::Waypoint*>()(s.waypoint);
        return h1 ^ (h2 << 1);
    }
};

struct EdgeKey
{
    PathComponent* path = nullptr;
    PathComponent::Edge* edge = nullptr;

    bool operator==(const EdgeKey& other) const
    {
        return path == other.path && edge == other.edge;
    }
};

class EdgeHash
{
public:
    size_t operator()(const EdgeKey& s) const
    {
        size_t h1 = std::hash<const PathComponent*>()(s.path);
        size_t h2 = std::hash<const PathComponent::Edge*>()(s.edge);
        return h1 ^ (h2 << 1);
    }
};

struct MappingValue
{
    RefPtr<Entity> entity;
    bool existsInSourceData = false;
};

struct EdgeMappingValue : MappingValue
{
    EdgeComponent* component = nullptr;
};
}

PathSystem::PathSystem(Scene* scene)
    : SceneSystem(scene)
    , currentPath(NULL)
    , isEditingEnabled(false)
{
}

PathSystem::~PathSystem()
{
    currentPath = NULL;

    pathes.clear();
    currentSelection.Clear();

    for (const auto& iter : edgeComponentCache)
    {
        delete iter.second;
    }
}

void PathSystem::AddEntity(Entity* entity)
{
    if (isEditingEnabled == false && !currentPath)
    {
        currentPath = entity;
    }

    // extract color data from custom properties for old scenes
    PathComponent* pc = GetPathComponent(entity);
    DVASSERT(pc != nullptr);
    if (pc->GetColor() == Color())
    {
        KeyedArchive* props = GetCustomPropertiesArchieve(entity);
        if (props && props->IsKeyExists(PathSystemDetail::PATH_COLOR_PROP_NAME))
        {
            pc->SetColor(Color(props->GetVector4(PathSystemDetail::PATH_COLOR_PROP_NAME)));
            props->DeleteKey(PathSystemDetail::PATH_COLOR_PROP_NAME);
        }
    }

    if (isEditingEnabled == true)
    {
        entitiesForExpand.insert(entity);
    }

    InitPathComponent(pc);
    pathes.push_back(entity);
}

void PathSystem::RemoveEntity(Entity* entity)
{
    PathComponent* pc = GetPathComponent(entity);
    if (isEditingEnabled == true && pc != nullptr)
    {
        entitiesForCollapse.emplace(entity, pc->GetName());
    }

    FindAndRemoveExchangingWithLast(pathes, entity);

    if (pathes.size())
    {
        if (entity == currentPath)
        {
            currentPath = pathes[0];
        }
    }
    else
    {
        currentPath = nullptr;
    }
}

void PathSystem::PrepareForRemove()
{
    pathes.clear();
    currentPath = nullptr;
    entitiesForCollapse.clear();
}

void PathSystem::WillClone(Entity* originalEntity)
{
    PathComponent* pc = GetPathComponent(originalEntity);
    if (isEditingEnabled && pc != nullptr)
    {
        CollapsePathEntity(originalEntity, pc->GetName());
    }
}

void PathSystem::DidCloned(Entity* originalEntity, Entity* newEntity)
{
    if (isEditingEnabled)
    {
        if (GetPathComponent(originalEntity) != nullptr)
        {
            entitiesForExpand.insert(originalEntity);
        }

        if (GetPathComponent(newEntity) != nullptr)
        {
            entitiesForExpand.insert(newEntity);
        }
    }
}

void PathSystem::OnWaypointAdded(PathComponent* path, PathComponent::Waypoint* waypoint)
{
    entitiesForExpand.insert(path->GetEntity());
}

void PathSystem::OnWaypointRemoved(PathComponent* path, PathComponent::Waypoint* waypoint)
{
    entitiesForExpand.insert(path->GetEntity());
}

void PathSystem::OnEdgeAdded(PathComponent* path, PathComponent::Waypoint* waypoint, PathComponent::Edge* edge)
{
    entitiesForExpand.insert(path->GetEntity());
}

void PathSystem::OnEdgeRemoved(PathComponent* path, PathComponent::Waypoint* waypoint, PathComponent::Edge* edge)
{
    entitiesForExpand.insert(path->GetEntity());
}

void PathSystem::OnWaypointDeleted(PathComponent* path, PathComponent::Waypoint* waypoint)
{
    size_t erasedCount = entityCache.erase(waypoint);
    DVASSERT(erasedCount > 0);
}

void PathSystem::OnEdgeDeleted(PathComponent* path, PathComponent::Waypoint* waypoint, PathComponent::Edge* edge)
{
    auto iter = edgeComponentCache.find(edge);
    if (iter != edgeComponentCache.end())
    {
        SafeDelete(iter->second);
        edgeComponentCache.erase(iter);
    }
}

void PathSystem::BakeWaypoints()
{
    SceneEditor2* scene = static_cast<SceneEditor2*>(GetScene());
    scene->BeginBatch("Bake waypoints");

    for (Entity* path : pathes)
    {
        TransformComponent* pathTC = path->GetComponent<TransformComponent>();
        const Transform& worldTransform = pathTC->GetWorldTransform();

        PathComponent* component = path->GetComponent<PathComponent>();
        for (PathComponent::Waypoint* point : component->GetPoints())
        {
            Vector3 newPosition = point->position * worldTransform;
            Reflection::Field field;
            field.ref = Reflection::Create(point).GetField("waypointPosition");
            scene->Exec(std::make_unique<SetFieldValueCommand>(field, newPosition));
        }

        for (Entity* parentEntity = path; parentEntity != nullptr && parentEntity != scene; parentEntity = parentEntity->GetParent())
        {
            TransformComponent* parentTC = parentEntity->GetComponent<TransformComponent>();
            scene->Exec(std::make_unique<TransformCommand>(Selectable(Any(parentEntity)), parentTC->GetLocalTransform(), Transform()));
        }
    }

    scene->EndBatch();
}

void PathSystem::Draw()
{
    const uint32 count = static_cast<uint32>(pathes.size());
    if (!count)
        return;

    if (isEditingEnabled)
    {
        DrawInEditableMode();
    }
    else
    {
        DrawInViewOnlyMode();
    }
}

void PathSystem::DrawInEditableMode()
{
    for (Entity* path : pathes)
    {
        PathComponent* pc = GetPathComponent(path);
        if (!path->GetVisible() || !pc)
        {
            continue;
        }

        const uint32 childrenCount = path->GetChildrenCount();
        for (uint32 c = 0; c < childrenCount; ++c)
        {
            Entity* waypoint = path->GetChild(c);

            const uint32 edgesCount = waypoint->GetComponentCount<EdgeComponent>();
            if (edgesCount)
            {
                TransformComponent* wayTC = waypoint->GetComponent<TransformComponent>();
                Vector3 startPosition = wayTC->GetWorldTransform().GetTranslation();
                startPosition.z += WAYPOINTS_DRAW_LIFTING;
                for (uint32 e = 0; e < edgesCount; ++e)
                {
                    EdgeComponent* edge = waypoint->GetComponent<EdgeComponent>(e);
                    Entity* nextEntity = edge->GetNextEntity();
                    if (nextEntity && nextEntity->GetParent())
                    {
                        TransformComponent* nextTC = nextEntity->GetComponent<TransformComponent>();

                        Vector3 finishPosition = nextTC->GetWorldTransform().GetTranslation();
                        finishPosition.z += WAYPOINTS_DRAW_LIFTING;
                        DrawArrow(startPosition, finishPosition, pc->GetColor());
                    }
                }
            }
        }
    }
}

void PathSystem::DrawInViewOnlyMode()
{
    GlobalSceneSettings* settings = Deprecated::GetDataNode<GlobalSceneSettings>();

    const float32 boxScale = settings->debugBoxWaypointScale;

    Scene* scene = GetScene();
    const SelectableGroup& selection = scene->GetSystem<SelectionSystem>()->GetSelection();
    for (Entity* entity : selection.ObjectsOfType<Entity>())
    {
        PathComponent* pathComponent = GetPathComponent(entity);
        if (entity->GetVisible() == false || !pathComponent)
        {
            continue;
        }

        const Vector<PathComponent::Waypoint*>& waypoints = pathComponent->GetPoints();
        for (PathComponent::Waypoint* waypoint : waypoints)
        {
            Vector3 startPosition = waypoint->position;
            const AABBox3 wpBoundingBox(startPosition, boxScale);

            TransformComponent* tc = entity->GetComponent<TransformComponent>();
            const Matrix4& transform = tc->GetWorldMatrix();
            bool isStarting = waypoint->IsStarting();

            scene->GetRenderSystem()->GetDebugDrawer()->DrawAABoxTransformed(wpBoundingBox, transform, Color(0.3f, 0.3f, isStarting ? 1.0f : 0.0f, 0.3f), RenderHelper::DRAW_SOLID_DEPTH);
            scene->GetRenderSystem()->GetDebugDrawer()->DrawAABoxTransformed(wpBoundingBox, transform, Color(0.7f, 0.7f, isStarting ? 0.7f : 0.0f, 1.0f), RenderHelper::DRAW_WIRE_DEPTH);

            //draw edges
            if (!waypoint->edges.empty())
            {
                startPosition.z += WAYPOINTS_DRAW_LIFTING;
                for (auto edge : waypoint->edges)
                {
                    Vector3 finishPosition = edge->destination->position;
                    finishPosition.z += WAYPOINTS_DRAW_LIFTING;
                    DrawArrow(startPosition * transform, finishPosition * transform, pathComponent->GetColor());
                }
            }
        }
    }
}

void PathSystem::DrawArrow(const Vector3& start, const Vector3& finish, const Color& color)
{
    GetScene()->GetRenderSystem()->GetDebugDrawer()->DrawArrow(start, finish, Min((finish - start).Length() / 4.f, 4.f), ClampToUnityRange(color), RenderHelper::DRAW_SOLID_DEPTH);
}

void PathSystem::InitPathComponent(PathComponent* component)
{
    if (component->GetName().IsValid() == false)
    {
        component->SetName(GeneratePathName());
    }

    if (component->GetColor() == Color())
    {
        component->SetColor(GetNextPathColor());
    }

    const Vector<PathComponent::Waypoint*>& points = component->GetPoints();
    for (PathComponent::Waypoint* waypoint : points)
    {
        if (waypoint->GetProperties() == nullptr)
        {
            RefPtr<KeyedArchive> props(new KeyedArchive());
            waypoint->SetProperties(props.Get());
        }

        for (PathComponent::Edge* edge : waypoint->edges)
        {
            if (edge->GetProperties() == nullptr)
            {
                RefPtr<KeyedArchive> props(new KeyedArchive());
                edge->SetProperties(props.Get());
            }
        }
    }
}

void PathSystem::Process(float32 timeElapsed)
{
    for (const auto& node : entitiesForCollapse)
    {
        CollapsePathEntity(node.first, node.second);
    }
    entitiesForCollapse.clear();

    for (Entity* entityToExpand : entitiesForExpand)
    {
        ExpandPathEntity(entityToExpand);
    }
    entitiesForExpand.clear();

    if (isEditingEnabled == false)
    {
        return;
    }

    const SelectableGroup& selection = GetScene()->GetSystem<SelectionSystem>()->GetSelection();
    if (currentSelection != selection)
    {
        currentSelection.Clear();
        currentSelection.Join(selection);

        for (auto entity : currentSelection.ObjectsOfType<Entity>())
        {
            if (GetPathComponent(entity) != nullptr)
            {
                currentPath = entity;
                break;
            }

            if (GetWaypointComponent(entity) && GetPathComponent(entity->GetParent()))
            {
                currentPath = entity->GetParent();
                break;
            }
        }
    }
}

FastName PathSystem::GeneratePathName() const
{
    const uint32 count = static_cast<uint32>(pathes.size());
    for (uint32 i = 0; i <= count; ++i)
    {
        FastName generatedName(Format("path_%02d", i));

        bool found = false;

        for (uint32 p = 0; p < count; ++p)
        {
            const PathComponent* pc = GetPathComponent(pathes[p]);
            if (generatedName == pc->GetName())
            {
                found = true;
                break;
            }
        }

        if (!found)
            return generatedName;
    }

    return FastName();
}

const Color& PathSystem::GetNextPathColor() const
{
    const uint32 count = static_cast<uint32>(pathes.size());
    const uint32 index = count % static_cast<uint32>(PathSystemDetail::PathColorPallete.size());

    return PathSystemDetail::PathColorPallete[index];
}

void PathSystem::EnablePathEdit(bool enable)
{
    DVASSERT(isEditingEnabled != enable);
    isEditingEnabled = enable;
    if (enable)
    {
        for (Entity* pathEntity : pathes)
        {
            entitiesForExpand.insert(pathEntity);
        }
    }
    else
    {
        for (Entity* pathEntity : pathes)
        {
            PathComponent* path = GetPathComponent(pathEntity);
            DVASSERT(path != nullptr);
            entitiesForCollapse.emplace(pathEntity, path->GetName());
        }
    }
}

void PathSystem::ExpandPathEntity(Entity* pathEntity)
{
    using namespace DAVA;
    using namespace PathSystemDetail;

    UnorderedMap<WaypointKey, MappingValue, WaypointHash> waypointToEntity;
    UnorderedMap<EdgeKey, EdgeMappingValue, EdgeHash> edgeToEntity;

    auto lookUpWaypointEntity = [this, &waypointToEntity, pathEntity](PathComponent* path, PathComponent::Waypoint* waypoint) -> RefPtr<Entity>
    {
        WaypointKey key;
        key.path = path;
        key.waypoint = waypoint;

        MappingValue& value = waypointToEntity[key];
        value.existsInSourceData = true;
        if (value.entity.Get() == nullptr)
        {
            auto iter = entityCache.find(waypoint);
            if (iter != entityCache.end())
            {
                value.entity = iter->second;
                WaypointComponent* component = GetWaypointComponent(value.entity.Get());
                DVASSERT(component != nullptr);
                DVASSERT(component->GetPath() == path);
                DVASSERT(component->GetWaypoint() == waypoint);
            }
            else
            {
                value.entity.ConstructInplace();
                value.entity->SetName(waypoint->name);
                if (waypoint->IsStarting())
                {
                    value.entity->SetNotRemovable(true);
                }

                WaypointComponent* wpComponent = new WaypointComponent();
                wpComponent->Init(path, waypoint);
                value.entity->AddComponent(wpComponent);

                entityCache.emplace(waypoint, value.entity);
            }

            TransformComponent* tc = value.entity->GetComponent<TransformComponent>();
            tc->SetLocalTranslation(waypoint->position);
            pathEntity->AddNode(value.entity.Get());
        }

        return value.entity;
    };

    for (int32 i = 0; i < pathEntity->GetChildrenCount(); ++i)
    {
        Entity* child = pathEntity->GetChild(i);
        WaypointComponent* component = GetWaypointComponent(child);
        if (component == nullptr)
        {
            continue;
        }

        WaypointKey key;
        key.path = component->GetPath();
        key.waypoint = component->GetWaypoint();

        MappingValue value;
        value.entity = RefPtr<Entity>::ConstructWithRetain(child);

        waypointToEntity.emplace(key, value);
        if (entityCache.count(key.waypoint) == 0)
        {
            entityCache[key.waypoint] = value.entity;
        }

        for (uint32 edgeIndex = 0; edgeIndex < child->GetComponentCount<EdgeComponent>(); ++edgeIndex)
        {
            EdgeComponent* component = child->GetComponent<EdgeComponent>(edgeIndex);

            EdgeKey key;
            key.path = component->GetPath();
            key.edge = component->GetEdge();

            EdgeMappingValue value;
            value.entity = RefPtr<Entity>::ConstructWithRetain(child);
            value.component = component;

            edgeToEntity.emplace(key, value);
        }
    }

    PathComponent* path = GetPathComponent(pathEntity);
    DVASSERT(path != nullptr);
    for (PathComponent::Waypoint* waypoint : path->GetPoints())
    {
        lookUpWaypointEntity(path, waypoint);

        for (PathComponent::Edge* edge : waypoint->edges)
        {
            EdgeKey edgeKey;
            edgeKey.path = path;
            edgeKey.edge = edge;

            EdgeMappingValue& value = edgeToEntity[edgeKey];
            value.existsInSourceData = true;
            if (value.component == nullptr)
            {
                RefPtr<Entity> srcWaypointEntity = lookUpWaypointEntity(path, waypoint);
                value.entity = srcWaypointEntity;
                auto iter = edgeComponentCache.find(edge);
                if (iter != edgeComponentCache.end())
                {
                    value.component = iter->second;
                    DVASSERT(value.component->GetPath() == path);
                    DVASSERT(value.component->GetEdge() == edge);
                    edgeComponentCache.erase(edge);
                }
                else
                {
                    value.component = new EdgeComponent();
                    value.component->Init(path, edge);
                }
                value.entity->AddComponent(value.component);
            }

            RefPtr<Entity> destinationEntity = lookUpWaypointEntity(path, edge->destination);
            value.component->SetNextEntity(destinationEntity.Get());
        }
    }

    Set<RefPtr<Entity>> entitiesToRemove;
    for (const auto& node : waypointToEntity)
    {
        if (node.second.existsInSourceData == false)
        {
            DVASSERT(entityCache.count(node.first.waypoint) > 0);
            entitiesToRemove.insert(node.second.entity);
        }
    }
    waypointToEntity.clear();

    for (const auto& node : edgeToEntity)
    {
        if (node.second.existsInSourceData == false)
        {
            node.second.component->SetNextEntity(nullptr);
            node.second.entity->DetachComponent(node.second.component);
            edgeComponentCache.emplace(node.first.edge, node.second.component);
        }
    }
    edgeToEntity.clear();

    for (const RefPtr<Entity>& e : entitiesToRemove)
    {
        pathEntity->RemoveNode(e.Get());
    }
    entitiesToRemove.clear();
}

void PathSystem::CollapsePathEntity(Entity* pathEntity, FastName pathName)
{
    Vector<Entity*> edgeChildren;
    pathEntity->GetChildEntitiesWithComponent(edgeChildren, Type::Instance<EdgeComponent>());
    for (Entity* edgeEntity : edgeChildren)
    {
        uint32 count = edgeEntity->GetComponentCount<EdgeComponent>();
        Vector<EdgeComponent*> edgeComponents;
        edgeComponents.reserve(count);
        for (uint32 i = 0; i < count; ++i)
        {
            EdgeComponent* edgeComponent = edgeEntity->GetComponent<EdgeComponent>(i);
            edgeComponents.push_back(edgeComponent);
            DVASSERT(edgeComponent->GetEdge() != nullptr);
            bool added = edgeComponentCache.emplace(edgeComponent->GetEdge(), edgeComponent).second;
            DVASSERT(added == true);
        }

        for (EdgeComponent* edgeComponent : edgeComponents)
        {
            edgeEntity->DetachComponent(edgeComponent);
        }
    }

    Vector<Entity*> children;
    pathEntity->GetChildEntitiesWithComponent(children, Type::Instance<WaypointComponent>());
    for (Entity* wpEntity : children)
    {
        WaypointComponent* wpComponent = GetWaypointComponent(wpEntity);
        if (pathName == wpComponent->GetPathName())
        {
            DVASSERT(entityCache.count(wpComponent->GetWaypoint()) > 0);
            pathEntity->RemoveNode(wpEntity);
        }
    }
}
} // namespace DAVA
