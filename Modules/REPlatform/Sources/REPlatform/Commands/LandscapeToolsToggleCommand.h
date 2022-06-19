#pragma once

#include "REPlatform/Commands/RECommand.h"
#include "REPlatform/Scene/Systems/LandscapeEditorDrawSystem.h"

#include <Functional/Function.h>
#include <Reflection/Reflection.h>
#include <Reflection/ReflectionRegistrator.h>

namespace DAVA
{
class SceneEditor2;
class LandscapeToolsToggleCommand : public RECommand
{
public:
    LandscapeToolsToggleCommand(SceneEditor2* sceneEditor, const String& commandDescr, bool isEnabling,
                                uint32 allowedTools, String disablingError);
    Entity* GetEntity() const;

    void Redo() override;
    void Undo() override;
    bool IsClean() const override;

    void SaveEnabledToolsState();
    void ApplySavedState();

    using IsEnabledFunction = Function<bool()>;
    using EnableFunction = Function<LandscapeEditorDrawSystem::eErrorType()>;
    using DisableFunction = Function<bool()>;

protected:
    virtual void OnEnabled();
    virtual void OnDisabled();

protected:
    SceneEditor2* sceneEditor = nullptr;
    String disablingError;
    uint32 allowedTools = 0;
    int32 enabledTools = 0;
    IsEnabledFunction isEnabledFunction;
    EnableFunction enableFunction;
    DisableFunction disableFunction;

    DAVA_VIRTUAL_REFLECTION(LandscapeToolsToggleCommand, RECommand);
};

inline bool LandscapeToolsToggleCommand::IsClean() const
{
    return true;
}

template <typename ForwardCommand>
class LandscapeToolsReverseCommand : public ForwardCommand
{
public:
    template <typename... Args>
    LandscapeToolsReverseCommand(SceneEditor2* sceneEditor, Args... a)
        : ForwardCommand(sceneEditor, a..., false)
    {
    }

    inline void Redo() override
    {
        ForwardCommand::Undo();
    }

    inline void Undo() override
    {
        ForwardCommand::Redo();
    }

    DAVA_VIRTUAL_REFLECTION_IN_PLACE(LandscapeToolsReverseCommand, ForwardCommand)
    {
        ReflectionRegistrator<LandscapeToolsReverseCommand<ForwardCommand>>::Begin()
        .End();
    }
};

/*
 * Concerete commands
 */
class EnableHeightmapEditorCommand : public LandscapeToolsToggleCommand
{
public:
    EnableHeightmapEditorCommand(SceneEditor2* forSceneEditor, bool isEnabling = true);

private:
    void OnDisabled() override;

    DAVA_VIRTUAL_REFLECTION(EnableHeightmapEditorCommand, LandscapeToolsToggleCommand);
};
using DisableHeightmapEditorCommand = LandscapeToolsReverseCommand<EnableHeightmapEditorCommand>;

class EnableNotPassableCommand : public LandscapeToolsToggleCommand
{
public:
    EnableNotPassableCommand(SceneEditor2* forSceneEditor, bool isEnabling = true);

private:
    DAVA_VIRTUAL_REFLECTION(EnableNotPassableCommand, LandscapeToolsToggleCommand);
};
using DisableNotPassableCommand = LandscapeToolsReverseCommand<EnableNotPassableCommand>;

class EnableRulerToolCommand : public LandscapeToolsToggleCommand
{
public:
    EnableRulerToolCommand(SceneEditor2* forSceneEditor, bool isEnabling = true);

private:
    DAVA_VIRTUAL_REFLECTION(EnableRulerToolCommand, LandscapeToolsToggleCommand);
};
using DisableRulerToolCommand = LandscapeToolsReverseCommand<EnableRulerToolCommand>;

class EnableTilemaskEditorCommand : public LandscapeToolsToggleCommand
{
public:
    EnableTilemaskEditorCommand(SceneEditor2* forSceneEditor, bool isEnabling = true);

private:
    DAVA_VIRTUAL_REFLECTION(EnableTilemaskEditorCommand, LandscapeToolsToggleCommand);
};
using DisableTilemaskEditorCommand = LandscapeToolsReverseCommand<EnableTilemaskEditorCommand>;

class EnableCustomColorsCommand : public LandscapeToolsToggleCommand
{
public:
    EnableCustomColorsCommand(SceneEditor2* forSceneEditor, bool saveChanges, bool isEnabling = true);

private:
    void OnEnabled() override;

    DAVA_VIRTUAL_REFLECTION(EnableCustomColorsCommand, LandscapeToolsToggleCommand);
};
using DisableCustomColorsCommand = LandscapeToolsReverseCommand<EnableCustomColorsCommand>;

} // namespace DAVA
