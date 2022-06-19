#pragma once

#include "REPlatform/Scene/Systems/EditorSceneSystem.h"
#include "REPlatform/Scene/SceneTypes.h"

#include "REPlatform/Commands/CreatePlaneLODCommandHelper.h"

#include <Entity/SceneSystem.h>
#include <Scene3D/Lod/LodComponent.h>

namespace DAVA
{
class Entity;
class RenderObject;
class RECommandNotificationObject;
class SelectableGroup;

struct ForceValues
{
    enum eApplyFlag : uint32
    {
        APPLY_DISTANCE = 1 << 0,
        APPLY_LAYER = 1 << 1,

        APPLY_NONE = 0,
        APPLY_ALL = APPLY_DISTANCE | APPLY_LAYER,

        APPLY_DEFAULT = APPLY_DISTANCE
    };

    ForceValues(float32 distance_ = LodComponent::INVALID_DISTANCE,
                int32 layer_ = LodComponent::INVALID_LOD_LAYER,
                eApplyFlag flag_ = APPLY_DEFAULT)
        : distance(distance_)
        , layer(layer_)
        , flag(flag_)
    {
    }

    float32 distance;
    int32 layer;
    eApplyFlag flag;
};

class SceneEditor2;
class EditorLODSystem;
class LODComponentHolder
{
    friend class EditorLODSystem;

public:
    LODComponentHolder();

    int32 GetMaxLODLayer() const;
    uint32 GetLODLayersCount() const;

    const Vector<float32>& GetDistances() const;
    const Vector<bool>& GetMultiple() const;

protected:
    void BindToSystem(EditorLODSystem* system, SceneEditor2* scene);

    void SummarizeValues();
    void PropagateValues();

    void ApplyForce(const ForceValues& force);
    bool DeleteLOD(int32 layer);
    bool CopyLod(int32 from, int32 to);

    Vector<LodComponent*> lodComponents;
    Vector<float32> distances;
    Vector<bool> isMultiple;
    Vector<bool> isChanged;

    int32 maxLodLayerIndex = LodComponent::INVALID_LOD_LAYER;

    EditorLODSystem* system = nullptr;
    SceneEditor2* scene = nullptr;
};

class EditorLODSystemUIDelegate;
class EditorLODSystem : public SceneSystem, public EditorSceneSystem
{
    friend class SceneEditor2;

    enum eLODSystemFlag : uint32
    {
        FLAG_MODE = 1 << 0,
        FLAG_FORCE = 1 << 1,
        FLAG_DISTANCE = 1 << 2,
        FLAG_ACTION = 1 << 3,

        FLAG_NONE = 0,
        FLAG_ALL = FLAG_MODE | FLAG_FORCE | FLAG_DISTANCE | FLAG_ACTION
    };

public:
    static const int32 LAST_LOD_LAYER = 0x7fffffff;
    static const float32 LOD_DISTANCE_INFINITY;

    EditorLODSystem(Scene* scene);
    ~EditorLODSystem() override;

    void AddEntity(Entity* entity) override;
    void RemoveEntity(Entity* entity) override;
    void AddComponent(Entity* entity, Component* component) override;
    void RemoveComponent(Entity* entity, Component* component) override;
    void PrepareForRemove() override;

    void Process(float32 timeElapsed) override;

    void SceneDidLoaded() override;

    eEditorMode GetMode() const;
    void SetMode(eEditorMode mode);

    bool GetRecursive() const;
    void SetRecursive(bool recursive);

    //actions
    bool CanDeleteLOD() const;
    bool CanCreateLOD() const;

    void DeleteLOD(int32 layer);

    void CreatePlaneLOD(int32 fromLayer, uint32 textureSize, const FilePath& texturePath);
    void CopyLastLODToFirst();
    //end of actions

    const ForceValues& GetForceValues() const;
    void SetForceValues(const ForceValues& values);

    const LODComponentHolder* GetActiveLODData() const;

    void SetLODDistances(const Vector<float32>& distances);

    //scene signals
    void SelectionChanged(const SelectableGroup& selection);

    void AddDelegate(EditorLODSystemUIDelegate* uiDelegate);
    void RemoveDelegate(EditorLODSystemUIDelegate* uiDelegate);

    FilePath GetPathForPlaneEntity() const;

    static bool IsFitModeEnabled(const Vector<float32>& distances);

protected:
    void ProcessCommand(const RECommandNotificationObject& commandNotification) override;

private:
    void RecalculateData();
    //actions
    void CopyLOD(int32 fromLayer, int32 toLayer);

    //signals
    void EmitInvalidateUI(uint32 flags);
    void DispatchSignals();
    //signals

    void ProcessAddedEntities();
    void ProcessPlaneLODs();

    LODComponentHolder lodData[eEditorMode::MODE_COUNT];
    LODComponentHolder* activeLodData = nullptr;
    ForceValues forceValues;
    eEditorMode mode = eEditorMode::MODE_DEFAULT;

    Vector<CreatePlaneLODCommandHelper::RequestPointer> planeLODRequests;

    bool generateCommands = false;

    Vector<EditorLODSystemUIDelegate*> uiDelegates;
    uint32 invalidateUIFlag = FLAG_NONE;

    Vector<std::pair<Entity*, LodComponent*>> componentsToAdd;

    bool recursive = false;
    DAVA::Array<bool, eEditorMode::MODE_COUNT> pendingSummarizeValues;
};

class EditorLODSystemUIDelegate
{
public:
    virtual ~EditorLODSystemUIDelegate() = default;

    virtual void UpdateModeUI(EditorLODSystem* forSystem, const eEditorMode mode, bool recursive)
    {
    }
    virtual void UpdateForceUI(EditorLODSystem* forSystem, const ForceValues& forceValues)
    {
    }
    virtual void UpdateDistanceUI(EditorLODSystem* forSystem, const LODComponentHolder* lodData)
    {
    }
    virtual void UpdateActionUI(EditorLODSystem* forSystem)
    {
    }
};
} // namespace DAVA
