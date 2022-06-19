#include "REPlatform/Scene/SceneEditor2.h"

#include "REPlatform/Deprecated/SceneValidator.h"
#include "REPlatform/Commands/CustomColorsCommands2.h"
#include "REPlatform/Commands/HeightmapEditorCommands2.h"
#include "REPlatform/Commands/LandscapeToolsToggleCommand.h"
#include "REPlatform/Commands/RECommandNotificationObject.h"
#include "REPlatform/Commands/TilemaskEditorCommands.h"

#include "REPlatform/Scene/Systems/BeastSystem.h"
#include "REPlatform/Scene/Systems/CameraSystem.h"
#include "REPlatform/Scene/Systems/CollisionSystem.h"
#include "REPlatform/Scene/Systems/CustomColorsSystem.h"
#include "REPlatform/Scene/Systems/EditorLightSystem.h"
#include "REPlatform/Scene/Systems/EditorLODSystem.h"
#include "REPlatform/Scene/Systems/EditorMaterialSystem.h"
#include "REPlatform/Scene/Systems/EditorParticlesSystem.h"
#include "REPlatform/Scene/Systems/EditorSceneSystem.h"
#include "REPlatform/Scene/Systems/EditorStatisticsSystem.h"
#include "REPlatform/Scene/Systems/EditorVegetationSystem.h"
#include "REPlatform/Scene/Systems/GridSystem.h"
#include "REPlatform/Scene/Systems/HeightmapEditorSystem.h"
#include "REPlatform/Scene/Systems/HoodSystem.h"
#include "REPlatform/Scene/Systems/PathSystem.h"
#include "REPlatform/Scene/Systems/RulerToolSystem.h"
#include "REPlatform/Scene/Systems/StructureSystem.h"
#include "REPlatform/Scene/Systems/TextDrawSystem.h"
#include "REPlatform/Scene/Systems/TilemaskEditorSystem.h"
#include "REPlatform/Scene/Systems/VisibilityCheckSystem.h"
#include "REPlatform/Scene/Systems/WayEditSystem.h"
#include "REPlatform/Scene/Utils/SceneExporter.h"
#include "REPlatform/DataNodes/ProjectManagerData.h"

#include <TArc/Utils/RenderContextGuard.h>
#include <TArc/Core/ContextAccessor.h>

#include <QtTools/ConsoleWidget/PointerSerializer.h>

// framework
#include <Base/BaseTypes.h>
#include <Command/Command.h>
#include <Debug/DVAssert.h>
#include <Engine/Engine.h>
#include <Entity/ComponentUtils.h>
#include <FileSystem/FileSystem.h>
#include <Reflection/ReflectionRegistrator.h>
#include <Render/Highlevel/RenderBatchArray.h>
#include <Render/Highlevel/RenderPass.h>
#include <Scene3D/Components/CameraComponent.h>
#include <Scene3D/Components/ComponentHelpers.h>
#include <Scene3D/Components/Controller/RotationControllerComponent.h>
#include <Scene3D/Components/Controller/SnapToLandscapeControllerComponent.h>
#include <Scene3D/Components/Controller/WASDControllerComponent.h>
#include <Scene3D/Components/LightComponent.h>
#include <Scene3D/Components/ParticleEffectComponent.h>
#include <Scene3D/Components/RenderComponent.h>
#include <Scene3D/Components/StaticOcclusionComponent.h>
#include <Scene3D/Components/TransformComponent.h>
#include <Scene3D/Components/VisibilityCheckComponent.h>
#include <Scene3D/Components/Waypoint/PathComponent.h>
#include <Scene3D/Components/Waypoint/WaypointComponent.h>
#include <Scene3D/Entity.h>
#include <Scene3D/Lod/LodComponent.h>
#include <Scene3D/SceneFileV2.h>
#include <Scene3D/Systems/Controller/SnapToLandscapeControllerSystem.h>
#include <Scene3D/Systems/RenderUpdateSystem.h>
#include <Scene3D/Systems/StaticOcclusionSystem.h>
#include <Scene3D/Systems/TransformSystem.h>
#include <Utils/Utils.h>

#include <time.h>

namespace DAVA
{
namespace SceneEditorDetail
{
struct EmitterDescriptor
{
    EmitterDescriptor(ParticleEmitter* emitter_, ParticleLayer* layer, FilePath path, String name)
        : emitter(emitter_)
        , ownerLayer(layer)
        , yamlPath(path)
        , entityName(name)
    {
    }

    ParticleEmitter* emitter = nullptr;
    ParticleLayer* ownerLayer = nullptr;
    FilePath yamlPath;
    String entityName;
};

void CollectEmittersForSave(ParticleEmitter* topLevelEmitter, List<EmitterDescriptor>& emitters, const String& entityName)
{
    DVASSERT(topLevelEmitter != nullptr);

    for (auto& layer : topLevelEmitter->layers)
    {
        if (nullptr != layer->innerEmitter)
        {
            CollectEmittersForSave(layer->innerEmitter->GetEmitter(), emitters, entityName);
            emitters.emplace_back(EmitterDescriptor(layer->innerEmitter->GetEmitter(), layer, layer->innerEmitter->GetEmitter()->configPath, entityName));
        }
    }

    emitters.emplace_back(topLevelEmitter, nullptr, topLevelEmitter->configPath, entityName);
}
}

SceneEditor2::SceneEditor2()
    : Scene()
    , commandStack(new RECommandStack())
{
    DVASSERT(Engine::Instance()->IsConsoleMode() == false);

    ScopedPtr<EditorCommandNotify> notify(new EditorCommandNotify(this));
    commandStack->SetNotify(notify);

    AddSystem(new SceneGridSystem(this), 0, SCENE_SYSTEM_REQUIRE_PROCESS, renderUpdateSystem);
    AddSystem(new SceneCameraSystem(this), ComponentUtils::MakeMask<CameraComponent>(), SCENE_SYSTEM_REQUIRE_PROCESS | SCENE_SYSTEM_REQUIRE_INPUT, transformSystem);
    AddSystem(new RotationControllerSystem(this), ComponentUtils::MakeMask<CameraComponent, RotationControllerComponent>(), SCENE_SYSTEM_REQUIRE_PROCESS | SCENE_SYSTEM_REQUIRE_INPUT);
    AddSystem(new SnapToLandscapeControllerSystem(this), ComponentUtils::MakeMask<CameraComponent, SnapToLandscapeControllerComponent>(), SCENE_SYSTEM_REQUIRE_PROCESS);
    AddSystem(new WASDControllerSystem(this), ComponentUtils::MakeMask<CameraComponent, WASDControllerComponent>(), SCENE_SYSTEM_REQUIRE_PROCESS);
    AddSystem(new SceneCollisionSystem(this), 0, SCENE_SYSTEM_REQUIRE_PROCESS | SCENE_SYSTEM_REQUIRE_INPUT, renderUpdateSystem);
    AddSystem(new HoodSystem(this), 0, SCENE_SYSTEM_REQUIRE_PROCESS | SCENE_SYSTEM_REQUIRE_INPUT, renderUpdateSystem);
    AddSystem(new EntityModificationSystem(this), 0, SCENE_SYSTEM_REQUIRE_INPUT, renderUpdateSystem);
    AddSystem(new LandscapeEditorDrawSystem(this), 0, SCENE_SYSTEM_REQUIRE_PROCESS, renderUpdateSystem);
    AddSystem(new HeightmapEditorSystem(this), 0, SCENE_SYSTEM_REQUIRE_PROCESS | SCENE_SYSTEM_REQUIRE_INPUT, renderUpdateSystem);
    AddSystem(new TilemaskEditorSystem(this), 0, SCENE_SYSTEM_REQUIRE_PROCESS | SCENE_SYSTEM_REQUIRE_INPUT, renderUpdateSystem);
    AddSystem(new CustomColorsSystem(this), 0, SCENE_SYSTEM_REQUIRE_PROCESS | SCENE_SYSTEM_REQUIRE_INPUT, renderUpdateSystem);
    AddSystem(new RulerToolSystem(this), 0, SCENE_SYSTEM_REQUIRE_PROCESS | SCENE_SYSTEM_REQUIRE_INPUT, renderUpdateSystem);
    AddSystem(new StructureSystem(this), 0, SCENE_SYSTEM_REQUIRE_PROCESS, renderUpdateSystem);
    AddSystem(new EditorParticlesSystem(this), ComponentUtils::MakeMask<ParticleEffectComponent>(), 0, renderUpdateSystem);
    AddSystem(new TextDrawSystem(this), 0, 0, renderUpdateSystem);
    AddSystem(new EditorLightSystem(this), ComponentUtils::MakeMask<LightComponent>(), SCENE_SYSTEM_REQUIRE_PROCESS, transformSystem);
    AddSystem(new BeastSystem(this), 0);
    AddSystem(new StaticOcclusionBuildSystem(this), ComponentUtils::MakeMask<StaticOcclusionComponent, TransformComponent>(), SCENE_SYSTEM_REQUIRE_PROCESS, renderUpdateSystem);
    AddSystem(new EditorMaterialSystem(this), ComponentUtils::MakeMask<RenderComponent>(), SCENE_SYSTEM_REQUIRE_PROCESS, renderUpdateSystem);
    AddSystem(new WayEditSystem(this), ComponentUtils::MakeMask<WaypointComponent, TransformComponent>(), SCENE_SYSTEM_REQUIRE_PROCESS | SCENE_SYSTEM_REQUIRE_INPUT);
    AddSystem(new PathSystem(this), ComponentUtils::MakeMask<PathComponent>(), SCENE_SYSTEM_REQUIRE_PROCESS);
    AddSystem(new EditorLODSystem(this), ComponentUtils::MakeMask<LodComponent>(), SCENE_SYSTEM_REQUIRE_PROCESS);
    AddSystem(new EditorStatisticsSystem(this), ComponentUtils::MakeMask<RenderComponent>(), SCENE_SYSTEM_REQUIRE_PROCESS);
    AddSystem(new VisibilityCheckSystem(this), ComponentUtils::MakeMask<VisibilityCheckComponent>(), SCENE_SYSTEM_REQUIRE_PROCESS);
    AddSystem(new EditorVegetationSystem(this), ComponentUtils::MakeMask<RenderComponent>(), 0);

    WayEditSystem* wayEditSystem = GetSystem<WayEditSystem>();
    PathSystem* pathSystem = GetSystem<PathSystem>();
    EntityModificationSystem* modifSystem = GetSystem<EntityModificationSystem>();

    GetSystem<StructureSystem>()->AddDelegate(wayEditSystem);
    modifSystem->AddDelegate(pathSystem);
    modifSystem->AddDelegate(wayEditSystem);

    GetSystem<LandscapeEditorDrawSystem>()->EnableSystem();
}

SceneEditor2::~SceneEditor2()
{
    RenderContextGuard guard;
    commandStack.reset();
    RemoveSystems();
}

SceneFileV2::eError SceneEditor2::LoadScene(const FilePath& path)
{
    StructureSystem* structureSystem = GetSystem<StructureSystem>();

    RenderContextGuard guard;
    SceneFileV2::eError ret = Scene::LoadScene(path);
    if (ret == SceneFileV2::ERROR_NO_ERROR)
    {
        curScenePath = path;
        isLoaded = true;
    }

    SceneValidator::ExtractEmptyRenderObjects(this);

    // TODO Move into SceneManagerModule
    FilePath projectPath = ProjectManagerData::CreateProjectPathFromPath(path);
    if (projectPath.IsEmpty() == false)
    {
        SceneValidator validator;
        validator.SetPathForChecking(projectPath);
        validator.ValidateScene(this, path);
    }
    return ret;
}

SceneFileV2::eError SceneEditor2::SaveScene(const FilePath& path, bool saveForGame /*= false*/)
{
    using namespace DAVA;
    EditorLightSystem* lightSystem = GetSystem<EditorLightSystem>();
    RenderContextGuard guard;
    bool cameraLightState = false;
    if (lightSystem != nullptr)
    {
        cameraLightState = lightSystem->GetCameraLightEnabled();
        lightSystem->SetCameraLightEnabled(false);
    }

    Vector<std::unique_ptr<Command>> prepareForSaveCommands;
    prepareForSaveCommands.reserve(editorSystems.size());
    for (EditorSceneSystem* editorSceneSystem : editorSystems)
    {
        std::unique_ptr<Command> cmd = editorSceneSystem->PrepareForSave(saveForGame);
        if (cmd != nullptr)
        {
            prepareForSaveCommands.push_back(std::move(cmd));
        }
    }

    std::for_each(prepareForSaveCommands.begin(), prepareForSaveCommands.end(), [](std::unique_ptr<Command>& cmd) {
        cmd->Redo();
    });

    ExtractEditorEntities();

    ScopedPtr<Texture> tilemaskTexture(nullptr);
    bool needToRestoreTilemask = false;

    LandscapeEditorDrawSystem* landscapeEditorDrawSystem = GetSystem<LandscapeEditorDrawSystem>();
    if (landscapeEditorDrawSystem)
    { //dirty magic to work with new saving of materials and FBO landscape texture
        tilemaskTexture = SafeRetain(landscapeEditorDrawSystem->GetTileMaskTexture());

        needToRestoreTilemask = landscapeEditorDrawSystem->SaveTileMaskTexture();
        landscapeEditorDrawSystem->ResetTileMaskTexture();
    }

    SceneFileV2::eError err = Scene::SaveScene(path, saveForGame);
    if (SceneFileV2::ERROR_NO_ERROR == err)
    {
        curScenePath = path;
        isLoaded = true;

        // mark current position in command stack as clean
        commandStack->SetClean();
    }

    if (needToRestoreTilemask)
    {
        landscapeEditorDrawSystem->SetTileMaskTexture(tilemaskTexture);
    }

    std::for_each(prepareForSaveCommands.rbegin(), prepareForSaveCommands.rend(), [](std::unique_ptr<Command>& cmd) {
        cmd->Undo();
    });

    InjectEditorEntities();

    if (lightSystem != nullptr)
    {
        lightSystem->SetCameraLightEnabled(cameraLightState);
    }

    return err;
}

void SceneEditor2::AddSystem(SceneSystem* sceneSystem, const ComponentMask& componentFlags, uint32 processFlags, SceneSystem* insertBeforeSceneForProcess, SceneSystem* insertBeforeSceneForInput, SceneSystem* insertBeforeSceneForFixedProcess)
{
    Scene::AddSystem(sceneSystem, componentFlags, processFlags, insertBeforeSceneForProcess, insertBeforeSceneForInput);
    EditorSceneSystem* editorSystem = dynamic_cast<EditorSceneSystem*>(sceneSystem);
    if (editorSystem != nullptr)
    {
        editorSystems.push_back(editorSystem);
        if (dynamic_cast<LandscapeEditorSystem*>(sceneSystem) != nullptr)
        {
            landscapeEditorSystems.push_back(editorSystem);
        }
    }
}

void SceneEditor2::RemoveSystem(SceneSystem* sceneSystem)
{
    EditorSceneSystem* editorSystem = dynamic_cast<EditorSceneSystem*>(sceneSystem);
    if (editorSystem != nullptr)
    {
        FindAndRemoveExchangingWithLast(editorSystems, editorSystem);
        if (dynamic_cast<LandscapeEditorSystem*>(sceneSystem) != nullptr)
        {
            FindAndRemoveExchangingWithLast(landscapeEditorSystems, editorSystem);
        }
    }

    Scene::RemoveSystem(sceneSystem);
}

bool SceneEditor2::AcquireInputLock(EditorSceneSystem* system)
{
    if (inputLockedByThis != nullptr && inputLockedByThis != system)
    {
        return false;
    }

    inputLockedByThis = system;
    return true;
}

void SceneEditor2::ReleaseInputLock(EditorSceneSystem* system)
{
    if (inputLockedByThis == system)
    {
        inputLockedByThis = nullptr;
    }
}

void SceneEditor2::ExtractEditorEntities()
{
    DVASSERT(editorEntities.size() == 0);

    Vector<Entity*> allEntities;
    GetChildNodes(allEntities);

    size_type count = allEntities.size();
    for (size_type i = 0; i < count; ++i)
    {
        if (allEntities[i]->GetName().find("editor.") != String::npos)
        {
            allEntities[i]->Retain();
            editorEntities.push_back(allEntities[i]);

            allEntities[i]->GetParent()->RemoveNode(allEntities[i]);
        }
    }
}

void SceneEditor2::InjectEditorEntities()
{
    for (int32 i = static_cast<int32>(editorEntities.size()) - 1; i >= 0; i--)
    {
        AddEditorEntity(editorEntities[i]);
        editorEntities[i]->Release();
    }
    editorEntities.clear();
}

SceneFileV2::eError SceneEditor2::SaveScene()
{
    return SaveScene(curScenePath);
}

bool SceneEditor2::Export(const SceneExporter::Params& exportingParams)
{
    ScopedPtr<SceneEditor2> clonedScene(CreateCopyForExport());
    if (clonedScene)
    {
        SceneExporter exporter;
        exporter.SetExportingParams(exportingParams);

        const FilePath& scenePathname = GetScenePath();
        FilePath newScenePathname = exportingParams.outputs[0].dataFolder + scenePathname.GetRelativePathname(exportingParams.dataSourceFolder);
        FileSystem::Instance()->CreateDirectory(newScenePathname.GetDirectory(), true);

        Vector<SceneExporter::ExportedObjectCollection> exportedObjects;
        exportedObjects.resize(SceneExporter::OBJECT_COUNT);
        bool sceneExported = exporter.ExportScene(clonedScene, scenePathname, newScenePathname, exportedObjects);

        bool objectExported = true;
        for (const SceneExporter::ExportedObjectCollection& collection : exportedObjects)
        {
            if (collection.empty() == false)
            {
                objectExported = exporter.ExportObjects(collection) && objectExported;
            }
        }

        return (sceneExported && objectExported);
    }

    return false;
}

void SceneEditor2::SaveEmitters(const Function<FilePath(const String&, const String&)>& getEmitterPathFn)
{
    List<Entity*> effectEntities;
    GetChildEntitiesWithComponent(effectEntities, Type::Instance<ParticleEffectComponent>());
    if (effectEntities.empty())
    {
        return;
    }

    List<SceneEditorDetail::EmitterDescriptor> emittersForSave;
    for (Entity* entityWithEffect : effectEntities)
    {
        const String entityName = entityWithEffect->GetName().c_str();
        ParticleEffectComponent* effect = GetEffectComponent(entityWithEffect);
        for (int32 i = 0, sz = effect->GetEmittersCount(); i < sz; ++i)
        {
            SceneEditorDetail::CollectEmittersForSave(effect->GetEmitterInstance(i)->GetEmitter(), emittersForSave, entityName);
        }
    }

    for (SceneEditorDetail::EmitterDescriptor& descriptor : emittersForSave)
    {
        ParticleEmitter* emitter = descriptor.emitter;
        const String& entityName = descriptor.entityName;

        FilePath yamlPathForSaving = descriptor.yamlPath;
        if (yamlPathForSaving.IsEmpty())
        {
            yamlPathForSaving = getEmitterPathFn(entityName, emitter->name.c_str());
        }

        if (!yamlPathForSaving.IsEmpty())
        {
            if (nullptr != descriptor.ownerLayer)
            {
                descriptor.ownerLayer->innerEmitterPath = yamlPathForSaving;
            }
            emitter->SaveToYaml(yamlPathForSaving);
        }
    }
}

const FilePath& SceneEditor2::GetScenePath() const
{
    return curScenePath;
}

void SceneEditor2::SetScenePath(const FilePath& newScenePath)
{
    curScenePath = newScenePath;
}

bool SceneEditor2::CanUndo() const
{
    return commandStack->CanUndo();
}

bool SceneEditor2::CanRedo() const
{
    return commandStack->CanRedo();
}

String SceneEditor2::GetUndoText() const
{
    const Command* undoCommand = commandStack->GetUndoCommand();
    if (undoCommand != nullptr)
    {
        return undoCommand->GetDescription();
    }
    return String();
}

String SceneEditor2::GetRedoText() const
{
    const Command* redoCommand = commandStack->GetRedoCommand();
    if (redoCommand != nullptr)
    {
        return redoCommand->GetDescription();
    }
    return String();
}

void SceneEditor2::Undo()
{
    if (commandStack->CanUndo())
    {
        commandStack->Undo();
    }
}

void SceneEditor2::Redo()
{
    if (commandStack->CanRedo())
    {
        commandStack->Redo();
    }
}

void SceneEditor2::BeginBatch(const String& text, uint32 commandsCount /*= 1*/)
{
    commandStack->BeginBatch(text, commandsCount);
}

void SceneEditor2::EndBatch()
{
    commandStack->EndBatch();
}

void SceneEditor2::Exec(std::unique_ptr<Command>&& command)
{
    if (command)
    {
        commandStack->Exec(std::move(command));
    }
}

void SceneEditor2::ClearAllCommands()
{
    commandStack->Clear();
}

const RECommandStack* SceneEditor2::GetCommandStack() const
{
    return commandStack.get();
}

bool SceneEditor2::IsLoaded() const
{
    return isLoaded;
}

void SceneEditor2::SetHUDVisible(bool visible)
{
    isHUDVisible = visible;
    GetSystem<HoodSystem>()->LockAxis(!visible);
}

bool SceneEditor2::IsHUDVisible() const
{
    return isHUDVisible;
}

bool SceneEditor2::IsChanged() const
{
    return !commandStack->IsClean();
}

void SceneEditor2::SetChanged()
{
    commandStack->SetChanged();
}

void SceneEditor2::Update(float timeElapsed)
{
    ++framesCount;

    Scene::Update(timeElapsed);

    renderStats = Renderer::GetRenderStats();
}

void SceneEditor2::SetViewportRect(const Rect& newViewportRect)
{
    GetSystem<SceneCameraSystem>()->SetViewportRect(newViewportRect);
}

void SceneEditor2::Draw()
{
    Scene::Draw();

    if (isHUDVisible)
    {
        for (EditorSceneSystem* system : editorSystems)
        {
            system->Draw();
        }
    }
    else
    {
        for (EditorSceneSystem* system : landscapeEditorSystems)
        {
            system->Draw();
        }
    }
}

void SceneEditor2::AccumulateDependentCommands(REDependentCommandsHolder& holder)
{
    if (holder.GetMasterCommandInfo().IsEmpty())
    {
        return;
    }

    for (EditorSceneSystem* system : editorSystems)
    {
        system->AccumulateDependentCommands(holder);
    }
}

void SceneEditor2::EditorCommandProcess(const RECommandNotificationObject& commandNotification)
{
    if (commandNotification.IsEmpty())
    {
        return;
    }

    for (EditorSceneSystem* system : editorSystems)
    {
        system->ProcessCommand(commandNotification);
    }
}

void SceneEditor2::AddEditorEntity(Entity* editorEntity)
{
    if (GetChildrenCount())
    {
        InsertBeforeNode(editorEntity, GetChild(0));
    }
    else
    {
        AddNode(editorEntity);
    }
}

SceneEditor2::EditorCommandNotify::EditorCommandNotify(SceneEditor2* _editor)
    : editor(_editor)
{
}

void SceneEditor2::EditorCommandNotify::AccumulateDependentCommands(REDependentCommandsHolder& holder)
{
    if (nullptr != editor)
    {
        editor->AccumulateDependentCommands(holder);
    }
}

void SceneEditor2::EditorCommandNotify::Notify(const RECommandNotificationObject& commandNotification)
{
    if (nullptr != editor)
    {
        editor->EditorCommandProcess(commandNotification);
        editor->commandExecuted.Emit(editor, commandNotification);
    }
}

const RenderStats& SceneEditor2::GetRenderStats() const
{
    return renderStats;
}

void SceneEditor2::EnableToolsInstantly(int32 toolFlags)
{
    if (toolFlags & LANDSCAPE_TOOL_CUSTOM_COLOR)
    {
        EnableCustomColorsCommand(this, true).Redo();
    }

    if (toolFlags & LANDSCAPE_TOOL_HEIGHTMAP_EDITOR)
    {
        EnableHeightmapEditorCommand(this).Redo();
    }

    if (toolFlags & LANDSCAPE_TOOL_TILEMAP_EDITOR)
    {
        EnableTilemaskEditorCommand(this).Redo();
    }

    if (toolFlags & LANDSCAPE_TOOL_RULER)
    {
        EnableRulerToolCommand(this).Redo();
    }

    if (toolFlags & LANDSCAPE_TOOL_NOT_PASSABLE_TERRAIN)
    {
        EnableNotPassableCommand(this).Redo();
    }
}

void SceneEditor2::DisableToolsInstantly(int32 toolFlags, bool saveChanges /*= true*/)
{
    if (toolFlags & LANDSCAPE_TOOL_CUSTOM_COLOR)
    {
        EnableCustomColorsCommand(this, saveChanges).Undo();
    }

    if (toolFlags & LANDSCAPE_TOOL_HEIGHTMAP_EDITOR)
    {
        EnableHeightmapEditorCommand(this).Undo();
    }

    if (toolFlags & LANDSCAPE_TOOL_TILEMAP_EDITOR)
    {
        EnableTilemaskEditorCommand(this).Undo();
    }

    if (toolFlags & LANDSCAPE_TOOL_RULER)
    {
        EnableRulerToolCommand(this).Undo();
    }

    if (toolFlags & LANDSCAPE_TOOL_NOT_PASSABLE_TERRAIN)
    {
        EnableNotPassableCommand(this).Undo();
    }
}

bool SceneEditor2::IsToolsEnabled(int32 toolFlags)
{
    bool res = false;

    if (toolFlags & LANDSCAPE_TOOL_CUSTOM_COLOR)
    {
        res |= GetSystem<CustomColorsSystem>()->IsLandscapeEditingEnabled();
    }

    if (toolFlags & LANDSCAPE_TOOL_HEIGHTMAP_EDITOR)
    {
        res |= GetSystem<HeightmapEditorSystem>()->IsLandscapeEditingEnabled();
    }

    if (toolFlags & LANDSCAPE_TOOL_TILEMAP_EDITOR)
    {
        res |= GetSystem<TilemaskEditorSystem>()->IsLandscapeEditingEnabled();
    }

    if (toolFlags & LANDSCAPE_TOOL_RULER)
    {
        res |= GetSystem<RulerToolSystem>()->IsLandscapeEditingEnabled();
    }

    if (toolFlags & LANDSCAPE_TOOL_NOT_PASSABLE_TERRAIN)
    {
        res |= GetSystem<LandscapeEditorDrawSystem>()->IsNotPassableTerrainEnabled();
    }

    return res;
}

int32 SceneEditor2::GetEnabledTools()
{
    int32 toolFlags = 0;

    if (GetSystem<CustomColorsSystem>()->IsLandscapeEditingEnabled())
    {
        toolFlags |= LANDSCAPE_TOOL_CUSTOM_COLOR;
    }

    if (GetSystem<HeightmapEditorSystem>()->IsLandscapeEditingEnabled())
    {
        toolFlags |= LANDSCAPE_TOOL_HEIGHTMAP_EDITOR;
    }

    if (GetSystem<TilemaskEditorSystem>()->IsLandscapeEditingEnabled())
    {
        toolFlags |= LANDSCAPE_TOOL_TILEMAP_EDITOR;
    }

    if (GetSystem<RulerToolSystem>()->IsLandscapeEditingEnabled())
    {
        toolFlags |= LANDSCAPE_TOOL_RULER;
    }

    if (GetSystem<LandscapeEditorDrawSystem>()->IsNotPassableTerrainEnabled())
    {
        toolFlags |= LANDSCAPE_TOOL_NOT_PASSABLE_TERRAIN;
    }

    return toolFlags;
}

Entity* SceneEditor2::Clone(Entity* dstNode /*= NULL*/)
{
    if (!dstNode)
    {
        DVASSERT(IsPointerToExactClass<SceneEditor2>(this), "Can clone only SceneEditor2");
        dstNode = new SceneEditor2();
    }

    return Scene::Clone(dstNode);
}

SceneEditor2* SceneEditor2::CreateCopyForExport()
{
    auto originalPath = curScenePath;
    auto tempName = Format(".tmp_%llu.sc2", static_cast<uint64>(time(nullptr)) ^ static_cast<uint64>(reinterpret_cast<pointer_size>(this)));

    SceneEditor2* ret = nullptr;
    FilePath tmpScenePath = FilePath::CreateWithNewExtension(curScenePath, tempName);
    if (SceneFileV2::ERROR_NO_ERROR == SaveScene(tmpScenePath))
    {
        SceneEditor2* sceneCopy = new SceneEditor2();
        if (SceneFileV2::ERROR_NO_ERROR == sceneCopy->LoadScene(tmpScenePath))
        {
            sceneCopy->RemoveSystems();
            ret = sceneCopy;
        }
        else
        {
            SafeRelease(sceneCopy);
        }

        FileSystem::Instance()->DeleteFile(tmpScenePath);
    }

    curScenePath = originalPath; // because SaveScene overwrites curScenePath

    return ret;
}

void SceneEditor2::RemoveSystems()
{
    EditorLightSystem* lightSystem = GetSystem<EditorLightSystem>();
    if (lightSystem)
    {
        lightSystem->SetCameraLightEnabled(false);
    }

    LandscapeEditorDrawSystem* landscapeEditorDrawSystem = GetSystem<LandscapeEditorDrawSystem>();
    if (landscapeEditorDrawSystem != nullptr)
    {
        landscapeEditorDrawSystem->DisableSystem();
        landscapeEditorDrawSystem = nullptr;
    }

    Vector<EditorSceneSystem*> localEditorSystems = editorSystems;
    for (EditorSceneSystem* system : localEditorSystems)
    {
        SceneSystem* sceneSystem = dynamic_cast<SceneSystem*>(system);
        DVASSERT(sceneSystem != nullptr);

        RemoveSystem(sceneSystem);
        SafeDelete(system);
    }
}

void SceneEditor2::MarkAsChanged()
{
    commandStack->SetChanged();
}

void SceneEditor2::Setup3DDrawing()
{
    if (drawCamera)
    {
        drawCamera->SetupDynamicParameters(false);
    }
}

void SceneEditor2::EnableEditorSystems()
{
    for (EditorSceneSystem* system : editorSystems)
    {
        system->EnableSystem();
    }
}

void SceneEditor2::SaveSystemsLocalProperties(PropertiesHolder* holder)
{
    for (EditorSceneSystem* system : editorSystems)
    {
        DVASSERT(system != nullptr);
        system->SaveLocalProperties(holder);
    }
}

void SceneEditor2::LoadSystemsLocalProperties(PropertiesHolder* holder, ContextAccessor* accessor)
{
    for (EditorSceneSystem* system : editorSystems)
    {
        DVASSERT(system != nullptr);
        system->LoadLocalProperties(holder, accessor);
    }
}

uint32 SceneEditor2::GetFramesCount() const
{
    return framesCount;
}

void SceneEditor2::ResetFramesCount()
{
    framesCount = 0;
}

DAVA_VIRTUAL_REFLECTION_IMPL(SceneEditor2)
{
    ReflectionRegistrator<SceneEditor2>::Begin()
    .End();
}
} // namespace DAVA