#include "REPlatform/Scene/Systems/DebugDrawSystem.h"
#include "REPlatform/Scene/Components/CollisionTypeComponent.h"
#include "REPlatform/Scene/Systems/BeastSystem.h"
#include "REPlatform/Scene/Systems/CollisionSystem.h"
#include "REPlatform/Scene/Systems/LandscapeEditorDrawSystem.h"
#include "REPlatform/Scene/Systems/LandscapeProxy.h"
#include "REPlatform/Scene/Systems/SelectionSystem.h"
#include "REPlatform/Scene/Systems/TextDrawSystem.h"

#include "REPlatform/Commands/RECommandBatch.h"
#include "REPlatform/Commands/KeyedArchiveCommand.h"

#include "REPlatform/DataNodes/ProjectManagerData.h"
#include "REPlatform/DataNodes/SelectableGroup.h"
#include "REPlatform/DataNodes/Settings/GlobalSceneSettings.h"
#include "REPlatform/Deprecated/EditorConfig.h"
#include "REPlatform/Deprecated/SceneValidator.h"

#include <TArc/Core/Deprecated.h>

#include <Base/Type.h>
#include <Engine/Engine.h>
#include <Entity/ComponentManager.h>
#include <Functional/Function.h>
#include <Render/Highlevel/GeometryOctTree.h>
#include <Scene3D/Components/ComponentHelpers.h>
#include <Scene3D/Components/GeoDecalComponent.h>

#define DAVA_ALLOW_OCTREE_DEBUG 0

namespace DAVA
{
const float32 DebugDrawSystem::HANGING_OBJECTS_DEFAULT_HEIGHT = 0.001f;

DebugDrawSystem::DebugDrawSystem(Scene* scene)
    : SceneSystem(scene)
{
    drawComponentFunctionsMap[Type::Instance<SoundComponent>()] = MakeFunction(this, &DebugDrawSystem::DrawSoundNode);
    drawComponentFunctionsMap[Type::Instance<WindComponent>()] = MakeFunction(this, &DebugDrawSystem::DrawWindNode);
    drawComponentFunctionsMap[Type::Instance<GeoDecalComponent>()] = MakeFunction(this, &DebugDrawSystem::DrawDecals);
    drawComponentFunctionsMap[Type::Instance<LightComponent>()] = Bind(&DebugDrawSystem::DrawLightNode, this, DAVA::_1, false);
    drawComponentFunctionsMap[Type::Instance<CollisionTypeComponent>()] = MakeFunction(this, &DebugDrawSystem::DrawCollisionTypeBox);
}

DebugDrawSystem::~DebugDrawSystem()
{
}

void DebugDrawSystem::SetCollisionType(int32 _collisionType)
{
    collisionType = _collisionType;

    if (collisionType != CollisionTypeValues::COLLISION_TYPE_OFF)
    {
        ProjectManagerData* data = Deprecated::GetDataNode<ProjectManagerData>();
        DVASSERT(data);
        const Vector<Color>& colors = data->GetEditorConfig()->GetColorPropertyValues("CollisionTypeColor");
        if (collisionType >= 0 && collisionType < static_cast<int32>(colors.size()))
        {
            collisionTypeColor = colors[collisionType];
        }
        else
        {
            collisionTypeColor = Color(1.f, 0, 0, 1.f);
        }
    }
}

int32 DebugDrawSystem::GetCollisionType() const
{
    return collisionType;
}

void DebugDrawSystem::AddEntity(Entity* entity)
{
    entities.push_back(entity);

    ComponentManager* cm = GetEngineContext()->componentManager;
    const Vector<const Type*> componentTypes = cm->GetRegisteredSceneComponents();

    AddCollisionTypeComponent(entity);

    for (const Type* type : componentTypes)
    {
        for (uint32 index = 0, count = entity->GetComponentCount(type); index < count; ++index)
        {
            Component* component = entity->GetComponent(type, index);
            RegisterComponent(entity, component);
        }
    }
}

void DebugDrawSystem::AddCollisionTypeComponent(Entity* entity) const
{
    KeyedArchive* customProperties = GetCustomPropertiesArchieve(entity);
    if (customProperties == nullptr)
        return;

    if (customProperties->IsKeyExists("CollisionType") == false
        && customProperties->IsKeyExists("CollisionTypeCrashed") == false)
    {
        return;
    }

    CollisionTypeComponent* comp = new CollisionTypeComponent();
    int32 type = customProperties->GetInt32("CollisionType", CollisionTypeValues::COLLISION_TYPE_UNDEFINED);
    int32 typeCrashed = customProperties->GetInt32("CollisionTypeCrashed", CollisionTypeValues::COLLISION_TYPE_UNDEFINED);
    comp->SetCollisionType(type);
    comp->SetCollisionTypeCrashed(typeCrashed);

    entity->AddComponent(comp);

    customProperties->DeleteKey("CollisionType");
    customProperties->DeleteKey("CollisionTypeCrashed");
}

void DebugDrawSystem::RemoveEntity(Entity* entity)
{
    FindAndRemoveExchangingWithLast(entities, entity);

    for (auto& it : entitiesComponentMap)
    {
        FindAndRemoveExchangingWithLast(it.second, entity);
    }
}

void DebugDrawSystem::RegisterComponent(Entity* entity, Component* component)
{
    const Type* type = component->GetType();

    auto it = drawComponentFunctionsMap.find(type);

    if (it != drawComponentFunctionsMap.end())
    {
        Vector<Entity*>& array = entitiesComponentMap[type];

        auto it = find_if(array.begin(), array.end(), [entity](const Entity* obj) { return entity == obj; });

        if (it == array.end())
        {
            array.push_back(entity);
        }
    }
}

void DebugDrawSystem::UnregisterComponent(Entity* entity, Component* component)
{
    const Type* type = component->GetType();

    auto it = entitiesComponentMap.find(type);

    if (it != entitiesComponentMap.end() && entity->GetComponentCount(type) == 1)
    {
        FindAndRemoveExchangingWithLast(it->second, entity);
    }
}

void DebugDrawSystem::PrepareForRemove()
{
    entities.clear();
    entitiesComponentMap.clear();
}

void DebugDrawSystem::DrawComponent(const Type* type, const Function<void(Entity*)>& func)
{
    auto it = entitiesComponentMap.find(type);

    if (it != entitiesComponentMap.end())
    {
        Vector<Entity*>& array = it->second;

        for (Entity* entity : array)
        {
            func(entity);
        }
    }
}

void DebugDrawSystem::Draw()
{
    for (auto& it : drawComponentFunctionsMap)
    {
        DrawComponent(it.first, it.second);
    }

    for (Entity* entity : entities)
    { //drawing methods do not use data from components
        DrawUndefinedCollisionTypeBoxes(entity);
        DrawHangingObjects(entity);
        DrawSwitchesWithDifferentLods(entity);
        DrawDebugOctTree(entity);
    }

    //draw selected objects
    const SelectableGroup& selection = GetScene()->GetSystem<SelectionSystem>()->GetSelection();
    for (auto entity : selection.ObjectsOfType<Entity>())
    {
        DrawLightNode(entity, true);
        DrawSelectedSoundNode(entity);
    }
}

void DebugDrawSystem::DrawCollisionTypeBox(Entity* entity) const
{
    if (collisionType == GetCollisionTypeComponent(entity)->GetCollisionType())
    {
        DrawEntityBox(entity, collisionTypeColor);
    }
}

void DebugDrawSystem::DrawUndefinedCollisionTypeBoxes(Entity* entity) const
{
    if (collisionType != CollisionTypeValues::COLLISION_TYPE_UNDEFINED)
        return;

    CollisionTypeComponent* coll = GetCollisionTypeComponent(entity);
    if (coll != nullptr)
        return;

    if (entity->GetParent() == GetScene()
        && GetLight(entity) == nullptr
        && GetCamera(entity) == nullptr
        && GetLandscape(entity) == nullptr)
    {
        DrawEntityBox(entity, collisionTypeColor);
    }
}

void DebugDrawSystem::DrawDebugOctTree(Entity* entity)
{
#if (DAVA_ALLOW_OCTREE_DEBUG)
    SceneEditor2* editorScene = static_cast<SceneEditor2*>(GetScene());
    RenderHelper* drawer = editorScene->GetRenderSystem()->GetDebugDrawer();

    RenderObject* renderObject = GetRenderObject(entity);
    if (renderObject == nullptr)
        return;

    for (uint32 k = 0; k < renderObject->GetActiveRenderBatchCount(); ++k)
    {
        RenderBatch* renderBatch = renderObject->GetActiveRenderBatch(k);

        PolygonGroup* pg = renderBatch->GetPolygonGroup();
        if (pg == nullptr)
            continue;

        GeometryOctTree* octTree = pg->GetGeometryOctTree();
        if (octTree == nullptr)
            continue;

        const Matrix4& wt = entity->GetWorldTransform();
        if (renderBatch->debugDrawOctree)
            octTree->DebugDraw(wt, 0, drawer);

        for (const GeometryOctTree::Triangle& tri : octTree->GetDebugTriangles())
        {
            Vector3 v1 = tri.v1 * entity->GetWorldTransform();
            Vector3 v2 = tri.v2 * entity->GetWorldTransform();
            Vector3 v3 = tri.v3 * entity->GetWorldTransform();
            GetScene()->GetRenderSystem()->GetDebugDrawer()->DrawLine(v1, v2, Color(1.0f, 0.0f, 0.0f, 1.0f), RenderHelper::eDrawType::DRAW_WIRE_NO_DEPTH);
            GetScene()->GetRenderSystem()->GetDebugDrawer()->DrawLine(v2, v3, Color(1.0f, 0.0f, 0.0f, 1.0f), RenderHelper::eDrawType::DRAW_WIRE_NO_DEPTH);
            GetScene()->GetRenderSystem()->GetDebugDrawer()->DrawLine(v3, v1, Color(1.0f, 0.0f, 0.0f, 1.0f), RenderHelper::eDrawType::DRAW_WIRE_NO_DEPTH);
        }

        for (const AABBox3& box : octTree->GetDebugBoxes())
        {
            GetScene()->GetRenderSystem()->GetDebugDrawer()->DrawAABoxTransformed(box, wt, Color(0.0f, 1.0f, 0.0f, 1.0f), RenderHelper::eDrawType::DRAW_WIRE_NO_DEPTH);
        }
    }
#endif
}

void DebugDrawSystem::DrawLightNode(Entity* entity, bool isSelected)
{
    Light* light = GetLight(entity);
    if (nullptr != light)
    {
        Scene* scene = GetScene();
        RenderHelper* drawer = scene->GetRenderSystem()->GetDebugDrawer();

        AABBox3 worldBox;
        AABBox3 localBox = scene->GetSystem<SceneCollisionSystem>()->GetUntransformedBoundingBox(entity);
        DVASSERT(!localBox.IsEmpty());

        TransformComponent* tc = entity->GetComponent<TransformComponent>();
        localBox.GetTransformedBox(tc->GetWorldMatrix(), worldBox);

        if (light->GetType() == Light::TYPE_DIRECTIONAL)
        {
            Vector3 center = worldBox.GetCenter();
            Vector3 direction = -light->GetDirection();
            direction.Normalize();
            direction = direction * worldBox.GetSize().x;
            center -= (direction / 2);
            drawer->DrawArrow(center + direction, center, direction.Length() / 2, Color(1.0f, 1.0f, 0, 1.0f), RenderHelper::DRAW_WIRE_DEPTH);
        }
        else if (light->GetType() == Light::TYPE_POINT)
        {
            Vector3 worldCenter = worldBox.GetCenter();
            drawer->DrawIcosahedron(worldCenter, worldBox.GetSize().x / 2, Color(1.0f, 1.0f, 0, 0.3f), RenderHelper::DRAW_SOLID_DEPTH);
            drawer->DrawIcosahedron(worldCenter, worldBox.GetSize().x / 2, Color(1.0f, 1.0f, 0, 1.0f), RenderHelper::DRAW_WIRE_DEPTH);
            KeyedArchive* properties = GetCustomPropertiesArchieve(entity);
            VariantType* value = properties->GetVariant("editor.staticlight.falloffcutoff");
            if (value != nullptr && value->GetType() == VariantType::TYPE_FLOAT && isSelected)
            {
                float32 distance = value->AsFloat();
                if (distance < BeastSystem::DEFAULT_FALLOFFCUTOFF_VALUE)
                {
                    uint32 segmentCount = 32;
                    drawer->DrawCircle(worldCenter, Vector3(1.0f, 0.0f, 0.0f), distance, segmentCount, Color(1.0f, 1.0f, 0.0f, 1.0f), RenderHelper::DRAW_WIRE_DEPTH);
                    drawer->DrawCircle(worldCenter, Vector3(0.0f, 1.0f, 0.0f), distance, segmentCount, Color(1.0f, 1.0f, 0.0f, 1.0f), RenderHelper::DRAW_WIRE_DEPTH);
                    drawer->DrawCircle(worldCenter, Vector3(0.0f, 0.0f, 1.0f), distance, segmentCount, Color(1.0f, 1.0f, 0.0f, 1.0f), RenderHelper::DRAW_WIRE_DEPTH);
                }
            }
        }
        else if (light->GetType() == Light::TYPE_SPOT)
        {
            float coneAngle = PI / 2.0f;
            float penumbraAngle = 0.0f;
            KeyedArchive* properties = GetCustomPropertiesArchieve(entity);
            if (properties != nullptr)
            {
                VariantType* value = properties->GetVariant("editor.staticlight.cone.angle");
                if (value->GetType() == VariantType::TYPE_FLOAT)
                {
                    coneAngle = value->AsFloat() * PI / 180.0f;
                }

                value = properties->GetVariant("editor.staticlight.cone.penumbra.angle");
                if (value->GetType() == VariantType::TYPE_FLOAT)
                {
                    penumbraAngle = value->AsFloat() * PI / 180.0f;
                }
            }

            Vector3 center = worldBox.GetCenter();
            Vector3 direction = light->GetDirection();
            direction.Normalize();

            center -= 0.5f * direction;
            drawer->DrawArrow(center, center + direction, 0.5f * direction.Length(), Color(1.0f, 1.0f, 1.0f, 1.0f), RenderHelper::DRAW_WIRE_DEPTH);

            float32 innerRadius = std::sin(0.5f * coneAngle);
            float32 outerRadius = std::sin(0.5f * std::max(coneAngle, coneAngle + penumbraAngle));
            uint32 sgm = 16;
            float32 dt = 0.2f;
            float32 t = dt;
            while (t <= 1.0f)
            {
                drawer->DrawCircle(center + t * direction, direction, t * innerRadius, sgm, Color(1.0f, 1.0f, 0.5f, 1.0f), RenderHelper::DRAW_WIRE_DEPTH);
                if (outerRadius > innerRadius)
                {
                    drawer->DrawCircle(center + t * direction, direction, t * outerRadius, sgm, Color(1.0f, 1.0f, 0.0f, 1.0f), RenderHelper::DRAW_WIRE_DEPTH);
                }
                t += dt;
            }
            drawer->DrawCircle(center + direction, direction, outerRadius, sgm, Color(1.0f, 1.0f, 0.0f, 0.3f), RenderHelper::DRAW_SOLID_DEPTH);
            drawer->DrawCircle(center + direction, direction, innerRadius, sgm, Color(1.0f, 1.0f, 0.5f, 0.3f), RenderHelper::DRAW_SOLID_DEPTH);
        }
        else
        {
            drawer->DrawAABox(worldBox, Color(1.0f, 1.0f, 0, 0.3f), RenderHelper::DRAW_SOLID_DEPTH);
            drawer->DrawAABox(worldBox, Color(1.0f, 1.0f, 0, 1.0f), RenderHelper::DRAW_WIRE_DEPTH);
        }
    }
}

void DebugDrawSystem::DrawSoundNode(Entity* entity)
{
    GlobalSceneSettings* settings = Deprecated::GetDataNode<GlobalSceneSettings>();

    if (settings->drawSoundObjects == false)
        return;

    SoundComponent* sc = GetSoundComponent(entity);
    if (sc)
    {
        Scene* scene = GetScene();

        AABBox3 worldBox;
        AABBox3 localBox = scene->GetSystem<SceneCollisionSystem>()->GetUntransformedBoundingBox(entity);
        if (!localBox.IsEmpty())
        {
            TransformComponent* tc = entity->GetComponent<TransformComponent>();
            localBox.GetTransformedBox(tc->GetWorldMatrix(), worldBox);

            Color soundColor = settings->soundObjectBoxColor;
            GetScene()->GetRenderSystem()->GetDebugDrawer()->DrawAABox(worldBox, ClampToUnityRange(soundColor), RenderHelper::DRAW_SOLID_DEPTH);
        }
    }
}

void DebugDrawSystem::DrawSelectedSoundNode(Entity* entity)
{
    GlobalSceneSettings* settings = Deprecated::GetDataNode<GlobalSceneSettings>();
    if (settings->drawSoundObjects == false)
        return;

    SoundComponent* sc = GetSoundComponent(entity);
    if (sc)
    {
        Scene* scene = GetScene();

        TransformComponent* tc = entity->GetComponent<TransformComponent>();
        Vector3 position = tc->GetWorldTransform().GetTranslation();

        uint32 fontHeight = 0;
        TextDrawSystem* textDrawSystem = scene->GetSystem<TextDrawSystem>();
        float32 fontSize = textDrawSystem->GetFontSize();
        GraphicFont* debugTextFont = textDrawSystem->GetFont();
        if (debugTextFont != nullptr)
        {
            fontHeight = debugTextFont->GetFontHeight(fontSize);
        }

        uint32 eventsCount = sc->GetEventsCount();
        for (uint32 i = 0; i < eventsCount; ++i)
        {
            SoundEvent* sEvent = sc->GetSoundEvent(i);
            float32 distance = sEvent->GetMaxDistance();

            Color soundColor = settings->soundObjectSphereColor;
            scene->GetRenderSystem()->GetDebugDrawer()->DrawIcosahedron(position, distance,
                                                                        ClampToUnityRange(soundColor), RenderHelper::DRAW_SOLID_DEPTH);

            textDrawSystem->DrawText(textDrawSystem->ToPos2d(position) - Vector2(0.f, fontHeight - 2.f) * i,
                                     sEvent->GetEventName(), Color::White, TextDrawSystem::Align::Center);

            if (sEvent->IsDirectional())
            {
                scene->GetRenderSystem()->GetDebugDrawer()->DrawArrow(position, sc->GetLocalDirection(i), .25f,
                                                                      Color(0.0f, 1.0f, 0.3f, 1.0f), RenderHelper::DRAW_SOLID_DEPTH);
            }
        }
    }
}

void DebugDrawSystem::DrawWindNode(Entity* entity)
{
    WindComponent* wind = GetWindComponent(entity);
    if (wind)
    {
        TransformComponent* tc = entity->GetComponent<TransformComponent>();
        Vector3 worldPosition = tc->GetWorldTransform().GetTranslation();

        GetScene()->GetRenderSystem()->GetDebugDrawer()->DrawArrow(worldPosition, worldPosition + wind->GetDirection() * 3.f, .75f,
                                                                   Color(1.0f, 0.5f, 0.2f, 1.0f), RenderHelper::DRAW_WIRE_DEPTH);
    }
}

void DebugDrawSystem::DrawEntityBox(Entity* entity, const Color& color) const
{
    Scene* scene = GetScene();
    AABBox3 localBox = scene->GetSystem<SceneCollisionSystem>()->GetUntransformedBoundingBox(entity);
    if (localBox.IsEmpty() == false)
    {
        AABBox3 worldBox;

        TransformComponent* tc = entity->GetComponent<TransformComponent>();
        localBox.GetTransformedBox(tc->GetWorldMatrix(), worldBox);
        GetScene()->GetRenderSystem()->GetDebugDrawer()->DrawAABox(worldBox, color, RenderHelper::DRAW_WIRE_DEPTH);
    }
}

void DebugDrawSystem::DrawHangingObjects(Entity* entity)
{
    if (hangingObjectsModeEnabled && (entity->GetParent() == GetScene()) && IsObjectHanging(entity))
    {
        DrawEntityBox(entity, Color(1.0f, 0.0f, 0.0f, 1.0f));
    }
}

void DebugDrawSystem::CollectRenderBatchesRecursively(Entity* entity, RenderBatchesWithTransforms& batches) const
{
    auto ro = GetRenderObject(entity);
    if (ro != nullptr)
    {
        auto roType = ro->GetType();
        if ((roType == RenderObject::TYPE_MESH) || (roType == RenderObject::TYPE_RENDEROBJECT) || (roType == RenderObject::TYPE_SPEED_TREE))
        {
            TransformComponent* tc = entity->GetComponent<TransformComponent>();
            const Matrix4& wt = tc->GetWorldMatrix();
            for (uint32 i = 0, e = ro->GetActiveRenderBatchCount(); i < e; ++i)
            {
                RenderBatch* batch = ro->GetActiveRenderBatch(i);
                if (batch != nullptr)
                {
                    PolygonGroup* pg = batch->GetPolygonGroup();
                    if (pg != nullptr)
                    {
                        batches.emplace_back(batch, wt);
                    }
                }
            }
        }
    }

    for (int32 i = 0, e = entity->GetChildrenCount(); i < e; ++i)
    {
        CollectRenderBatchesRecursively(entity->GetChild(i), batches);
    }
}

float32 DebugDrawSystem::GetMinimalZ(const RenderBatchesWithTransforms& batches) const
{
    float32 minZ = AABBOX_INFINITY;
    for (const auto& batch : batches)
    {
        PolygonGroup* polygonGroup = batch.first->GetPolygonGroup();
        for (uint32 v = 0, e = polygonGroup->GetVertexCount(); v < e; ++v)
        {
            Vector3 pos;
            polygonGroup->GetCoord(v, pos);
            minZ = Min(minZ, pos.z);
        }
    }
    return minZ;
}

void DebugDrawSystem::GetLowestVertexes(const RenderBatchesWithTransforms& batches, Vector<Vector3>& vertexes) const
{
    const float32 minZ = GetMinimalZ(batches);
    for (const auto& batch : batches)
    {
        float32 scale = std::sqrt(batch.second._20 * batch.second._20 + batch.second._21 * batch.second._21 + batch.second._22 * batch.second._22);
        PolygonGroup* polygonGroup = batch.first->GetPolygonGroup();
        for (uint32 v = 0, e = polygonGroup->GetVertexCount(); v < e; ++v)
        {
            Vector3 pos;
            polygonGroup->GetCoord(v, pos);
            if (scale * (pos.z - minZ) <= hangingObjectsHeight)
            {
                vertexes.push_back(pos * batch.second);
            }
        }
    }
}

bool DebugDrawSystem::IsObjectHanging(Entity* entity) const
{
    Vector<Vector3> lowestVertexes;
    RenderBatchesWithTransforms batches;
    CollectRenderBatchesRecursively(entity, batches);
    GetLowestVertexes(batches, lowestVertexes);

    for (const auto& vertex : lowestVertexes)
    {
        Vector3 landscapePoint = GetLandscapePointAtCoordinates(Vector2(vertex.x, vertex.y));
        if ((vertex.z - landscapePoint.z) > EPSILON)
        {
            return true;
        }
    }

    return false;
}

Vector3 DebugDrawSystem::GetLandscapePointAtCoordinates(const Vector2& centerXY) const
{
    LandscapeEditorDrawSystem* landSystem = GetScene()->GetSystem<LandscapeEditorDrawSystem>();
    LandscapeProxy* landscape = landSystem->GetLandscapeProxy();

    if (landscape)
    {
        return landscape->PlacePoint(Vector3(centerXY));
    }

    return Vector3();
}

void DebugDrawSystem::DrawSwitchesWithDifferentLods(Entity* entity)
{
    if (switchesWithDifferentLodsEnabled && SceneValidator::IsEntityHasDifferentLODsCount(entity))
    {
        Scene* scene = GetScene();

        AABBox3 worldBox;
        AABBox3 localBox = scene->GetSystem<SceneCollisionSystem>()->GetUntransformedBoundingBox(entity);
        DVASSERT(!localBox.IsEmpty());

        TransformComponent* tc = entity->GetComponent<TransformComponent>();
        localBox.GetTransformedBox(tc->GetWorldMatrix(), worldBox);
        scene->GetRenderSystem()->GetDebugDrawer()->DrawAABox(worldBox, Color(1.0f, 0.f, 0.f, 1.f), RenderHelper::DRAW_WIRE_DEPTH);
    }
}

void DebugDrawSystem::DrawDecals(Entity* entity)
{
    TransformComponent* tc = entity->GetComponent<TransformComponent>();

    RenderHelper* drawer = GetScene()->GetRenderSystem()->GetDebugDrawer();
    uint32 componentsCount = entity->GetComponentCount<GeoDecalComponent>();
    for (uint32 i = 0; i < componentsCount; ++i)
    {
        GeoDecalComponent* decal = entity->GetComponent<GeoDecalComponent>(i);

        DVASSERT(decal != nullptr);
        Matrix4 transform = tc->GetWorldMatrix();

        RenderHelper::eDrawType dt = RenderHelper::eDrawType::DRAW_WIRE_DEPTH;
        Color baseColor(1.0f, 0.5f, 0.25f, 1.0f);
        Color accentColor(1.0f, 1.0f, 0.5f, 1.0f);

        AABBox3 box = decal->GetBoundingBox();
        Vector3 boxCenter = box.GetCenter();
        Vector3 boxHalfSize = 0.5f * box.GetSize();

        Vector3 farPoint = Vector3(boxCenter.x, boxCenter.y, box.min.z) * transform;
        Vector3 nearPoint = Vector3(boxCenter.x, boxCenter.y, box.max.z) * transform;

        Vector3 direction = farPoint - nearPoint;
        direction.Normalize();

        drawer->DrawAABoxTransformed(box, transform, baseColor, dt);

        if (decal->GetConfig().mapping == GeoDecalManager::Mapping::CYLINDRICAL)
        {
            Vector3 side = Vector3(boxCenter.x - boxHalfSize.x, 0.0f, box.max.z) * transform;

            float radius = (side - nearPoint).Length();
            drawer->DrawCircle(nearPoint, direction, radius, 32, accentColor, dt);
            drawer->DrawCircle(farPoint, -direction, radius, 32, accentColor, dt);
            drawer->DrawLine(nearPoint, side, accentColor);
        }
        else if (decal->GetConfig().mapping == GeoDecalManager::Mapping::SPHERICAL)
        {
            // no extra debug visualization
        }
        else /* planar assumed */
        {
            drawer->DrawArrow(nearPoint - direction, nearPoint, 0.25f * direction.Length(), accentColor, dt);
        }
    }
}

std::unique_ptr<Command> DebugDrawSystem::PrepareForSave(bool saveForGame)
{
    Vector<Entity*> collisionTypeEntities = entitiesComponentMap[Type::Instance<CollisionTypeComponent>()];

    if (collisionTypeEntities.empty())
        return nullptr;

    RECommandBatch* batchCommand = new RECommandBatch("Prepare for save", static_cast<uint32>(collisionTypeEntities.size()));
    for (Entity* entity : collisionTypeEntities)
    {
        CollisionTypeComponent* component = GetCollisionTypeComponent(entity);
        DVASSERT(component != nullptr);

        GetOrCreateCustomProperties(entity);
        KeyedArchive* archive = GetCustomPropertiesArchieve(entity);

        int32 collisionType = component->GetCollisionType();
        if (collisionType != CollisionTypeValues::COLLISION_TYPE_UNDEFINED)
        {
            batchCommand->Add(std::make_unique<KeyedArchiveAddValueCommand>(archive, "CollisionType", VariantType(collisionType)));
        }

        int32 collisionTypeCrashed = component->GetCollisionTypeCrashed();
        if (collisionTypeCrashed != CollisionTypeValues::COLLISION_TYPE_UNDEFINED)
        {
            batchCommand->Add(std::make_unique<KeyedArchiveAddValueCommand>(archive, "CollisionTypeCrashed", VariantType(collisionTypeCrashed)));
        }
    }
    return std::unique_ptr<Command>(batchCommand);
}
} // namespace DAVA
