#pragma once

#include "REPlatform/Commands/CommandNotify.h"
#include "REPlatform/Commands/Private/RECommandStack.h"
#include "REPlatform/Scene/Utils/SceneExporter.h"

#include <FileSystem/FilePath.h>
#include <Functional/Signal.h>
#include <Reflection/Reflection.h>
#include <Render/Renderer.h>
#include <Scene3D/Scene.h>
#include <Scene3D/Systems/Controller/RotationControllerSystem.h>
#include <Scene3D/Systems/Controller/WASDControllerSystem.h>
#include <Scene3D/Systems/StaticOcclusionBuildSystem.h>

namespace DAVA
{
class RECommandNotificationObject;
class RECommandStack;
class EditorSceneSystem;
class Command;
class PropertiesHolder;
class ContextAccessor;

class SceneEditor2 : public Scene
{
public:
    enum LandscapeTools : uint32
    {
        LANDSCAPE_TOOL_CUSTOM_COLOR = 1 << 0,
        LANDSCAPE_TOOL_HEIGHTMAP_EDITOR = 1 << 1,
        LANDSCAPE_TOOL_TILEMAP_EDITOR = 1 << 2,
        LANDSCAPE_TOOL_RULER = 1 << 3,
        LANDSCAPE_TOOL_NOT_PASSABLE_TERRAIN = 1 << 4,

        LANDSCAPE_TOOLS_ALL = LANDSCAPE_TOOL_CUSTOM_COLOR | LANDSCAPE_TOOL_HEIGHTMAP_EDITOR | LANDSCAPE_TOOL_TILEMAP_EDITOR |
        LANDSCAPE_TOOL_RULER |
        LANDSCAPE_TOOL_NOT_PASSABLE_TERRAIN
    };

    SceneEditor2();
    ~SceneEditor2() override;

    //to manage editor systems adding/deleting
    //to manage editor systems adding/deleting
    void AddSystem(SceneSystem* sceneSystem,
                   const DAVA::ComponentMask& componentFlags,
                   uint32 processFlags = 0,
                   SceneSystem* insertBeforeSceneForProcess = nullptr,
                   SceneSystem* insertBeforeSceneForInput = nullptr,
                   SceneSystem* insertBeforeSceneForFixedProcess = nullptr) override;
    void RemoveSystem(SceneSystem* sceneSystem) override;

    bool AcquireInputLock(EditorSceneSystem* system);
    void ReleaseInputLock(EditorSceneSystem* system);

    // save/load
    SceneFileV2::eError LoadScene(const FilePath& path) override;
    SceneFileV2::eError SaveScene(const FilePath& pathname, bool saveForGame = false) override;
    SceneFileV2::eError SaveScene();
    bool Export(const SceneExporter::Params& exportingParams);

    void SaveEmitters(const Function<FilePath(const String& /*entityName*/, const String& /*emitterName*/)>& getEmitterPathFn);

    const FilePath& GetScenePath() const;
    void SetScenePath(const FilePath& newScenePath);

    // commands
    bool CanUndo() const;
    bool CanRedo() const;

    String GetUndoText() const;
    String GetRedoText() const;

    void Undo();
    void Redo();

    void BeginBatch(const String& text, uint32 commandsCount = 1);
    void EndBatch();

    void Exec(std::unique_ptr<Command>&& command);

    void ClearAllCommands();
    const RECommandStack* GetCommandStack() const;

    template <typename... Args>
    void RemoveCommands()
    {
        commandStack->RemoveCommands<Args...>();
    }

    Signal<SceneEditor2*, const RECommandNotificationObject&> commandExecuted;

    // checks whether the scene changed since the last save
    bool IsLoaded() const;
    bool IsChanged() const;
    void SetChanged();

    // enable/disable drawing custom HUD
    void SetHUDVisible(bool visible);
    bool IsHUDVisible() const;

    // DAVA events
    void Update(float timeElapsed) override;
    void Draw() override;

    // this function should be called each time UI3Dview changes its position
    // viewport rect is used to calc. ray from camera to any 2d point on this viewport
    void SetViewportRect(const Rect& newViewportRect);

    //Insert entity to begin of scene hierarchy to display editor entities at one place on top og scene tree
    void AddEditorEntity(Entity* editorEntity);

    const RenderStats& GetRenderStats() const;

    void EnableToolsInstantly(int32 toolFlags);
    void DisableToolsInstantly(int32 toolFlags, bool saveChanges = true);
    bool IsToolsEnabled(int32 toolFlags);
    int32 GetEnabledTools();

    SceneEditor2* CreateCopyForExport(); //Need to prevent changes of original scene
    Entity* Clone(Entity* dstNode /* = NULL */) override;

    void EnableEditorSystems();
    void LoadSystemsLocalProperties(DAVA::PropertiesHolder* holder, ContextAccessor* accessor);
    void SaveSystemsLocalProperties(DAVA::PropertiesHolder* holder);

    uint32 GetFramesCount() const;
    void ResetFramesCount();

    DAVA_DEPRECATED(void MarkAsChanged()); // for old material & particle editors

protected:
    bool isLoaded = false;
    bool isHUDVisible = true;

    FilePath curScenePath;
    std::unique_ptr<RECommandStack> commandStack;
    RenderStats renderStats;

    Vector<EditorSceneSystem*> editorSystems;
    Vector<EditorSceneSystem*> landscapeEditorSystems;
    Vector<Entity*> editorEntities;
    EditorSceneSystem* inputLockedByThis = nullptr;

    void AccumulateDependentCommands(REDependentCommandsHolder& holder);
    void EditorCommandProcess(const RECommandNotificationObject& commandNotification);

    void ExtractEditorEntities();
    void InjectEditorEntities();

    void RemoveSystems();

    void Setup3DDrawing();

    uint32 framesCount = 0;

private:
    friend struct EditorCommandNotify;

    class EditorCommandNotify : public CommandNotify
    {
    public:
        EditorCommandNotify(SceneEditor2* _editor);

        void AccumulateDependentCommands(REDependentCommandsHolder& holder) override;
        void Notify(const RECommandNotificationObject& commandNotification) override;

    private:
        SceneEditor2* editor = nullptr;
    };

    DAVA_VIRTUAL_REFLECTION(SceneEditor2, Scene);
};

} // namespace DAVA
