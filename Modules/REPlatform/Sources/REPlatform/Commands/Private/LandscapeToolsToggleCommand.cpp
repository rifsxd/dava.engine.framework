#include "REPlatform/Commands/LandscapeToolsToggleCommand.h"

#include "REPlatform/Global/StringConstants.h"
#include "REPlatform/Scene/Private/Systems/LandscapeEditorDrawSystem/CustomColorsProxy.h"
#include "REPlatform/Scene/SceneEditor2.h"
#include "REPlatform/Scene/Systems/CustomColorsSystem.h"
#include "REPlatform/Scene/Systems/HeightmapEditorSystem.h"
#include "REPlatform/Scene/Systems/LandscapeEditorDrawSystem.h"
#include "REPlatform/Scene/Systems/RulerToolSystem.h"
#include "REPlatform/Scene/Systems/TilemaskEditorSystem.h"

#include <Functional/Function.h>
#include <Logger/Logger.h>
#include <Reflection/ReflectionRegistrator.h>
#include <Scene3D/Systems/FoliageSystem.h>

namespace DAVA
{
namespace LTTCLocal
{
bool TryEnableWithFunctions(SceneEditor2* editor, uint32 allowedTools,
                            const LandscapeToolsToggleCommand::IsEnabledFunction& isEnabled,
                            const LandscapeToolsToggleCommand::EnableFunction& enable)
{
    if (editor == nullptr)
        return false;

    if (isEnabled())
        return false;

    uint32 disableFlags = SceneEditor2::LANDSCAPE_TOOLS_ALL & (~allowedTools);
    editor->DisableToolsInstantly(disableFlags);
    if (editor->IsToolsEnabled(disableFlags))
    {
        Logger::Error(ResourceEditor::LANDSCAPE_EDITOR_SYSTEM_DISABLE_EDITORS.c_str());
        return false;
    }

    LandscapeEditorDrawSystem::eErrorType enablingError = enable();
    if (enablingError == LandscapeEditorDrawSystem::LANDSCAPE_EDITOR_SYSTEM_NO_ERRORS)
    {
        editor->foliageSystem->SetFoliageVisible(false);
    }
    else
    {
        Logger::Error(LandscapeEditorDrawSystem::GetDescriptionByError(enablingError).c_str());
        return false;
    }

    return true;
}

bool TryDisableWithFunctions(SceneEditor2* editor, const String& error,
                             const LandscapeToolsToggleCommand::IsEnabledFunction& isEnabled,
                             const LandscapeToolsToggleCommand::DisableFunction& disable)
{
    if (editor == nullptr)
        return false;

    if (isEnabled())
    {
        if (disable())
        {
            editor->foliageSystem->SetFoliageVisible(true);
        }
        else
        {
            Logger::Error(error.c_str());
            return false;
        }
    }

    return true;
}
}

/*
 * Common
 */
LandscapeToolsToggleCommand::LandscapeToolsToggleCommand(SceneEditor2* _sceneEditor, const String& commandDescr,
                                                         bool isEnabling, uint32 _allowedTools, String _disablingError)
    : RECommand(commandDescr + ((isEnabling == true) ? "Enabled" : "Disabled"))
    , sceneEditor(_sceneEditor)
    , disablingError(_disablingError)
    , allowedTools(_allowedTools)
{
}

Entity* LandscapeToolsToggleCommand::GetEntity() const
{
    return sceneEditor;
}

void LandscapeToolsToggleCommand::SaveEnabledToolsState()
{
    if (sceneEditor != nullptr)
    {
        enabledTools = sceneEditor->GetEnabledTools();
    }
}

void LandscapeToolsToggleCommand::ApplySavedState()
{
    if (sceneEditor != nullptr)
    {
        sceneEditor->EnableToolsInstantly(enabledTools);
    }
}

void LandscapeToolsToggleCommand::Redo()
{
    SaveEnabledToolsState();
    if (LTTCLocal::TryEnableWithFunctions(sceneEditor, allowedTools, isEnabledFunction, enableFunction))
    {
        OnEnabled();
    }
}

void LandscapeToolsToggleCommand::Undo()
{
    if (LTTCLocal::TryDisableWithFunctions(sceneEditor, disablingError, isEnabledFunction, disableFunction))
    {
        OnDisabled();
        ApplySavedState();
    }
}

void LandscapeToolsToggleCommand::OnEnabled()
{
}

void LandscapeToolsToggleCommand::OnDisabled()
{
}

DAVA_VIRTUAL_REFLECTION_IMPL(LandscapeToolsToggleCommand)
{
    ReflectionRegistrator<LandscapeToolsToggleCommand>::Begin()
    .End();
}

/*
 * Ruler
 */
EnableRulerToolCommand::EnableRulerToolCommand(SceneEditor2* forSceneEditor, bool isEnabling)
    : LandscapeToolsToggleCommand(forSceneEditor, "Ruler Tool ", isEnabling,
                                  0, ResourceEditor::RULER_TOOL_DISABLE_ERROR)
{
    RulerToolSystem* system = forSceneEditor->GetSystem<RulerToolSystem>();
    isEnabledFunction = MakeFunction(system, &RulerToolSystem::IsLandscapeEditingEnabled);
    enableFunction = MakeFunction(system, &RulerToolSystem::EnableLandscapeEditing);
    disableFunction = MakeFunction(system, &RulerToolSystem::DisableLandscapeEdititing);
}

DAVA_VIRTUAL_REFLECTION_IMPL(EnableRulerToolCommand)
{
    ReflectionRegistrator<EnableRulerToolCommand>::Begin()
    .End();
}

/*
 * Tilemask
 */
EnableTilemaskEditorCommand::EnableTilemaskEditorCommand(SceneEditor2* forSceneEditor, bool isEnabling)
    : LandscapeToolsToggleCommand(forSceneEditor, "Tilemask Editor ", isEnabling,
                                  0, ResourceEditor::TILEMASK_EDITOR_DISABLE_ERROR)
{
    TilemaskEditorSystem* system = forSceneEditor->GetSystem<TilemaskEditorSystem>();
    isEnabledFunction = MakeFunction(system, &TilemaskEditorSystem::IsLandscapeEditingEnabled);
    enableFunction = MakeFunction(system, &TilemaskEditorSystem::EnableLandscapeEditing);
    disableFunction = MakeFunction(system, &TilemaskEditorSystem::DisableLandscapeEdititing);
}

DAVA_VIRTUAL_REFLECTION_IMPL(EnableTilemaskEditorCommand)
{
    ReflectionRegistrator<EnableTilemaskEditorCommand>::Begin()
    .End();
}

/*
 * Heightmap editor
 */
EnableHeightmapEditorCommand::EnableHeightmapEditorCommand(SceneEditor2* forSceneEditor, bool isEnabling)
    : LandscapeToolsToggleCommand(forSceneEditor, "Heightmap Editor ", isEnabling,
                                  SceneEditor2::LANDSCAPE_TOOL_NOT_PASSABLE_TERRAIN,
                                  ResourceEditor::HEIGHTMAP_EDITOR_DISABLE_ERROR)
{
    HeightmapEditorSystem* system = forSceneEditor->GetSystem<HeightmapEditorSystem>();
    isEnabledFunction = MakeFunction(system, &HeightmapEditorSystem::IsLandscapeEditingEnabled);
    enableFunction = MakeFunction(system, &HeightmapEditorSystem::EnableLandscapeEditing);
    disableFunction = MakeFunction(system, &HeightmapEditorSystem::DisableLandscapeEdititing);
}

void EnableHeightmapEditorCommand::OnDisabled()
{
    sceneEditor->foliageSystem->SyncFoliageWithLandscape();
}

DAVA_VIRTUAL_REFLECTION_IMPL(EnableHeightmapEditorCommand)
{
    ReflectionRegistrator<EnableHeightmapEditorCommand>::Begin()
    .End();
}

/*
 * Custom colors
 */
EnableCustomColorsCommand::EnableCustomColorsCommand(SceneEditor2* forSceneEditor, bool _saveChanges, bool isEnabling)
    : LandscapeToolsToggleCommand(forSceneEditor, "Custom Colors Editor ", isEnabling, 0, ResourceEditor::CUSTOM_COLORS_DISABLE_ERROR)
{
    CustomColorsSystem* system = forSceneEditor->GetSystem<CustomColorsSystem>();
    isEnabledFunction = MakeFunction(system, &CustomColorsSystem::IsLandscapeEditingEnabled);
    enableFunction = MakeFunction(system, &CustomColorsSystem::EnableLandscapeEditing);
    disableFunction = Bind(MakeFunction(system, &CustomColorsSystem::DisableLandscapeEdititing), _saveChanges);
}

void EnableCustomColorsCommand::OnEnabled()
{
    LandscapeEditorDrawSystem* drawSystem = sceneEditor->GetSystem<LandscapeEditorDrawSystem>();
    if (drawSystem->GetCustomColorsProxy()->IsTextureLoaded() == false)
    {
        Logger::Error(LandscapeEditorDrawSystem::GetDescriptionByError(LandscapeEditorDrawSystem::LANDSCAPE_EDITOR_SYSTEM_CUSTOMCOLORS_ABSENT).c_str());
        drawSystem->GetCustomColorsProxy()->ResetLoadedState();
    }
}

DAVA_VIRTUAL_REFLECTION_IMPL(EnableCustomColorsCommand)
{
    ReflectionRegistrator<EnableCustomColorsCommand>::Begin()
    .End();
}

/*
 * Not passable - special case
 */
EnableNotPassableCommand::EnableNotPassableCommand(SceneEditor2* forSceneEditor, bool isEnabling)
    : LandscapeToolsToggleCommand(forSceneEditor, "Not Passable Editor ", isEnabling,
                                  SceneEditor2::LANDSCAPE_TOOL_HEIGHTMAP_EDITOR,
                                  ResourceEditor::NOT_PASSABLE_TERRAIN_DISABLE_ERROR)
{
    LandscapeEditorDrawSystem* system = forSceneEditor->GetSystem<LandscapeEditorDrawSystem>();
    isEnabledFunction = MakeFunction(system, &LandscapeEditorDrawSystem::IsNotPassableTerrainEnabled);
    enableFunction = MakeFunction(system, &LandscapeEditorDrawSystem::EnableNotPassableTerrain);
    disableFunction = MakeFunction(system, &LandscapeEditorDrawSystem::DisableNotPassableTerrain);
}

DAVA_VIRTUAL_REFLECTION_IMPL(EnableNotPassableCommand)
{
    ReflectionRegistrator<EnableNotPassableCommand>::Begin()
    .End();
}
} // namespace DAVA
