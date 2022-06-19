#include "REPlatform/Scene/Systems/EditorStatisticsSystem.h"
#include "REPlatform/Scene/Systems/SelectionSystem.h"

#include "REPlatform/Commands/CopyLastLODCommand.h"
#include "REPlatform/Commands/CreatePlaneLODCommand.h"
#include "REPlatform/Commands/DeleteLODCommand.h"
#include "REPlatform/Commands/DeleteRenderBatchCommand.h"
#include "REPlatform/Commands/RECommandNotificationObject.h"
#include "REPlatform/DataNodes/SelectableGroup.h"
#include "REPlatform/DataNodes/SelectionData.h"
#include "REPlatform/DataNodes/Settings/RESettings.h"

#include <TArc/Core/Deprecated.h>

#include <Debug/DVAssert.h>
#include <Render/Highlevel/RenderBatch.h>
#include <Render/Highlevel/RenderObject.h>
#include <Scene3D/Components/ComponentHelpers.h>
#include <Scene3D/Components/RenderComponent.h>
#include <Scene3D/Entity.h>
#include <Scene3D/Lod/LodComponent.h>
#include <Scene3D/Scene.h>

namespace DAVA
{
struct TrianglesData
{
    Vector<uint32> storedTriangles;
    Vector<uint32> visibleTriangles;
    Vector<RenderObject*> renderObjects;
};

DAVA_VIRTUAL_REFLECTION_IMPL(RenderStatsSettings)
{
    ReflectionRegistrator<RenderStatsSettings>::Begin()[M::DisplayName("Render statistics")]
    .ConstructorByPointer()
    .Field("calculatePerFrame", &RenderStatsSettings::calculatePerFrame)[M::DisplayName(" Calculate per frame")]
    .End();
}

namespace EditorStatisticsSystemInternal
{
static const int32 SIZE_OF_TRIANGLES = LodComponent::MAX_LOD_LAYERS + 1;

void EnumerateTriangles(RenderObject* renderObject, Vector<uint32>& triangles, Vector<uint32>& visibleTriangles)
{
    uint32 batchCount = renderObject->GetRenderBatchCount();
    for (uint32 b = 0; b < batchCount; ++b)
    {
        int32 lodIndex = 0;
        int32 switchIndex = 0;

        RenderBatch* rb = renderObject->GetRenderBatch(b, lodIndex, switchIndex);
        lodIndex += 1; //because of non-lod index is -1
        if (lodIndex < 0)
        {
            continue; //means that lod is uninitialized
        }
        DVASSERT(lodIndex <= static_cast<int32>(triangles.size()));

        if (IsPointerToExactClass<RenderBatch>(rb))
        {
            bool batchIsVisible = false;
            uint32 activeBatchCount = renderObject->GetActiveRenderBatchCount();
            for (uint32 a = 0; a < activeBatchCount; ++a)
            {
                if (renderObject->GetActiveRenderBatch(a) == rb)
                {
                    batchIsVisible = true;
                    break;
                }
            }

            PolygonGroup* pg = rb->GetPolygonGroup();
            if (nullptr != pg)
            {
                int32 trianglesCount = pg->GetPrimitiveCount();
                triangles[lodIndex] += trianglesCount;
                if (batchIsVisible)
                {
                    visibleTriangles[lodIndex] += trianglesCount;
                }
            }
        }
    }
}

void EnumerateTriangles(TrianglesData& triangles)
{
    std::fill(triangles.storedTriangles.begin(), triangles.storedTriangles.end(), 0);
    std::fill(triangles.visibleTriangles.begin(), triangles.visibleTriangles.end(), 0);
    for (RenderObject* ro : triangles.renderObjects)
    {
        if (ro && (ro->GetType() == RenderObject::TYPE_MESH || ro->GetType() == RenderObject::TYPE_SPEED_TREE))
        {
            EnumerateTriangles(ro, triangles.storedTriangles, triangles.visibleTriangles);
        }
    }
}

void EnumerateRenderObjectsRecursive(Entity* entity, Vector<RenderObject*>& renderObjects, bool recursive)
{
    if (HasComponent(entity, Type::Instance<RenderComponent>()))
    {
        uint32 componentsCount = entity->GetComponentCount<RenderComponent>();
        for (uint32 c = 0; c < componentsCount; ++c)
        {
            RenderComponent* rc = entity->GetComponent<RenderComponent>(c);
            RenderObject* ro = rc->GetRenderObject();
            if (ro != nullptr)
            {
                if (std::find(renderObjects.begin(), renderObjects.end(), ro) == renderObjects.end())
                {
                    renderObjects.push_back(ro);
                }
            }
        }
    }

    if (recursive)
    {
        uint32 count = entity->GetChildrenCount();
        for (uint32 c = 0; c < count; ++c)
        {
            EnumerateRenderObjectsRecursive(entity->GetChild(c), renderObjects, recursive);
        }
    }
}

void EnumerateRenderObjects(const SelectableGroup& group, Vector<RenderObject*>& renderObjects)
{
    renderObjects.clear();
    if (group.IsEmpty())
        return;

    renderObjects.reserve(group.GetSize());

    CommonInternalSettings* settings = Deprecated::GetDataNode<CommonInternalSettings>();
    for (auto entity : group.ObjectsOfType<Entity>())
    {
        EnumerateRenderObjectsRecursive(entity, renderObjects, settings->lodEditorRecursive);
    }
}
}
EditorStatisticsSystem::EditorStatisticsSystem(Scene* scene)
    : SceneSystem(scene)
    , binder(new FieldBinder(Deprecated::GetAccessor()))
{
    triangles.resize(eEditorMode::MODE_COUNT);
    for (uint32 m = 0; m < eEditorMode::MODE_COUNT; ++m)
    {
        triangles[m].storedTriangles.resize(EditorStatisticsSystemInternal::SIZE_OF_TRIANGLES, 0);
        triangles[m].visibleTriangles.resize(EditorStatisticsSystemInternal::SIZE_OF_TRIANGLES, 0);
    }

    {
        FieldDescriptor descr;
        descr.type = ReflectedTypeDB::Get<RenderStatsSettings>();
        descr.fieldName = FastName("calculatePerFrame");
        binder->BindField(descr, [this](const Any& v) {
            calculatePerFrame = v.Cast<bool>(true);
        });
    }

    {
        FieldDescriptor descr;
        descr.type = ReflectedTypeDB::Get<SelectionData>();
        descr.fieldName = FastName(SelectionData::selectionPropertyName);
        binder->BindField(descr, [this](const Any& v) {
            EmitInvalidateUI(FLAG_TRIANGLES);
        });
    }
}

void EditorStatisticsSystem::RegisterEntity(Entity* entity)
{
    if (HasComponent(entity, Type::Instance<RenderComponent>()) || HasComponent(entity, Type::Instance<LodComponent>()))
    {
        EmitInvalidateUI(FLAG_TRIANGLES);
    }
}

void EditorStatisticsSystem::UnregisterEntity(Entity* entity)
{
    if (HasComponent(entity, Type::Instance<RenderComponent>()) || HasComponent(entity, Type::Instance<LodComponent>()))
    {
        EmitInvalidateUI(FLAG_TRIANGLES);
    }
}

void EditorStatisticsSystem::RegisterComponent(Entity* entity, Component* component)
{
    const Type* type = component->GetType();
    if (type->Is<RenderComponent>() || type->Is<LodComponent>())
    {
        EmitInvalidateUI(FLAG_TRIANGLES);
    }
}

void EditorStatisticsSystem::UnregisterComponent(Entity* entity, Component* component)
{
    const Type* type = component->GetType();
    if (type->Is<RenderComponent>() || type->Is<LodComponent>())
    {
        EmitInvalidateUI(FLAG_TRIANGLES);
    }
}

void EditorStatisticsSystem::PrepareForRemove()
{
}

const Vector<uint32>& EditorStatisticsSystem::GetTriangles(eEditorMode mode, bool allTriangles)
{
    if (calculatePerFrame == false && initialized == true)
    {
        CalculateTriangles();
    }

    if (allTriangles)
    {
        return triangles[mode].storedTriangles;
    }

    return triangles[mode].visibleTriangles;
}

void EditorStatisticsSystem::Process(float32 timeElapsed)
{
    initialized = true;
    if (calculatePerFrame == true)
    {
        CalculateTriangles();
    }
    DispatchSignals();
}

void EditorStatisticsSystem::ClipSelection(Camera* camera, Vector<RenderObject*>& selection,
                                           Vector<RenderObject*>& visibilityArray, uint32 visibilityCriteria)
{
    Frustum* frustum = camera->GetFrustum();
    uint32 size = static_cast<uint32>(selection.size());
    for (uint32 pos = 0; pos < size; ++pos)
    {
        RenderObject* node = selection[pos];
        if ((node->GetFlags() & visibilityCriteria) != visibilityCriteria)
        {
            continue;
        }
        if ((RenderObject::ALWAYS_CLIPPING_VISIBLE & node->GetFlags()) ||
            frustum->IsInside(node->GetWorldBoundingBox()))
        {
            visibilityArray.push_back(node);
        }
    }
}

void EditorStatisticsSystem::CalculateTriangles()
{
    auto CalculateTrianglesForMode = [this](eEditorMode mode)
    {
        Vector<uint32> storedTriangles = triangles[mode].storedTriangles;
        Vector<uint32> visibleTriangles = triangles[mode].visibleTriangles;

        EditorStatisticsSystemInternal::EnumerateTriangles(triangles[mode]);
        if (triangles[mode].storedTriangles != storedTriangles || triangles[mode].visibleTriangles != visibleTriangles)
        {
            EmitInvalidateUI(FLAG_TRIANGLES);
        }
    };

    //Scene
    triangles[eEditorMode::MODE_ALL_SCENE].renderObjects.clear();

    //Selection
    triangles[eEditorMode::MODE_SELECTION].renderObjects.clear();

    Scene* scene = GetScene();
    const SelectableGroup& selection = scene->GetSystem<SelectionSystem>()->GetSelection();

    Vector<RenderObject*> selectedObjects;
    EditorStatisticsSystemInternal::EnumerateRenderObjects(selection, selectedObjects);

    // Clip objects
    Camera* drawCamera = scene->GetDrawCamera();
    if (drawCamera != nullptr)
    {
        uint32 currVisibilityCriteria = RenderObject::CLIPPING_VISIBILITY_CRITERIA;
        scene->GetRenderSystem()->GetRenderHierarchy()->Clip(drawCamera, triangles[eEditorMode::MODE_ALL_SCENE].renderObjects, currVisibilityCriteria);
        ClipSelection(drawCamera, selectedObjects, triangles[eEditorMode::MODE_SELECTION].renderObjects, currVisibilityCriteria);
    }
    CalculateTrianglesForMode(eEditorMode::MODE_ALL_SCENE);
    CalculateTrianglesForMode(eEditorMode::MODE_SELECTION);
}

void EditorStatisticsSystem::EmitInvalidateUI(uint32 flags)
{
    invalidateUIflag = flags;
}

void EditorStatisticsSystem::DispatchSignals()
{
    if (invalidateUIflag == FLAG_NONE)
    {
        return;
    }

    for (auto& d : uiDelegates)
    {
        if (invalidateUIflag & FLAG_TRIANGLES)
        {
            d->UpdateTrianglesUI(this);
        }
    }

    invalidateUIflag = FLAG_NONE;
}

void EditorStatisticsSystem::ProcessCommand(const RECommandNotificationObject& commandNotification)
{
    if (commandNotification.MatchCommandTypes<DeleteRenderBatchCommand, CopyLastLODToLod0Command,
                                              CreatePlaneLODCommand, DeleteLODCommand>())
    {
        EmitInvalidateUI(FLAG_TRIANGLES);
    }
}

void EditorStatisticsSystem::AddDelegate(EditorStatisticsSystemUIDelegate* uiDelegate)
{
    DVASSERT(uiDelegate != nullptr);

    uiDelegates.push_back(uiDelegate);
    if (uiDelegate != nullptr)
    {
        uiDelegate->UpdateTrianglesUI(this);
    }
}

void EditorStatisticsSystem::RemoveDelegate(EditorStatisticsSystemUIDelegate* uiDelegate)
{
    DVASSERT(uiDelegate != nullptr);

    FindAndRemoveExchangingWithLast(uiDelegates, uiDelegate);
}
} // namespace DAVA
