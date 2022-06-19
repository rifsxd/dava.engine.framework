#pragma once

#include "REPlatform/DataNodes/SelectableGroup.h"
#include "REPlatform/Global/StringConstants.h"
#include "REPlatform/Scene/Systems/SystemDelegates.h"
#include "REPlatform/Scene/Systems/EditorSceneSystem.h"

#include <Entity/SceneSystem.h>
#include <Functional/Function.h>
#include <Particles/ParticleEmitter.h>
#include <Particles/ParticleLayer.h>
#include <Render/Highlevel/Landscape.h>
#include <Scene3D/Components/ParticleEffectComponent.h>
#include <Scene3D/Entity.h>
#include <UI/UIEvent.h>

namespace DAVA
{
class RECommandNotificationObject;
class SceneEditor2;
class StructureSystem : public SceneSystem, public EditorSceneSystem
{
public:
    using InternalMapping = Map<Entity*, Entity*>;

public:
    StructureSystem(Scene* scene);
    ~StructureSystem();

    void Process(float32 timeElapsed) override;

    void Move(const SelectableGroup& objects, Entity* newParent, Entity* newBefore, bool saveEntityPositionOnHierarchyChange);
    void MoveEmitter(const Vector<ParticleEmitterInstance*>& emitters, const Vector<ParticleEffectComponent*>& oldEffects, ParticleEffectComponent* newEffect, int dropAfter);
    void MoveLayer(const Vector<ParticleLayer*>& layers, const Vector<ParticleEmitterInstance*>& oldEmitters, ParticleEmitterInstance* newEmitter, ParticleLayer* newBefore);
    void MoveSimplifiedForce(const Vector<ParticleForceSimplified*>& forces, const Vector<ParticleLayer*>& oldLayers, ParticleLayer* newLayer);
    void MoveParticleForce(const Vector<ParticleForce*>& forces, const Vector<ParticleLayer*>& oldLayers, ParticleLayer* newLayer);

    void Remove(const SelectableGroup& objects);

    SelectableGroup ReloadEntities(const SelectableGroup& objects, bool saveLightmapSettings = false);

    // Mapping is link between old entity and new entity
    void ReloadRefs(const FilePath& modelPath, InternalMapping& mapping, bool saveLightmapSettings = false);
    SelectableGroup ReloadEntitiesAs(const SelectableGroup& objects, const FilePath& newModelPath, bool saveLightmapSettings = false);
    void Add(const FilePath& newModelPath, const Vector3 pos = Vector3());

    void EmitChanged();

    Entity* Load(const FilePath& sc2path);

    void AddDelegate(StructureSystemDelegate* delegate);
    void RemoveDelegate(StructureSystemDelegate* delegate);

    Signal<SceneEditor2*> structureChangedSignal;

protected:
    void ProcessCommand(const RECommandNotificationObject& commandNotification) override;

private:
    void AddEntity(Entity* entity) override;
    void RemoveEntity(Entity* entity) override;
    void PrepareForRemove() override;

    void ReloadInternal(InternalMapping& mapping, const FilePath& newModelPath, bool saveLightmapSettings);
    Entity* LoadInternal(const FilePath& sc2path, bool clearCached);

    bool CopyLightmapSettings(Entity* fromState, Entity* toState) const;
    void CopyLightmapSettings(NMaterial* fromEntity, NMaterial* toEntity) const;
    void FindMeshesRecursive(Entity* entity, Vector<RenderObject*>& objects) const;

    void SearchEntityByRef(Entity* parent, const FilePath& refToOwner, const Function<void(Entity*)>& callback);

    void RemoveEntities(Vector<Entity*>& entitiesToRemove);

    List<StructureSystemDelegate*> delegates;
    bool structureChanged = false;
};
} // namespace DAVA