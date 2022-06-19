#pragma once

#include "REPlatform/Scene/SceneEditor2.h"
#include "REPlatform/Scene/Utils/Utils.h"

#include <TArc/DataProcessing/TArcDataNode.h>
#include <TArc/Qt/QtString.h>

#include <Reflection/ReflectionRegistrator.h>
#include <Base/RefPtr.h>

class SceneManagerModule;
namespace DAVA
{
class PropertiesItem;
class FileSystem;
class SceneData : public TArcDataNode
{
public:
    // use this alias in constructions like Any::CanCast, Any::Cast, Any::CanGet and Any::Get
    using TSceneType = RefPtr<SceneEditor2>;
    TSceneType GetScene();

    bool IsSceneChanged() const;
    FilePath GetScenePath() const;
    uint32 GetEnabledLandscapeTools() const;

    bool IsSavingAllowed(QString* message = nullptr) const;

    bool IsHUDVisible() const;

    PropertiesHolder* GetPropertiesRoot();

    static const char* scenePropertyName;
    static const char* sceneChangedPropertyName;
    static const char* scenePathPropertyName;
    static const char* sceneLandscapeToolsPropertyName;
    static const char* sceneHUDVisiblePropertyName;
    static const char* sceneCanUndoPropertyName;
    static const char* sceneUndoDescriptionPropertyName;
    static const char* sceneCanRedoPropertyName;
    static const char* sceneRedoDescriptionPropertyName;

private:
    friend class ::SceneManagerModule;

    void CreatePropertiesRoot(FileSystem* fs, const FilePath& dirPath, const FilePath& fileName);

    SceneEditor2* GetScenePtr() const;
    RefPtr<SceneEditor2> scene;
    std::unique_ptr<PropertiesHolder> propertiesRoot = nullptr;

    DAVA_VIRTUAL_REFLECTION_IN_PLACE(SceneData, TArcDataNode)
    {
        ReflectionRegistrator<SceneData>::Begin()
        .Field(scenePropertyName, &SceneData::scene)
        .Field(sceneChangedPropertyName, &SceneData::IsSceneChanged, nullptr)
        .Field(scenePathPropertyName, &SceneData::GetScenePath, nullptr)
        .Field(sceneLandscapeToolsPropertyName, &SceneData::GetEnabledLandscapeTools, nullptr)
        .Field("ScenePtr", &SceneData::GetScenePtr, nullptr)
        .Field(sceneHUDVisiblePropertyName, &SceneData::IsHUDVisible, nullptr)
        .Field(sceneCanUndoPropertyName, [](SceneData* data) { return data->scene->CanUndo(); }, nullptr)
        .Field(sceneUndoDescriptionPropertyName, [](SceneData* data) { return data->scene->GetUndoText(); }, nullptr)
        .Field(sceneCanRedoPropertyName, [](SceneData* data) { return data->scene->CanRedo(); }, nullptr)
        .Field(sceneRedoDescriptionPropertyName, [](SceneData* data) { return data->scene->GetRedoText(); }, nullptr)
        .End();
    }
};
} // namespace DAVA