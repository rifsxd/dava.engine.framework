#pragma once

#include "REPlatform/Scene/Systems/LandscapeEditorSystem.h"
#include "REPlatform/Scene/Systems/LandscapeEditorDrawSystem.h"

#include <BAse/BaseTypes.h>
#include <Functional/Signal.h>
#include <Render/UniqueStateSet.h>

namespace DAVA
{
class HoodSystem;
class SceneEditor2;
class HeightmapEditorSystem : public LandscapeEditorSystem
{
public:
    enum eHeightmapDrawType
    {
        HEIGHTMAP_DRAW_ABSOLUTE = 0,
        HEIGHTMAP_DRAW_RELATIVE,
        HEIGHTMAP_DRAW_AVERAGE,
        HEIGHTMAP_DRAW_ABSOLUTE_DROPPER,
        HEIGHTMAP_DROPPER,
        HEIGHTMAP_COPY_PASTE,

        HEIGHTMAP_DRAW_TYPES_COUNT
    };

    HeightmapEditorSystem(Scene* scene);
    virtual ~HeightmapEditorSystem();

    LandscapeEditorDrawSystem::eErrorType EnableLandscapeEditing();
    bool DisableLandscapeEdititing();

    void PrepareForRemove() override
    {
    }
    void Process(float32 timeElapsed) override;
    bool Input(UIEvent* event) override;
    void InputCancelled(UIEvent* event) override;

    void SetBrushSize(int32 brushSize);
    int32 GetBrushSize();
    void SetStrength(float32 strength);
    float32 GetStrength();
    void SetAverageStrength(float32 averageStrength);
    float32 GetAverageStrength();
    void SetToolImage(const FilePath& toolImagePath, int32 index);
    int32 GetToolImageIndex();
    void SetDrawingType(eHeightmapDrawType type);
    eHeightmapDrawType GetDrawingType();

    void SetDropperHeight(float32 height);
    float32 GetDropperHeight();

    Signal<SceneEditor2*, float32> dropperHeightChanged;

protected:
    Vector2 GetHeightmapPositionFromCursor() const;

protected:
    Texture* squareTexture = nullptr;
    uint32 curToolSize = 30;
    Image* curToolImage = nullptr;

    eHeightmapDrawType drawingType = HEIGHTMAP_DRAW_ABSOLUTE;
    float32 strength = 15.f;
    float32 averageStrength = 0.5f;
    bool inverseDrawingEnabled = false;
    FilePath toolImagePath;
    int32 toolImageIndex = 0;

    float32 curHeight = 0.f;
    Vector2 copyPasteFrom;
    Vector2 copyPasteTo;

    Rect heightmapUpdatedRect;

    bool editingIsEnabled = false;

    Heightmap* originalHeightmap = nullptr;

    eHeightmapDrawType activeDrawingType;

    void UpdateToolImage();
    void UpdateBrushTool(float32 timeElapsed);

    void AddRectToAccumulator(Rect& accumulator, const Rect& rect);
    void ResetAccumulatorRect(Rect& accumulator);
    Rect GetHeightmapUpdatedRect();

    void StoreOriginalHeightmap();
    void FinishEditing(bool applyModification);
};
} // namespace DAVA
