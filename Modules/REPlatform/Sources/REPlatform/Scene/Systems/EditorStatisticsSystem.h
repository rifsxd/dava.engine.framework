#pragma once

#include "REPlatform/Scene/SceneTypes.h"
#include "REPlatform/Scene/Systems/EditorSceneSystem.h"

#include <TArc/Core/FieldBinder.h>
#include <TArc/DataProcessing/SettingsNode.h>

#include <Base/BaseTypes.h>
#include <Entity/SceneSystem.h>

namespace DAVA
{
class Entity;
class RenderObject;
class RenderComponent;
class EditorStatisticsSystemUIDelegate;
class Camera;

struct TrianglesData;

class RenderStatsSettings : public DAVA::SettingsNode
{
public:
    bool calculatePerFrame = true;

    DAVA_VIRTUAL_REFLECTION(RenderStatsSettings, DAVA::SettingsNode);
};

class EditorStatisticsSystem : public SceneSystem, public EditorSceneSystem
{
    enum eStatisticsSystemFlag : uint32
    {
        FLAG_TRIANGLES = 1 << 0,
        FLAG_NONE = 0
    };

public:
    static const int32 INDEX_OF_ALL_LODS_TRIANGLES = 0;
    static const int32 INDEX_OF_FIRST_LOD_TRIANGLES = 1;

    EditorStatisticsSystem(Scene* scene);

    void RegisterEntity(Entity* entity) override;
    void UnregisterEntity(Entity* entity) override;

    void RegisterComponent(Entity* entity, Component* component) override;
    void UnregisterComponent(Entity* entity, Component* component) override;
    void PrepareForRemove() override;

    void Process(float32 timeElapsed) override;

    const Vector<uint32>& GetTriangles(eEditorMode mode, bool allTriangles);

    void AddDelegate(EditorStatisticsSystemUIDelegate* uiDelegate);
    void RemoveDelegate(EditorStatisticsSystemUIDelegate* uiDelegate);

private:
    void CalculateTriangles();
    void ClipSelection(Camera* camera, Vector<RenderObject*>& selection,
                       Vector<RenderObject*>& visibilityArray, uint32 visibilityCriteria);

    //signals
    void EmitInvalidateUI(uint32 flags);
    void DispatchSignals();
    //signals

    void ProcessCommand(const RECommandNotificationObject& commandNotification) override;

private:
    Vector<TrianglesData> triangles;

    Vector<EditorStatisticsSystemUIDelegate*> uiDelegates;
    uint32 invalidateUIflag = FLAG_NONE;
    std::unique_ptr<DAVA::FieldBinder> binder;
    bool calculatePerFrame = true;
    bool initialized = false;
};

class EditorStatisticsSystemUIDelegate
{
public:
    virtual ~EditorStatisticsSystemUIDelegate() = default;

    virtual void UpdateTrianglesUI(EditorStatisticsSystem* forSystem){};
};
} //namespace DAVA
