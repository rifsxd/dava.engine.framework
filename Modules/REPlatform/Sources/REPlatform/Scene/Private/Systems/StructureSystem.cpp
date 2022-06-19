#include "REPlatform/Scene/Systems/StructureSystem.h"
#include "REPlatform/Scene/Systems/CameraSystem.h"
#include "REPlatform/Scene/Systems/CollisionSystem.h"
#include "REPlatform/Scene/SceneEditor2.h"

#include "REPlatform/DataNodes/ProjectManagerData.h"

#include "REPlatform/Commands/RECommandNotificationObject.h"
#include "REPlatform/Commands/EntityParentChangeCommand.h"
#include "REPlatform/Commands/EntityAddCommand.h"
#include "REPlatform/Commands/EntityRemoveCommand.h"
#include "REPlatform/Commands/ParticleEmitterMoveCommands.h"
#include "REPlatform/Commands/ParticleLayerMoveCommand.h"
#include "REPlatform/Commands/ParticleLayerRemoveCommand.h"
#include "REPlatform/Commands/ParticleForceMoveCommand.h"
#include "REPlatform/Commands/ParticleForceRemoveCommand.h"
#include "REPlatform/Deprecated/SceneValidator.h"

#include <TArc/Core/Deprecated.h>

#include <Engine/Engine.h>
#include <Engine/EngineContext.h>
#include <FileSystem/FileSystem.h>
#include <Scene3D/Components/ComponentHelpers.h>
#include <Scene3D/Components/CustomPropertiesComponent.h>
#include <Scene3D/Components/TransformComponent.h>

namespace DAVA
{
namespace StructSystemDetails
{
void MapSelectableGroup(const SelectableGroup& srcGroup, SelectableGroup& dstGroup,
                        const StructureSystem::InternalMapping& mapping, SceneCollisionSystem* collisionSystem)
{
    DVASSERT(collisionSystem != nullptr);

    for (auto entity : srcGroup.ObjectsOfType<Entity>())
    {
        auto i = mapping.find(entity);
        if (i != mapping.end())
        {
            dstGroup.Add(i->first);
        }
    }
}

struct SortEntity
{
    SortEntity(Entity* entity, uint32 hierarchyIndex)
        : entity(entity)
        , hierarchyIndex(hierarchyIndex)
    {
    }

    Entity* entity = nullptr;
    uint32 hierarchyIndex = 0;

    bool operator<(const SortEntity& e) const
    {
        return hierarchyIndex < e.hierarchyIndex;
    }
};
}

StructureSystem::StructureSystem(Scene* scene)
    : SceneSystem(scene)
{
}

StructureSystem::~StructureSystem()
{
}

void StructureSystem::Move(const SelectableGroup& objects, Entity* newParent, Entity* newBefore, bool saveEntityPositionOnHierarchyChange)
{
    SceneEditor2* sceneEditor = (SceneEditor2*)GetScene();
    const auto& objectsContent = objects.GetContent();
    if ((sceneEditor == nullptr) || objectsContent.empty())
        return;

    sceneEditor->BeginBatch("Move entities", objects.GetSize());
    for (auto entity : objects.ObjectsOfType<Entity>())
    {
        sceneEditor->Exec(std::unique_ptr<Command>(new EntityParentChangeCommand(entity, newParent,
                                                                                 saveEntityPositionOnHierarchyChange,
                                                                                 newBefore)));
    }
    sceneEditor->EndBatch();
    EmitChanged();
}

void StructureSystem::RemoveEntities(Vector<Entity*>& objects)
{
    // sort objects by parents (even if parent == nullptr), in order to remove children first
    std::priority_queue<StructSystemDetails::SortEntity> queue;
    for (Entity* entity : objects)
    {
        uint32 hierarchyIndex = 0;
        Entity* e = entity;
        while (e->GetParent() != nullptr)
        {
            ++hierarchyIndex;
            e = e->GetParent();
        }

        queue.push(StructSystemDetails::SortEntity(entity, hierarchyIndex));
    }

    // check if we still somebody to delete
    if (queue.empty())
        return;

    // actually delete bastards
    SceneEditor2* sceneEditor = (SceneEditor2*)GetScene();
    sceneEditor->BeginBatch("Remove entities", static_cast<uint32>(objects.size()));
    while (!queue.empty())
    {
        StructSystemDetails::SortEntity sortEntity = queue.top();
        queue.pop();
        if (sortEntity.entity->GetNotRemovable() == true)
        {
            continue;
        }

        StructureSystemDelegate* exclusiveRemoveDelegate = nullptr;
        for (StructureSystemDelegate* d : delegates)
        {
            if (d->HasCustomRemovingForEntity(sortEntity.entity) == true)
            {
                DVASSERT(exclusiveRemoveDelegate == nullptr);
                exclusiveRemoveDelegate = d;
            }
        }

        for (StructureSystemDelegate* delegate : delegates)
        {
            delegate->WillRemove(sortEntity.entity);
        }
        if (exclusiveRemoveDelegate != nullptr)
        {
            exclusiveRemoveDelegate->PerformRemoving(sortEntity.entity);
        }
        else
        {
            sceneEditor->Exec(std::unique_ptr<Command>(new EntityRemoveCommand(sortEntity.entity)));
        }
        for (StructureSystemDelegate* delegate : delegates)
        {
            delegate->DidRemoved(sortEntity.entity);
        }
    }
    sceneEditor->EndBatch();
}

void StructureSystem::Remove(const SelectableGroup& objects)
{
    SceneEditor2* sceneEditor = (SceneEditor2*)GetScene();
    if ((nullptr == sceneEditor) || objects.IsEmpty())
        return;

    Vector<Entity*> entitiesToRemove;
    entitiesToRemove.reserve(objects.GetSize());
    for (auto entity : objects.ObjectsOfType<Entity>())
    {
        entitiesToRemove.push_back(entity);
    }
    RemoveEntities(entitiesToRemove);
    EmitChanged();
}

void StructureSystem::MoveEmitter(const Vector<ParticleEmitterInstance*>& emitters, const Vector<ParticleEffectComponent*>& oldEffects, ParticleEffectComponent* newEffect, int dropAfter)
{
    SceneEditor2* sceneEditor = (SceneEditor2*)GetScene();
    if (sceneEditor == nullptr)
        return;

    sceneEditor->BeginBatch("Move particle emitter", static_cast<uint32>(emitters.size()));
    for (size_t i = 0; i < emitters.size(); ++i)
    {
        sceneEditor->Exec(std::unique_ptr<Command>(new ParticleEmitterMoveCommand(oldEffects[i], emitters[i], newEffect, dropAfter++)));
    }
    sceneEditor->EndBatch();
    EmitChanged();
}

void StructureSystem::MoveLayer(const Vector<ParticleLayer*>& layers, const Vector<ParticleEmitterInstance*>& oldEmitters, ParticleEmitterInstance* newEmitter, ParticleLayer* newBefore)
{
    SceneEditor2* sceneEditor = (SceneEditor2*)GetScene();
    if (sceneEditor == nullptr)
        return;

    sceneEditor->BeginBatch("Move particle layers", static_cast<uint32>(layers.size()));
    for (size_t i = 0; i < layers.size(); ++i)
    {
        sceneEditor->Exec(std::unique_ptr<Command>(new ParticleLayerMoveCommand(oldEmitters[i], layers[i], newEmitter, newBefore)));
    }
    sceneEditor->EndBatch();
    EmitChanged();
}

void StructureSystem::MoveSimplifiedForce(const Vector<ParticleForceSimplified*>& forces, const DAVA::Vector<DAVA::ParticleLayer*>& oldLayers, DAVA::ParticleLayer* newLayer)
{
    SceneEditor2* sceneEditor = (SceneEditor2*)GetScene();
    if (sceneEditor == nullptr)
        return;

    sceneEditor->BeginBatch("Move particle simplified force", static_cast<uint32>(forces.size()));
    for (size_t i = 0; i < forces.size(); ++i)
    {
        sceneEditor->Exec(std::unique_ptr<DAVA::Command>(new ParticleSimplifiedForceMoveCommand(forces[i], oldLayers[i], newLayer)));
    }
    sceneEditor->EndBatch();
    EmitChanged();
}

void StructureSystem::MoveParticleForce(const Vector<ParticleForce*>& forces, const Vector<ParticleLayer*>& oldLayers, ParticleLayer* newLayer)
{
    SceneEditor2* sceneEditor = (SceneEditor2*)GetScene();
    if (sceneEditor == nullptr)
        return;

    sceneEditor->BeginBatch("Move particle force", static_cast<uint32>(forces.size()));
    for (size_t i = 0; i < forces.size(); ++i)
    {
        sceneEditor->Exec(std::unique_ptr<Command>(new ParticleForceMoveCommand(forces[i], oldLayers[i], newLayer)));
    }
    sceneEditor->EndBatch();
    EmitChanged();
}

SelectableGroup StructureSystem::ReloadEntities(const SelectableGroup& objects, bool saveLightmapSettings)
{
    if (objects.IsEmpty())
        return SelectableGroup();

    Set<FilePath> refsToReload;

    for (auto entity : objects.ObjectsOfType<Entity>())
    {
        KeyedArchive* props = GetCustomPropertiesArchieve(entity);
        if (props != nullptr)
        {
            FilePath pathToReload(props->GetString(ResourceEditor::EDITOR_REFERENCE_TO_OWNER));
            if (!pathToReload.IsEmpty())
            {
                refsToReload.insert(pathToReload);
            }
        }
    }

    Set<FilePath>::iterator it = refsToReload.begin();
    InternalMapping groupMapping;
    for (; it != refsToReload.end(); ++it)
    {
        InternalMapping mapping;
        ReloadRefs(*it, mapping, saveLightmapSettings);
        groupMapping.insert(mapping.begin(), mapping.end());
    }

    DVASSERT(dynamic_cast<SceneEditor2*>(GetScene()) != nullptr);
    SceneEditor2* scene = static_cast<SceneEditor2*>(GetScene());

    SelectableGroup result;
    StructSystemDetails::MapSelectableGroup(objects, result, groupMapping, scene->GetSystem<SceneCollisionSystem>());
    return result;
}

void StructureSystem::ReloadRefs(const FilePath& modelPath, InternalMapping& mapping, bool saveLightmapSettings)
{
    if (!modelPath.IsEmpty())
    {
        ReloadInternal(mapping, modelPath, saveLightmapSettings);
    }
}

SelectableGroup StructureSystem::ReloadEntitiesAs(const SelectableGroup& objects, const FilePath& newModelPath, bool saveLightmapSettings)
{
    if (objects.IsEmpty())
        return SelectableGroup();

    InternalMapping entitiesToReload;

    for (auto entity : objects.ObjectsOfType<Entity>())
    {
        entitiesToReload.emplace(entity, nullptr);
    }

    ReloadInternal(entitiesToReload, newModelPath, saveLightmapSettings);

    DVASSERT(dynamic_cast<SceneEditor2*>(GetScene()) != nullptr);
    SceneEditor2* scene = static_cast<SceneEditor2*>(GetScene());

    SelectableGroup result;
    StructSystemDetails::MapSelectableGroup(objects, result, entitiesToReload, scene->GetSystem<SceneCollisionSystem>());
    return result;
}

void StructureSystem::ReloadInternal(InternalMapping& mapping, const FilePath& newModelPath, bool saveLightmapSettings)
{
    SceneEditor2* sceneEditor = (SceneEditor2*)GetScene();
    if (NULL != sceneEditor)
    {
        // also we should reload all entities, that already has reference to the same newModelPath
        SearchEntityByRef(GetScene(), newModelPath, [&mapping](Entity* item) {
            mapping.emplace(item, nullptr);
        });

        if (mapping.size() > 0)
        {
            // try to load new model
            Entity* loadedEntity = LoadInternal(newModelPath, true);

            if (NULL != loadedEntity)
            {
                InternalMapping::iterator it = mapping.begin();
                InternalMapping::iterator end = mapping.end();

                sceneEditor->BeginBatch("Reload model", static_cast<uint32>(mapping.size()));

                for (; it != end; ++it)
                {
                    ScopedPtr<Entity> newEntityInstance(loadedEntity->Clone());
                    Entity* origEntity = it->first;

                    if ((origEntity != nullptr) && (newEntityInstance.get() != nullptr) && (origEntity->GetParent() != nullptr))
                    {
                        Entity* before = origEntity->GetParent()->GetNextChild(origEntity);

                        TransformComponent* origTC = origEntity->GetComponent<TransformComponent>();
                        TransformComponent* newTC = newEntityInstance->GetComponent<TransformComponent>();
                        newTC->SetLocalTransform(origTC->GetLocalTransform());
                        newEntityInstance->SetID(origEntity->GetID());
                        newEntityInstance->SetSceneID(origEntity->GetSceneID());
                        newEntityInstance->SetNotRemovable(origEntity->GetNotRemovable());
                        it->second = newEntityInstance;

                        if (saveLightmapSettings)
                        {
                            CopyLightmapSettings(origEntity, newEntityInstance);
                        }

                        sceneEditor->Exec(std::unique_ptr<Command>(new EntityAddCommand(newEntityInstance, origEntity->GetParent(), before)));
                        sceneEditor->Exec(std::unique_ptr<Command>(new EntityRemoveCommand(origEntity)));
                    }
                }

                sceneEditor->EndBatch();
                loadedEntity->Release();
            }
        }
    }
}

void StructureSystem::Add(const FilePath& newModelPath, const Vector3 pos)
{
    SceneEditor2* sceneEditor = static_cast<SceneEditor2*>(GetScene());
    if (nullptr != sceneEditor)
    {
        ScopedPtr<Entity> loadedEntity(Load(newModelPath));
        if (loadedEntity.get() != nullptr)
        {
            Vector3 entityPos = pos;
            if (entityPos.IsZero() && FindLandscape(loadedEntity) == nullptr)
            {
                SceneCameraSystem* cameraSystem = sceneEditor->GetSystem<SceneCameraSystem>();

                Vector3 camDirection = cameraSystem->GetCameraDirection();
                Vector3 camPosition = cameraSystem->GetCameraPosition();

                AABBox3 commonBBox = loadedEntity->GetWTMaximumBoundingBoxSlow();
                float32 bboxSize = 5.0f;

                if (!commonBBox.IsEmpty())
                {
                    bboxSize += (commonBBox.max - commonBBox.min).Length();
                }

                camDirection.Normalize();

                entityPos = camPosition + camDirection * bboxSize;
            }

            TransformComponent* tc = loadedEntity->GetComponent<TransformComponent>();
            tc->SetLocalTranslation(entityPos);

            sceneEditor->Exec(std::unique_ptr<Command>(new EntityAddCommand(loadedEntity, sceneEditor)));

            // TODO: move this code to some another place (into command itself or into ProcessCommand function)
            //
            // Перенести в Load и завалидейтить только подгруженную Entity
            // -->
            SceneValidator validator;
            ProjectManagerData* data = Deprecated::GetDataNode<ProjectManagerData>();
            if (data)
            {
                validator.SetPathForChecking(data->GetProjectPath());
            }
            validator.ValidateScene(sceneEditor, sceneEditor->GetScenePath());
            // <--

            EmitChanged();
        }
    }
}

void StructureSystem::EmitChanged()
{
    // mark that structure was changed. real signal will be emited on next update() call
    // this should done be to increase performance - on Change emit on multiple scene structure operations
    structureChanged = true;
}

void StructureSystem::AddDelegate(StructureSystemDelegate* delegate)
{
    delegates.push_back(delegate);
}

void StructureSystem::RemoveDelegate(StructureSystemDelegate* delegate)
{
    delegates.remove(delegate);
}

void StructureSystem::Process(float32 timeElapsed)
{
    if (structureChanged)
    {
        structureChangedSignal.Emit(static_cast<SceneEditor2*>(GetScene()));
        structureChanged = false;
    }
}

void StructureSystem::ProcessCommand(const RECommandNotificationObject& commandNotification)
{
    if (commandNotification.MatchCommandTypes<ParticleLayerRemoveCommand, ParticleLayerMoveCommand,
                                              ParticleForceRemoveCommand, ParticleForceMoveCommand,
                                              ParticleSimplifiedForceMoveCommand>() == true)
    {
        EmitChanged();
    }
}

void StructureSystem::AddEntity(Entity* entity)
{
    EmitChanged();
}

void StructureSystem::RemoveEntity(Entity* entity)
{
    EmitChanged();
}

void StructureSystem::PrepareForRemove()
{
    EmitChanged();
}

Entity* StructureSystem::Load(const FilePath& sc2path)
{
    return LoadInternal(sc2path, false);
}

Entity* StructureSystem::LoadInternal(const FilePath& sc2path, bool clearCache)
{
    Entity* loadedEntity = nullptr;

    SceneEditor2* sceneEditor = static_cast<SceneEditor2*>(GetScene());
    FileSystem* fs = GetEngineContext()->fileSystem;
    if (nullptr != sceneEditor && sc2path.IsEqualToExtension(".sc2") && fs->Exists(sc2path))
    {
        if (clearCache)
        {
            // if there is already entity for such file, we should release it
            // to be sure that latest version will be loaded
            sceneEditor->cache.Clear(sc2path);
        }

        loadedEntity = sceneEditor->cache.GetClone(sc2path);
        if (nullptr != loadedEntity)
        {
            // this is for backward compatibility.
            // sceneFileV2 will remove empty nodes only
            // if there is parent for such nodes.
            {
                ScopedPtr<SceneFileV2> tmpSceneFile(new SceneFileV2());
                ScopedPtr<Entity> tmpParent(new Entity());
                Entity* tmpEntity = loadedEntity;

                tmpParent->AddNode(tmpEntity);
                tmpSceneFile->RemoveEmptyHierarchy(tmpEntity);

                loadedEntity = SafeRetain(tmpParent->GetChild(0));

                SafeRelease(tmpEntity);
            }

            KeyedArchive* props = GetOrCreateCustomProperties(loadedEntity)->GetArchive();
            props->SetString(ResourceEditor::EDITOR_REFERENCE_TO_OWNER, sc2path.GetAbsolutePathname());

            SceneValidator::ExtractEmptyRenderObjects(loadedEntity);
        }
    }
    else
    {
        Logger::Error("Wrong extension or no such file: %s", sc2path.GetAbsolutePathname().c_str());
    }

    return loadedEntity;
}

void StructureSystem::CopyLightmapSettings(NMaterial* fromState, NMaterial* toState) const
{
    if (fromState->HasLocalTexture(NMaterialTextureName::TEXTURE_LIGHTMAP))
    {
        Texture* lightmap = fromState->GetLocalTexture(NMaterialTextureName::TEXTURE_LIGHTMAP);
        if (toState->HasLocalTexture(NMaterialTextureName::TEXTURE_LIGHTMAP))
            toState->SetTexture(NMaterialTextureName::TEXTURE_LIGHTMAP, lightmap);
        else
            toState->AddTexture(NMaterialTextureName::TEXTURE_LIGHTMAP, lightmap);
    }

    if (fromState->HasLocalProperty(NMaterialParamName::PARAM_UV_SCALE))
    {
        const float* data = fromState->GetLocalPropValue(NMaterialParamName::PARAM_UV_SCALE);
        if (toState->HasLocalProperty(NMaterialParamName::PARAM_UV_SCALE))
            toState->SetPropertyValue(NMaterialParamName::PARAM_UV_SCALE, data);
        else
            toState->AddProperty(NMaterialParamName::PARAM_UV_SCALE, data, rhi::ShaderProp::TYPE_FLOAT2);
    }

    if (fromState->HasLocalProperty(NMaterialParamName::PARAM_UV_OFFSET))
    {
        const float* data = fromState->GetLocalPropValue(NMaterialParamName::PARAM_UV_OFFSET);
        if (toState->HasLocalProperty(NMaterialParamName::PARAM_UV_OFFSET))
            toState->SetPropertyValue(NMaterialParamName::PARAM_UV_OFFSET, data);
        else
            toState->AddProperty(NMaterialParamName::PARAM_UV_OFFSET, data, rhi::ShaderProp::TYPE_FLOAT2);
    }
}

struct BatchInfo
{
    BatchInfo()
        : switchIndex(-1)
        , lodIndex(-1)
        , batch(NULL)
    {
    }

    int32 switchIndex;
    int32 lodIndex;

    RenderBatch* batch;
};

struct SortBatches
{
    bool operator()(const BatchInfo& b1, const BatchInfo& b2)
    {
        if (b1.switchIndex == b2.switchIndex)
        {
            return b1.lodIndex < b2.lodIndex;
        }

        return b1.switchIndex < b2.switchIndex;
    }
};

void CreateBatchesInfo(RenderObject* object, Vector<BatchInfo>& batches)
{
    if (!object)
        return;

    uint32 batchesCount = object->GetRenderBatchCount();
    for (uint32 i = 0; i < batchesCount; ++i)
    {
        BatchInfo info;
        info.batch = object->GetRenderBatch(i, info.lodIndex, info.switchIndex);
        batches.push_back(info);
    }

    std::sort(batches.begin(), batches.end(), SortBatches());
}

bool StructureSystem::CopyLightmapSettings(Entity* fromEntity, Entity* toEntity) const
{
    Vector<RenderObject*> fromMeshes;
    FindMeshesRecursive(fromEntity, fromMeshes);

    Vector<RenderObject*> toMeshes;
    FindMeshesRecursive(toEntity, toMeshes);

    if (fromMeshes.size() == toMeshes.size())
    {
        uint32 meshCount = (uint32)fromMeshes.size();
        for (uint32 m = 0; m < meshCount; ++m)
        {
            Vector<BatchInfo> fromBatches;
            CreateBatchesInfo(fromMeshes[m], fromBatches);

            Vector<BatchInfo> toBatches;
            CreateBatchesInfo(toMeshes[m], toBatches);

            uint32 rbFromCount = fromMeshes[m]->GetRenderBatchCount();
            uint32 rbToCount = toMeshes[m]->GetRenderBatchCount();

            for (uint32 from = 0, to = 0; from < rbFromCount && to < rbToCount;)
            {
                BatchInfo& fromBatch = fromBatches[from];
                BatchInfo& toBatch = toBatches[to];

                if (fromBatch.switchIndex == toBatch.switchIndex)
                {
                    if (fromBatch.lodIndex <= toBatch.lodIndex)
                    {
                        for (uint32 usedToIndex = to; usedToIndex < rbToCount; ++usedToIndex)
                        {
                            BatchInfo& usedToBatch = toBatches[usedToIndex];

                            if ((fromBatch.switchIndex != usedToBatch.switchIndex))
                                break;

                            PolygonGroup* fromPG = fromBatch.batch->GetPolygonGroup();
                            PolygonGroup* toPG = usedToBatch.batch->GetPolygonGroup();

                            uint32 fromSize = fromPG->GetVertexCount() * fromPG->vertexStride;
                            uint32 toSize = toPG->GetVertexCount() * toPG->vertexStride;
                            if ((fromSize == toSize) && (0 == Memcmp(fromPG->meshData, toPG->meshData, fromSize)))
                            {
                                CopyLightmapSettings(fromBatch.batch->GetMaterial(), usedToBatch.batch->GetMaterial());
                            }
                        }

                        ++from;
                    }
                    else if (fromBatch.lodIndex < toBatch.lodIndex)
                    {
                        ++from;
                    }
                    else
                    {
                        ++to;
                    }
                }
                else if (fromBatch.switchIndex < toBatch.switchIndex)
                {
                    ++from;
                }
                else
                {
                    ++to;
                }
            }
        }

        return true;
    }

    return false;
}

void StructureSystem::FindMeshesRecursive(Entity* entity, Vector<RenderObject*>& objects) const
{
    RenderObject* ro = GetRenderObject(entity);
    if (ro && ro->GetType() == RenderObject::TYPE_MESH)
    {
        objects.push_back(ro);
    }

    int32 count = entity->GetChildrenCount();
    for (int32 i = 0; i < count; ++i)
    {
        FindMeshesRecursive(entity->GetChild(i), objects);
    }
}

void StructureSystem::SearchEntityByRef(Entity* parent, const FilePath& refToOwner, const Function<void(Entity*)>& callback)
{
    DVASSERT(callback);
    if (NULL != parent)
    {
        for (int i = 0; i < parent->GetChildrenCount(); ++i)
        {
            Entity* entity = parent->GetChild(i);
            KeyedArchive* arch = GetCustomPropertiesArchieve(entity);

            if (arch)
            {
                // if this entity has searched reference - add it to the set
                if (FilePath(arch->GetString(ResourceEditor::EDITOR_REFERENCE_TO_OWNER, "")) == refToOwner)
                {
                    callback(entity);
                    continue;
                }
            }

            // else continue searching in child entities
            SearchEntityByRef(entity, refToOwner, callback);
        }
    }
}
} // namespace DAVA
