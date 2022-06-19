#include "REPlatform/Scene/Systems/HeightmapEditorSystem.h"
#include "REPlatform/Scene/Private/Systems/LandscapeEditorDrawSystem/HeightmapProxy.h"
#include "REPlatform/Scene/Systems/CollisionSystem.h"
#include "REPlatform/Scene/Systems/HoodSystem.h"
#include "REPlatform/Scene/Systems/LandscapeProxy.h"
#include "REPlatform/Scene/Systems/ModifSystem.h"
#include "REPlatform/Scene/Systems/SelectionSystem.h"
#include "REPlatform/Scene/Utils/Utils.h"

#include "REPlatform/Commands/HeightmapEditorCommands2.h"

#include <TArc/Utils/Utils.h>

#include <Render/Image/ImageConvert.h>
#include "../../SceneEditor2.h"

namespace DAVA
{
HeightmapEditorSystem::HeightmapEditorSystem(Scene* scene)
    : LandscapeEditorSystem(scene, DefaultCursorPath())
    , copyPasteFrom(-1.f, -1.f)
    , copyPasteTo(-1.f, -1.f)
{
    activeDrawingType = drawingType;
}

HeightmapEditorSystem::~HeightmapEditorSystem()
{
    SafeRelease(curToolImage);
    SafeRelease(squareTexture);
}

LandscapeEditorDrawSystem::eErrorType HeightmapEditorSystem::EnableLandscapeEditing()
{
    if (enabled)
    {
        return LandscapeEditorDrawSystem::LANDSCAPE_EDITOR_SYSTEM_NO_ERRORS;
    }

    LandscapeEditorDrawSystem::eErrorType canBeEnabledError = IsCanBeEnabled();
    if (canBeEnabledError != LandscapeEditorDrawSystem::LANDSCAPE_EDITOR_SYSTEM_NO_ERRORS)
    {
        return canBeEnabledError;
    }

    LandscapeEditorDrawSystem::eErrorType enableCustomDrawError = drawSystem->EnableCustomDraw();
    if (enableCustomDrawError != LandscapeEditorDrawSystem::LANDSCAPE_EDITOR_SYSTEM_NO_ERRORS)
    {
        return enableCustomDrawError;
    }

    bool inputLocked = AcquireInputLock(GetScene());
    DVASSERT(inputLocked == true);

    Scene* scene = GetScene();
    scene->GetSystem<SelectionSystem>()->SetLocked(true);

    landscapeSize = drawSystem->GetHeightmapProxy()->Size();
    copyPasteFrom = Vector2(-1.f, -1.f);

    drawSystem->EnableCursor();
    drawSystem->SetCursorTexture(cursorTexture);
    drawSystem->GetBaseLandscape()->SetUpdatable(true);

    SetBrushSize(curToolSize);

    enabled = true;
    return LandscapeEditorDrawSystem::LANDSCAPE_EDITOR_SYSTEM_NO_ERRORS;
}

bool HeightmapEditorSystem::DisableLandscapeEdititing()
{
    if (!enabled)
    {
        return true;
    }

    FinishEditing(false);

    ReleaseInputLock(GetScene());
    Scene* scene = GetScene();
    scene->GetSystem<SelectionSystem>()->SetLocked(false);
    scene->GetSystem<EntityModificationSystem>()->SetLocked(false);

    drawSystem->DisableCursor();
    drawSystem->DisableCustomDraw();

    enabled = false;
    return !enabled;
}

void HeightmapEditorSystem::Process(float32 timeElapsed)
{
    if (!IsLandscapeEditingEnabled())
    {
        return;
    }

    if (editingIsEnabled && isIntersectsLandscape)
    {
        UpdateBrushTool(timeElapsed);
    }
}

bool HeightmapEditorSystem::Input(UIEvent* event)
{
    if (!IsLandscapeEditingEnabled())
    {
        return false;
    }

    UpdateCursorPosition();

    if (event->mouseButton == eMouseButtons::LEFT)
    {
        Vector3 point;

        switch (event->phase)
        {
        case UIEvent::Phase::BEGAN:
            if (drawingType == HEIGHTMAP_DRAW_ABSOLUTE_DROPPER ||
                drawingType == HEIGHTMAP_DROPPER)
            {
                curHeight = drawSystem->GetHeightAtHeightmapPoint(GetHeightmapPositionFromCursor());

                dropperHeightChanged.Emit(static_cast<SceneEditor2*>(GetScene()), curHeight);
            }

            if (isIntersectsLandscape)
            {
                if (drawingType == HEIGHTMAP_COPY_PASTE)
                {
                    bool altPressed = IsKeyPressed(eModifierKeys::ALT);
                    if (altPressed == true)
                    {
                        copyPasteFrom = GetHeightmapPositionFromCursor();
                        copyPasteTo = Vector2(-1.f, -1.f);
                        return false;
                    }
                    else
                    {
                        if (copyPasteFrom == Vector2(-1.f, -1.f))
                        {
                            return false;
                        }
                        copyPasteTo = GetHeightmapPositionFromCursor();
                        StoreOriginalHeightmap();
                    }
                }
                else
                {
                    if (drawingType != HEIGHTMAP_DROPPER)
                    {
                        StoreOriginalHeightmap();
                    }
                }

                editingIsEnabled = true;
            }

            activeDrawingType = drawingType;
            break;

        case UIEvent::Phase::DRAG:
            break;

        case UIEvent::Phase::ENDED:
            FinishEditing(true);
            break;

        default:
            break;
        }
    }
    return false;
}

void HeightmapEditorSystem::InputCancelled(UIEvent* event)
{
    if (IsLandscapeEditingEnabled() && (event->mouseButton == eMouseButtons::LEFT))
    {
        FinishEditing(true);
    }
}

void HeightmapEditorSystem::FinishEditing(bool applyModification)
{
    if (editingIsEnabled)
    {
        if (drawingType != HEIGHTMAP_DROPPER)
        {
            if (applyModification)
            {
                SceneEditor2* scene = static_cast<SceneEditor2*>(GetScene());
                scene->Exec(std::make_unique<ModifyHeightmapCommand>(drawSystem, originalHeightmap, GetHeightmapUpdatedRect()));
            }
            SafeRelease(originalHeightmap);
        }
        editingIsEnabled = false;
    }
}

void HeightmapEditorSystem::UpdateToolImage()
{
    if (!toolImagePath.IsEmpty())
    {
        SafeRelease(curToolImage);

        Vector<Image*> images;
        ImageSystem::Load(toolImagePath, images);
        if (images.size())
        {
            DVASSERT(images.size() == 1);
            DVASSERT(images[0]->GetPixelFormat() == FORMAT_RGBA8888);

            curToolImage = Image::Create(curToolSize, curToolSize, FORMAT_RGBA8888);
            ImageConvert::ResizeRGBA8Billinear(reinterpret_cast<uint32*>(images[0]->data), images[0]->GetWidth(), images[0]->GetHeight(),
                                               reinterpret_cast<uint32*>(curToolImage->data), curToolSize, curToolSize);

            SafeRelease(images[0]);
        }
    }
}

void HeightmapEditorSystem::UpdateBrushTool(float32 timeElapsed)
{
    if (!curToolImage)
    {
        Logger::Error("Tool image is empty!");
        return;
    }

    EditorHeightmap* editorHeightmap = drawSystem->GetHeightmapProxy();

    int32 scaleSize = curToolImage->GetWidth();
    Vector2 pos = GetHeightmapPositionFromCursor() - Vector2(static_cast<float32>(scaleSize), static_cast<float32>(scaleSize)) / 2.0f;
    {
        switch (activeDrawingType)
        {
        case HEIGHTMAP_DRAW_RELATIVE:
        {
            float32 koef = (strength * timeElapsed);
            if (inverseDrawingEnabled)
            {
                koef = -koef;
            }

            if (IsKeyModificatorPressed(eInputElements::KB_LALT))
            {
                koef = -koef;
            }

            editorHeightmap->DrawRelativeRGBA(curToolImage, static_cast<int32>(pos.x), static_cast<int32>(pos.y), scaleSize, scaleSize, koef);
            break;
        }

        case HEIGHTMAP_DRAW_AVERAGE:
        {
            float32 koef = (averageStrength * timeElapsed) * 2.0f;
            editorHeightmap->DrawAverageRGBA(curToolImage, static_cast<int32>(pos.x), static_cast<int32>(pos.y), scaleSize, scaleSize, koef);
            break;
        }

        case HEIGHTMAP_DRAW_ABSOLUTE:
        case HEIGHTMAP_DRAW_ABSOLUTE_DROPPER:
        {
            float32 maxHeight = drawSystem->GetLandscapeMaxHeight();
            float32 height = curHeight / maxHeight * Heightmap::MAX_VALUE;

            float32 koef = (averageStrength * timeElapsed) * 2.0f;
            editorHeightmap->DrawAbsoluteRGBA(curToolImage, static_cast<int32>(pos.x), static_cast<int32>(pos.y), scaleSize, scaleSize, koef, height);
            break;
        }

        case HEIGHTMAP_DROPPER:
        {
            float32 curHeight = drawSystem->GetHeightAtHeightmapPoint(GetHeightmapPositionFromCursor());
            dropperHeightChanged.Emit(static_cast<SceneEditor2*>(GetScene()), curHeight);
            return;
        }

        case HEIGHTMAP_COPY_PASTE:
        {
            if (copyPasteFrom == Vector2(-1.f, -1.f) || copyPasteTo == Vector2(-1.f, -1.f))
            {
                return;
            }

            Vector2 posTo = pos;

            Vector2 deltaPos = GetHeightmapPositionFromCursor() - copyPasteTo;
            Vector2 posFrom = copyPasteFrom + deltaPos - Vector2(static_cast<float32>(scaleSize), static_cast<float32>(scaleSize)) / 2.f;

            float32 koef = (averageStrength * timeElapsed) * 2.0f;

            editorHeightmap->DrawCopypasteRGBA(curToolImage, posFrom, posTo, scaleSize, scaleSize, koef);

            break;
        }

        default:
            Logger::Error("Invalid drawing type!");
            return;
        }

        Rect rect(pos.x, pos.y, static_cast<float32>(scaleSize), static_cast<float32>(scaleSize));
        drawSystem->GetHeightmapProxy()->UpdateRect(rect);
        AddRectToAccumulator(heightmapUpdatedRect, rect);
    }
}

void HeightmapEditorSystem::ResetAccumulatorRect(Rect& accumulator)
{
    float32 inf = std::numeric_limits<float32>::infinity();
    accumulator = Rect(inf, inf, -inf, -inf);
}

void HeightmapEditorSystem::AddRectToAccumulator(Rect& accumulator, const Rect& rect)
{
    accumulator = accumulator.Combine(rect);
}

Rect HeightmapEditorSystem::GetHeightmapUpdatedRect()
{
    Rect r = heightmapUpdatedRect;
    drawSystem->ClampToHeightmap(r);
    return r;
}

void HeightmapEditorSystem::StoreOriginalHeightmap()
{
    EditorHeightmap* editorHeightmap = drawSystem->GetHeightmapProxy();

    DVASSERT(originalHeightmap == NULL);
    originalHeightmap = editorHeightmap->Clone(NULL);
    ResetAccumulatorRect(heightmapUpdatedRect);
}

void HeightmapEditorSystem::SetBrushSize(int32 brushSize)
{
    if (brushSize > 0)
    {
        curToolSize = brushSize;
        cursorSize = static_cast<float32>(brushSize) / landscapeSize;
        drawSystem->SetCursorSize(cursorSize);

        UpdateToolImage();
    }
}

void HeightmapEditorSystem::SetStrength(float32 strength)
{
    float32 s = Abs(strength);
    this->strength = s;

    inverseDrawingEnabled = false;
    if (strength < 0.f)
    {
        inverseDrawingEnabled = true;
    }
}

void HeightmapEditorSystem::SetAverageStrength(float32 averageStrength)
{
    if (averageStrength >= 0)
    {
        this->averageStrength = averageStrength;
    }
}

void HeightmapEditorSystem::SetToolImage(const FilePath& toolImagePath, int32 index)
{
    this->toolImagePath = toolImagePath;
    this->toolImageIndex = index;
    UpdateToolImage();
}

void HeightmapEditorSystem::SetDrawingType(eHeightmapDrawType type)
{
    copyPasteFrom = Vector2(-1.f, -1.f);
    drawingType = type;
}

int32 HeightmapEditorSystem::GetBrushSize()
{
    return curToolSize;
}

float32 HeightmapEditorSystem::GetStrength()
{
    float32 s = strength;
    if (inverseDrawingEnabled)
    {
        s = -s;
    }

    return s;
}

float32 HeightmapEditorSystem::GetAverageStrength()
{
    return averageStrength;
}

int32 HeightmapEditorSystem::GetToolImageIndex()
{
    return toolImageIndex;
}

HeightmapEditorSystem::eHeightmapDrawType HeightmapEditorSystem::GetDrawingType()
{
    return drawingType;
}

void HeightmapEditorSystem::SetDropperHeight(float32 height)
{
    float32 maxHeight = drawSystem->GetLandscapeMaxHeight();

    if (height >= 0 && height <= maxHeight)
    {
        curHeight = height;
        dropperHeightChanged.Emit(static_cast<SceneEditor2*>(GetScene()), curHeight);
    }
}

float32 HeightmapEditorSystem::GetDropperHeight()
{
    return curHeight;
}

Vector2 HeightmapEditorSystem::GetHeightmapPositionFromCursor() const
{
    return drawSystem->GetHeightmapProxy()->Size() * Vector2(cursorPosition.x, 1.f - cursorPosition.y);
}
} // namespace DAVA
