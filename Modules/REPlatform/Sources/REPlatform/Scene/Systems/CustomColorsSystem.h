#pragma once

#include "REPlatform/Scene/Systems/LandscapeEditorSystem.h"
#include "REPlatform/Scene/Systems/LandscapeEditorDrawSystem.h"

namespace DAVA
{
class CustomColorsSystem : public LandscapeEditorSystem
{
public:
    CustomColorsSystem(Scene* scene);
    ~CustomColorsSystem() override;

    LandscapeEditorDrawSystem::eErrorType EnableLandscapeEditing();
    bool DisableLandscapeEdititing(bool saveNeeded = true);

    void PrepareForRemove() override
    {
    }
    void Process(float32 timeElapsed) override;
    bool Input(UIEvent* event) override;
    void InputCancelled(UIEvent* event) override;

    void SetBrushSize(int32 brushSize, bool updateDrawSystem = true);
    int32 GetBrushSize();
    void SetColor(int32 colorIndex);
    int32 GetColor();

    void SaveTexture(); // with current of default generated path
    void SaveTexture(const FilePath& filePath);
    bool LoadTexture(const FilePath& filePath, bool createUndo);
    FilePath GetCurrentSaveFileName();

    bool ChangesPresent();

private:
    bool CouldApplyImage(Image* image, const String& imageName) const;

    void UpdateToolImage(bool force = false);
    void UpdateBrushTool();
    void CreateToolImage(const FilePath& filePath);

    void AddRectToAccumulator(const Rect& rect);
    void ResetAccumulatorRect();
    Rect GetUpdatedRect();

    void StoreOriginalState();

    void StoreSaveFileName(const FilePath& filePath);

    FilePath GetScenePath();
    String GetRelativePathToScenePath(const FilePath& absolutePath);
    FilePath GetAbsolutePathFromScenePath(const String& relativePath);
    String GetRelativePathToProjectPath(const FilePath& absolutePath);
    FilePath GetAbsolutePathFromProjectPath(const String& relativePath);

    void FinishEditing(bool applyModification);

    std::unique_ptr<Command> CreateSaveFileNameCommand(const String& filePath);

private:
    Texture* toolImageTexture = nullptr;
    Texture* loadedTexture = nullptr;
    Image* originalImage = nullptr;
    Color drawColor = Color::Transparent;
    int32 colorIndex = 0;
    int32 curToolSize = 120;
    Rect updatedRectAccumulator;
    bool editingIsEnabled = false;
};
} // namespace DAVA
