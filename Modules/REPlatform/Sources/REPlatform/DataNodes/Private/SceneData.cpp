#include "REPlatform/DataNodes/SceneData.h"
#include "REPlatform/Scene/Systems/WayEditSystem.h"

#include <TArc/DataProcessing/PropertiesHolder.h>

#include <FileSystem/FileSystem.h>

namespace DAVA
{
RefPtr<SceneEditor2> SceneData::GetScene()
{
    return scene;
}

bool SceneData::IsSceneChanged() const
{
    if (scene.Get() == nullptr)
    {
        return false;
    }

    return scene->IsChanged();
}

FilePath SceneData::GetScenePath() const
{
    if (scene.Get() == nullptr)
    {
        return FilePath();
    }

    return scene->GetScenePath();
}

uint32 SceneData::GetEnabledLandscapeTools() const
{
    if (scene.Get() == nullptr)
    {
        return 0;
    }

    return scene->GetEnabledTools();
}

bool SceneData::IsSavingAllowed(QString* message /*= nullptr*/) const
{
    DVASSERT(scene.Get() != nullptr);
    QString warningMessage;
    if (scene->GetEnabledTools() != 0)
    {
        warningMessage = "Disable landscape editing before save!";
    }
    else if (scene->GetSystem<WayEditSystem>()->IsWayEditEnabled())
    {
        warningMessage = "Disable path editing before save!";
    }

    if (warningMessage.isEmpty())
    {
        return true;
    }
    if (message != nullptr)
    {
        *message = warningMessage;
    }
    return false;
}

SceneEditor2* SceneData::GetScenePtr() const
{
    return scene.Get();
}

bool SceneData::IsHUDVisible() const
{
    if (scene.Get() == nullptr)
    {
        return false;
    }

    return scene->IsHUDVisible();
}

PropertiesHolder* SceneData::GetPropertiesRoot()
{
    return propertiesRoot.get();
}

void SceneData::CreatePropertiesRoot(FileSystem* fs, const FilePath& dirPath, const FilePath& fileName)
{
    fs->CreateDirectory(dirPath, true);
    if (propertiesRoot.get() == nullptr)
    {
        propertiesRoot = std::make_unique<PropertiesHolder>(fileName.GetFilename(), dirPath);
    }
    else
    {
        propertiesRoot = PropertiesHolder::CopyWithNewPath(*propertiesRoot, fs, fileName.GetFilename(), dirPath);
    }
}

const char* SceneData::scenePropertyName = "Scene";
const char* SceneData::sceneChangedPropertyName = "IsSceneChanged";
const char* SceneData::scenePathPropertyName = "ScenePath";
const char* SceneData::sceneLandscapeToolsPropertyName = "EnabledLandscapeTools";
const char* SceneData::sceneHUDVisiblePropertyName = "sceneHUDVisiblePropertyName";
const char* SceneData::sceneCanUndoPropertyName = "canUndo";
const char* SceneData::sceneUndoDescriptionPropertyName = "undoDescription";
const char* SceneData::sceneCanRedoPropertyName = "canRedo";
const char* SceneData::sceneRedoDescriptionPropertyName = "redoDescription";

} // namespace DAVA