#include "REPlatform/Scene/Systems/EditorLODSystem.h"
#include "REPlatform/Scene/Systems/SelectionSystem.h"

#include "REPlatform/Commands/ChangeLODDistanceCommand.h"
#include "REPlatform/Commands/CloneLastBatchCommand.h"
#include "REPlatform/Commands/CopyLastLODCommand.h"
#include "REPlatform/Commands/CreatePlaneLODCommand.h"
#include "REPlatform/Commands/DeleteLODCommand.h"
#include "REPlatform/Commands/DeleteRenderBatchCommand.h"
#include "REPlatform/Commands/RECommandNotificationObject.h"
#include "REPlatform/Commands/RemoveComponentCommand.h"

#include "REPlatform/DataNodes/Settings/RESettings.h"
#include "REPlatform/Global/StringConstants.h"
#include "REplatform/Scene/SceneEditor2.h"

#include <TArc/Core/Deprecated.h>
#include <TArc/Utils/ScopedValueGuard.h>

#include <Entity/Component.h>
#include <Entity/SceneSystem.h>
#include <FileSystem/FileSystem.h>
#include <Scene3D/Components/ComponentHelpers.h>
#include <Scene3D/Components/RenderComponent.h>
#include <Scene3D/Entity.h>
#include <Scene3D/Lod/LodSystem.h>
#include <Scene3D/Components/ParticleEffectComponent.h>
#include <Scene3D/Scene.h>
#include <Utils/StringFormat.h>
#include <Utils/Utils.h>

namespace DAVA
{
namespace LODComponentHolderDetail
{
const float32 LOD_DISTANCE_EPSILON = 0.009f; //because we have only 2 sign after point in SpinBox (100.00f)
}

LODComponentHolder::LODComponentHolder()
    : distances(LodComponent::MAX_LOD_LAYERS, EditorLODSystem::LOD_DISTANCE_INFINITY)
    , isMultiple(LodComponent::MAX_LOD_LAYERS, false)
    , isChanged(LodComponent::MAX_LOD_LAYERS, false)
{
}

void LODComponentHolder::BindToSystem(EditorLODSystem* system_, SceneEditor2* scene_)
{
    DVASSERT(system_ != nullptr);
    DVASSERT(scene_ != nullptr);

    system = system_;
    scene = scene_;
}

void LODComponentHolder::SummarizeValues()
{
    maxLodLayerIndex = LodComponent::INVALID_LOD_LAYER;

    uint32 count = static_cast<uint32>(lodComponents.size());
    if (count > 0)
    {
        LodComponent* lc = lodComponents.front();
        for (int32 index = 0; index < LodComponent::MAX_LOD_LAYERS; ++index)
        {
            distances[index] = lc->GetLodLayerDistance(index);
            isMultiple[index] = false;
        }

        for (LodComponent* lc : lodComponents)
        {
            uint32 layersCount = GetLodLayersCount(lc);
            maxLodLayerIndex = Max(maxLodLayerIndex, static_cast<int32>(layersCount) - 1);

            for (int32 index = 0; index < LodComponent::MAX_LOD_LAYERS; ++index)
            {
                float32 dist = lc->GetLodLayerDistance(index);
                if (!isMultiple[index] && (fabs(distances[index] - dist) > LODComponentHolderDetail::LOD_DISTANCE_EPSILON))
                {
                    isMultiple[index] = true;
                }
            }
        }
    }
    else
    {
        for (int32 index = 0; index < LodComponent::MAX_LOD_LAYERS; ++index)
        {
            distances[index] = EditorLODSystem::LOD_DISTANCE_INFINITY;
            isMultiple[index] = false;
        }
    }
}

void LODComponentHolder::PropagateValues()
{
    scene->BeginBatch("LOD Distance Changed", static_cast<uint32>(lodComponents.size()) * LodComponent::MAX_LOD_LAYERS);
    for (LodComponent* lc : lodComponents)
    {
        for (int32 i = 0; i < LodComponent::MAX_LOD_LAYERS; ++i)
        {
            if (isChanged[i])
            {
                scene->Exec(std::unique_ptr<Command>(new ChangeLODDistanceCommand(lc, i, distances[i])));
            }
        }
    }
    scene->EndBatch();
}

bool LODComponentHolder::DeleteLOD(int32 layer)
{
    bool wasLayerRemoved = false;

    scene->BeginBatch(Format("Delete lod layer %", layer), static_cast<uint32>(lodComponents.size()));
    for (LodComponent* lc : lodComponents)
    {
        if ((GetLodLayersCount(lc) > 0) && (HasComponent(lc->GetEntity(), Type::Instance<ParticleEffectComponent>()) == false))
        {
            scene->Exec(std::unique_ptr<Command>(new DeleteLODCommand(lc, layer, -1)));
            wasLayerRemoved = true;
        }
    }
    scene->EndBatch();

    return wasLayerRemoved;
}

bool LODComponentHolder::CopyLod(int32 from, int32 to)
{
    bool wasCopiedRemoved = false;

    scene->BeginBatch(Format("Copy lod layer %d to %d", from, to), static_cast<uint32>(lodComponents.size()));
    for (LodComponent* lc : lodComponents)
    {
        Entity* entity = lc->GetEntity();
        if (HasComponent(entity, Type::Instance<ParticleEffectComponent>()))
        {
            continue;
        }

        if (GetLodLayersCount(entity) < LodComponent::MAX_LOD_LAYERS)
        {
            scene->Exec(std::unique_ptr<Command>(new CopyLastLODToLod0Command(lc)));
            wasCopiedRemoved = true;
        }
    }
    scene->EndBatch();
    return wasCopiedRemoved;
}

void LODComponentHolder::ApplyForce(const ForceValues& force)
{
    for (LodComponent* lc : lodComponents)
    {
        if (force.flag & ForceValues::APPLY_LAYER)
        {
            if (force.layer == EditorLODSystem::LAST_LOD_LAYER)
            {
                int32 lastLayer = Max(0, static_cast<int32>(GetLodLayersCount(lc)) - 1);
                scene->lodSystem->SetForceLodLayer(lc, lastLayer);
            }
            else
            {
                scene->lodSystem->SetForceLodLayer(lc, force.layer);
            }
        }

        if (force.flag & ForceValues::APPLY_DISTANCE)
        {
            scene->lodSystem->SetForceLodDistance(lc, force.distance);
        }
    }
}

int32 LODComponentHolder::GetMaxLODLayer() const
{
    return maxLodLayerIndex;
}

uint32 LODComponentHolder::GetLODLayersCount() const
{
    return (maxLodLayerIndex + 1);
}

const Vector<float32>& LODComponentHolder::GetDistances() const
{
    return distances;
}

const Vector<bool>& LODComponentHolder::GetMultiple() const
{
    return isMultiple;
}

//SYSTEM
const float32 EditorLODSystem::LOD_DISTANCE_INFINITY = std::numeric_limits<float32>::max();

EditorLODSystem::EditorLODSystem(Scene* scene)
    : SceneSystem(scene)
{
    for (uint32 m = 0; m < eEditorMode::MODE_COUNT; ++m)
    {
        lodData[m].BindToSystem(this, static_cast<SceneEditor2*>(GetScene()));
    }

    CommonInternalSettings* settings = Deprecated::GetDataNode<CommonInternalSettings>();

    const bool allSceneModeEnabled = settings->lodEditorSceneMode;
    mode = (allSceneModeEnabled) ? eEditorMode::MODE_ALL_SCENE : eEditorMode::MODE_SELECTION;
    activeLodData = &lodData[mode];

    recursive = settings->lodEditorRecursive;
}

EditorLODSystem::~EditorLODSystem()
{
    activeLodData = nullptr;
}

void EditorLODSystem::Process(float32 timeElapsed)
{
    for (DAVA::uint32 mode = 0; mode < eEditorMode::MODE_COUNT; ++mode)
    {
        if (pendingSummarizeValues[mode] == true)
        {
            lodData[mode].SummarizeValues();
            pendingSummarizeValues[mode] = false;
        }
    }
    ProcessAddedEntities();
    DispatchSignals();
    ProcessPlaneLODs();
}

void EditorLODSystem::ProcessAddedEntities()
{
    if (componentsToAdd.empty())
    {
        return;
    }

    bool invalidateUI = mode == eEditorMode::MODE_ALL_SCENE;

    const SelectableGroup& selection = GetScene()->GetSystem<SelectionSystem>()->GetSelection();
    for (const std::pair<Entity*, LodComponent*>& pair : componentsToAdd)
    {
        if (selection.ContainsObject(pair.first))
        {
            ForceValues resetForceValues(LodComponent::INVALID_DISTANCE, LodComponent::INVALID_LOD_LAYER, ForceValues::APPLY_ALL);

            resetForceValues.flag = ForceValues::APPLY_ALL;
            resetForceValues.layer = LodComponent::INVALID_LOD_LAYER;
            resetForceValues.distance = LodComponent::INVALID_DISTANCE;
            lodData[eEditorMode::MODE_SELECTION].ApplyForce(resetForceValues);
            lodData[eEditorMode::MODE_SELECTION].lodComponents.push_back(pair.second);
            lodData[eEditorMode::MODE_SELECTION].ApplyForce(forceValues);
            pendingSummarizeValues[eEditorMode::MODE_SELECTION] = true;
            invalidateUI = true;
        }
    }

    if (invalidateUI == true)
    {
        EmitInvalidateUI(FLAG_ALL);
    }

    componentsToAdd.clear();
}

void EditorLODSystem::AddEntity(Entity* entity)
{
    LodComponent* lc = GetLodComponent(entity);
    DVASSERT(lc != nullptr);

    AddComponent(entity, lc);
}

void EditorLODSystem::RemoveEntity(Entity* entity)
{
    LodComponent* lc = GetLodComponent(entity);
    DVASSERT(lc != nullptr);

    RemoveComponent(entity, lc);
}

void EditorLODSystem::AddComponent(Entity* entity, Component* component)
{
    DVASSERT(component->GetType()->Is<LodComponent>());

    LodComponent* lodComponent = static_cast<LodComponent*>(component);
    lodData[eEditorMode::MODE_ALL_SCENE].lodComponents.push_back(lodComponent);
    pendingSummarizeValues[eEditorMode::MODE_ALL_SCENE] = true;

    componentsToAdd.emplace_back(entity, lodComponent);
}

void EditorLODSystem::RemoveComponent(Entity* entity, Component* component)
{
    DVASSERT(component->GetType()->Is<LodComponent>());

    LodComponent* removedComponent = static_cast<LodComponent*>(component);
    for (uint32 m = 0; m < eEditorMode::MODE_COUNT; ++m)
    {
        bool removed = FindAndRemoveExchangingWithLast(lodData[m].lodComponents, removedComponent);
        if (removed)
        {
            pendingSummarizeValues[m] = true;
            if (m == mode)
            {
                EmitInvalidateUI(FLAG_ALL);
            }
        }
    }

    if (componentsToAdd.empty() == false)
    {
        componentsToAdd.erase(std::remove_if(componentsToAdd.begin(), componentsToAdd.end(),
                                             [removedComponent](const std::pair<Entity*, LodComponent*>& pair) {
                                                 return pair.second == removedComponent;
                                             }
                                             ),
                              componentsToAdd.end());
    }
}

void EditorLODSystem::PrepareForRemove()
{
    for (uint32 m = 0; m < eEditorMode::MODE_COUNT; ++m)
    {
        lodData[m].lodComponents.clear();
    }
    componentsToAdd.clear();
}

void EditorLODSystem::SceneDidLoaded()
{
    pendingSummarizeValues[eEditorMode::MODE_ALL_SCENE] = true;
    if (mode == eEditorMode::MODE_ALL_SCENE)
    {
        EmitInvalidateUI(FLAG_ALL);
    }
}

eEditorMode EditorLODSystem::GetMode() const
{
    return mode;
}

void EditorLODSystem::SetMode(eEditorMode mode_)
{
    DVASSERT(activeLodData != nullptr);

    activeLodData->ApplyForce({ -1, -1, ForceValues::APPLY_ALL });
    mode = mode_;
    activeLodData = &lodData[mode];
    activeLodData->ApplyForce(forceValues);

    EmitInvalidateUI(FLAG_ALL);
}

bool EditorLODSystem::GetRecursive() const
{
    return recursive;
}

void EditorLODSystem::SetRecursive(bool recursive_)
{
    recursive = recursive_;

    const SelectableGroup& selection = GetScene()->GetSystem<SelectionSystem>()->GetSelection();
    SelectionChanged(selection);
}

const ForceValues& EditorLODSystem::GetForceValues() const
{
    return forceValues;
}

void EditorLODSystem::SetForceValues(const ForceValues& values)
{
    DVASSERT(activeLodData != nullptr);

    ForceValues distanceDiffValues;
    distanceDiffValues.flag = ForceValues::APPLY_ALL;

    if (values.flag != forceValues.flag)
    {
        if (values.flag & ForceValues::APPLY_DISTANCE)
        {
            distanceDiffValues.distance = values.distance;
            distanceDiffValues.layer = LodComponent::INVALID_LOD_LAYER;
        }
        if (values.flag & ForceValues::APPLY_LAYER)
        {
            distanceDiffValues.distance = LodComponent::INVALID_DISTANCE;
            distanceDiffValues.layer = values.layer;
        }
    }
    if (values.distance != forceValues.distance)
    {
        distanceDiffValues.distance = values.distance;
        distanceDiffValues.layer = LodComponent::INVALID_LOD_LAYER;
    }
    if (values.layer != forceValues.layer)
    {
        distanceDiffValues.distance = LodComponent::INVALID_DISTANCE;
        distanceDiffValues.layer = values.layer;
    }

    activeLodData->ApplyForce(distanceDiffValues);
    forceValues = values;

    EmitInvalidateUI(FLAG_FORCE);
}

bool EditorLODSystem::CanDeleteLOD() const
{
    DVASSERT(activeLodData != nullptr);
    using namespace DAVA;

    bool canDeleteLod = (!activeLodData->lodComponents.empty()) && (activeLodData->GetLODLayersCount() > 1);
    if (canDeleteLod == true)
    {
        for (auto& lc : activeLodData->lodComponents)
        {
            Entity* entity = lc->GetEntity();
            if (HasComponent(entity, DAVA::Type::Instance<DAVA::ParticleEffectComponent>()))
            {
                canDeleteLod = false;
                break;
            }
            else
            {
                RenderObject* ro = GetRenderObject(entity);
                if (ro == nullptr)
                {
                    continue;
                }

                Set<int32> lodIndexes;
                uint32 batchCount = ro->GetRenderBatchCount();

                for (uint32 i = 0; i < batchCount; ++i)
                {
                    int32 lodIndex, switchIndex;
                    ro->GetRenderBatch(i, lodIndex, switchIndex);
                    lodIndexes.insert(lodIndex);
                }

                if (lodIndexes.size() == 1)
                {
                    canDeleteLod = false;
                    break;
                }
            }
        }
    }

    return canDeleteLod;
}

bool EditorLODSystem::CanCreateLOD() const
{
    DVASSERT(activeLodData != nullptr);
    if (activeLodData->lodComponents.size() == 1) // we can create lod only for one entity
    {
        bool canCreateLod = (activeLodData->GetLODLayersCount() < LodComponent::MAX_LOD_LAYERS);
        if (canCreateLod)
        {
            Entity* entity = activeLodData->lodComponents[0]->GetEntity();
            if (HasComponent(entity, Type::Instance<ParticleEffectComponent>()))
            {
                return false;
            }

            return HasComponent(entity, Type::Instance<RenderComponent>());
        }
    }

    return false;
}

void EditorLODSystem::CreatePlaneLOD(int32 fromLayer, uint32 textureSize, const FilePath& texturePath)
{
    DVASSERT(activeLodData != nullptr);

    planeLODRequests.reserve(activeLodData->lodComponents.size());
    for (LodComponent* lc : activeLodData->lodComponents)
    {
        Entity* entity = lc->GetEntity();
        DVASSERT(entity != nullptr);

        RenderObject* ro = GetRenderObject(entity);
        if (ro != nullptr)
        {
            bool hasGeometry = false;
            uint32 count = ro->GetRenderBatchCount();
            for (uint32 i = 0; i < count; ++i)
            {
                int32 lod, sw;
                if ((ro->GetRenderBatch(i, lod, sw) != nullptr) && (lod == fromLayer))
                {
                    hasGeometry = true;
                    break;
                }
            }

            if (hasGeometry)
            {
                auto request = CreatePlaneLODCommandHelper::RequestRenderToTexture(lc, fromLayer, textureSize, texturePath);
                planeLODRequests.push_back(request);
            }
            else
            {
                Logger::Error("Cannot create planeLod from not exiting geometry at layer %d", fromLayer);
            }
        }
    }
}

void EditorLODSystem::DeleteLOD(int32 layer)
{
    if (activeLodData->GetLODLayersCount() > 0)
    {
        ScopedValueGuard<bool> guard(generateCommands, true);
        bool deleted = activeLodData->DeleteLOD(layer);
        if (deleted)
        {
            RecalculateData();
            EmitInvalidateUI(FLAG_ALL);
        }
    }
}

void EditorLODSystem::CopyLastLODToFirst()
{
    DVASSERT(activeLodData != nullptr);

    ScopedValueGuard<bool> guard(generateCommands, true);
    bool copied = activeLodData->CopyLod(activeLodData->GetMaxLODLayer(), 0);
    if (copied)
    {
        RecalculateData();
        EmitInvalidateUI(FLAG_FORCE | FLAG_DISTANCE | FLAG_ACTION);
    }
}

const LODComponentHolder* EditorLODSystem::GetActiveLODData() const
{
    return activeLodData;
}

void EditorLODSystem::SetLODDistances(const Vector<float32>& distances)
{
    DVASSERT(activeLodData != nullptr);
    DVASSERT(distances.size() == LodComponent::MAX_LOD_LAYERS);

    for (int32 layer = 0; layer < LodComponent::MAX_LOD_LAYERS; ++layer)
    {
        if (fabs(activeLodData->distances[layer] - distances[layer]) > LODComponentHolderDetail::LOD_DISTANCE_EPSILON)
        {
            activeLodData->distances[layer] = distances[layer];
            activeLodData->isChanged[layer] = true;
        }
        else
        {
            activeLodData->isChanged[layer] = false;
        }
    }

    ScopedValueGuard<bool> guard(generateCommands, true);
    activeLodData->PropagateValues();

    RecalculateData();
    EmitInvalidateUI(FLAG_DISTANCE);
}

void EditorLODSystem::SelectionChanged(const SelectableGroup& selection)
{
    { //reset force values
        ForceValues resetForceValues(LodComponent::INVALID_DISTANCE, LodComponent::INVALID_LOD_LAYER, ForceValues::APPLY_ALL);

        resetForceValues.flag = ForceValues::APPLY_ALL;
        resetForceValues.layer = LodComponent::INVALID_LOD_LAYER;
        resetForceValues.distance = LodComponent::INVALID_DISTANCE;

        lodData[eEditorMode::MODE_SELECTION].ApplyForce(resetForceValues);
        lodData[eEditorMode::MODE_SELECTION].lodComponents.clear();
    }

    uint32 count = selection.GetSize();
    Vector<Entity*> lodEntities;
    lodEntities.reserve(count); //mostly we have less than 5 lods in hierarchy

    for (auto entity : selection.ObjectsOfType<Entity>())
    {
        if (recursive)
        {
            entity->GetChildEntitiesWithComponent(lodEntities, Type::Instance<LodComponent>());
        }

        if (entity->GetComponentCount<LodComponent>() > 0)
        {
            lodEntities.push_back(entity);
        }
    }

    std::sort(lodEntities.begin(), lodEntities.end());
    lodEntities.erase(std::unique(lodEntities.begin(), lodEntities.end()), lodEntities.end());

    for (auto& entity : lodEntities)
    {
        uint32 count = entity->GetComponentCount<LodComponent>();
        for (uint32 i = 0; i < count; ++i)
        {
            lodData[eEditorMode::MODE_SELECTION].lodComponents.push_back(entity->GetComponent<LodComponent>(i));
        }
    }

    pendingSummarizeValues[eEditorMode::MODE_SELECTION] = true;
    if (mode == eEditorMode::MODE_SELECTION)
    {
        lodData[eEditorMode::MODE_SELECTION].ApplyForce(forceValues);

        EmitInvalidateUI(FLAG_ALL);
    }
}

void EditorLODSystem::AddDelegate(EditorLODSystemUIDelegate* uiDelegate)
{
    DVASSERT(uiDelegate != nullptr);

    uiDelegates.push_back(uiDelegate);
    if (uiDelegate != nullptr)
    {
        uiDelegate->UpdateModeUI(this, mode, recursive);
        uiDelegate->UpdateForceUI(this, forceValues);
        uiDelegate->UpdateDistanceUI(this, activeLodData);
        uiDelegate->UpdateActionUI(this);
    }
}

void EditorLODSystem::RemoveDelegate(EditorLODSystemUIDelegate* uiDelegate)
{
    DVASSERT(uiDelegate != nullptr);

    FindAndRemoveExchangingWithLast(uiDelegates, uiDelegate);
}

void EditorLODSystem::EmitInvalidateUI(uint32 flags)
{
    invalidateUIFlag |= flags;
}

void EditorLODSystem::DispatchSignals()
{
    if (invalidateUIFlag == FLAG_NONE)
    {
        return;
    }

    for (auto& d : uiDelegates)
    {
        if (invalidateUIFlag & FLAG_MODE)
        {
            d->UpdateModeUI(this, mode, recursive);
        }

        if (invalidateUIFlag & FLAG_FORCE)
        {
            d->UpdateForceUI(this, forceValues);
        }

        if (invalidateUIFlag & FLAG_DISTANCE)
        {
            d->UpdateDistanceUI(this, activeLodData);
        }

        if (invalidateUIFlag & FLAG_ACTION)
        {
            d->UpdateActionUI(this);
        }
    }

    invalidateUIFlag = FLAG_NONE;
}

void EditorLODSystem::ProcessCommand(const RECommandNotificationObject& commandNotification)
{
    SCOPED_VALUE_GUARD(bool, generateCommands, true, void());

    if (commandNotification.MatchCommandTypes<ChangeLODDistanceCommand>())
    {
        RecalculateData();
        EmitInvalidateUI(FLAG_DISTANCE);
    }

    auto InvalidateAllData = [this]()
    {
        RecalculateData();
        activeLodData->ApplyForce(forceValues);

        EmitInvalidateUI(FLAG_ALL);
    };

    if (commandNotification.MatchCommandTypes<DeleteRenderBatchCommand, CloneLastBatchCommand, CreatePlaneLODCommand,
                                              CopyLastLODToLod0Command, DeleteLODCommand>())
    {
        InvalidateAllData();
    }

    commandNotification.ForEach<RemoveComponentCommand>([&](const RemoveComponentCommand* cmd) {
        if (cmd->GetComponent()->GetType()->Is<RenderComponent>())
        {
            InvalidateAllData();
        }
    });
}

FilePath EditorLODSystem::GetPathForPlaneEntity() const
{
    DVASSERT(activeLodData != nullptr);
    DVASSERT(!activeLodData->lodComponents.empty());

    SceneEditor2* editorScene = static_cast<SceneEditor2*>(GetScene());
    Entity* entity = activeLodData->lodComponents.front()->GetEntity();

    FilePath entityPath = editorScene->GetScenePath();
    KeyedArchive* properties = GetCustomPropertiesArchieve(entity);
    if (nullptr != properties && properties->IsKeyExists(ResourceEditor::EDITOR_REFERENCE_TO_OWNER))
    {
        entityPath = FilePath(properties->GetString(ResourceEditor::EDITOR_REFERENCE_TO_OWNER, entityPath.GetAbsolutePathname()));
    }
    String entityName = entity->GetName().c_str();
    FilePath textureFolder = entityPath.GetDirectory() + "images/";

    String texturePostfix = "_planes.png";
    FilePath texturePath = textureFolder + entityName + texturePostfix;
    int32 i = 0;
    while (FileSystem::Instance()->Exists(texturePath))
    {
        i++;
        texturePath = textureFolder + Format("%s_%d%s", entityName.c_str(), i, texturePostfix.c_str());
    }

    return texturePath;
}

void EditorLODSystem::ProcessPlaneLODs()
{
    if (planeLODRequests.empty())
    {
        return;
    }

    bool allRequestsProcessed = true;
    for (const auto& req : planeLODRequests)
    {
        allRequestsProcessed = allRequestsProcessed && req->completed;
    }

    if (allRequestsProcessed)
    {
        ScopedValueGuard<bool> guard(generateCommands, true);

        SceneEditor2* sceneEditor2 = static_cast<SceneEditor2*>(GetScene());
        sceneEditor2->BeginBatch("Create plane lods", static_cast<uint32>(planeLODRequests.size()));
        for (const auto& req : planeLODRequests)
        {
            sceneEditor2->Exec(std::unique_ptr<Command>(new CreatePlaneLODCommand(req)));
        }
        sceneEditor2->EndBatch();
        planeLODRequests.clear();

        RecalculateData();
        EmitInvalidateUI(FLAG_FORCE | FLAG_DISTANCE | FLAG_ACTION);
    }
}

void EditorLODSystem::RecalculateData()
{
    for (uint32 m = 0; m < eEditorMode::MODE_COUNT; ++m)
    {
        lodData[m].SummarizeValues();
    }
}

bool EditorLODSystem::IsFitModeEnabled(const Vector<float32>& distances)
{
    if (Deprecated::GetDataNode<GeneralSettings>()->fitSliders)
    {
        return true;
    }

    for (float32 dist : distances)
    {
        if ((fabs(dist - LOD_DISTANCE_INFINITY) > EPSILON) && (dist > LodComponent::MAX_LOD_DISTANCE))
        { // not max float and more than 1000
            return true;
        }
    }

    return false;
}
} // namespace DAVA
