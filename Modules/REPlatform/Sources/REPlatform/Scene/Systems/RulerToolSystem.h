#pragma once

#include "REPlatform/Scene/Systems/LandscapeEditorSystem.h"
#include "REPlatform/Scene/Systems/LandscapeEditorDrawSystem.h"

#include <Base/BaseTypes.h>
#include <Functional/Signal.h>

namespace DAVA
{
class SceneEditor2;
class RulerToolSystem : public LandscapeEditorSystem
{
    static const int32 APPROXIMATION_COUNT = 10;

public:
    RulerToolSystem(Scene* scene);
    virtual ~RulerToolSystem();

    LandscapeEditorDrawSystem::eErrorType EnableLandscapeEditing();
    bool DisableLandscapeEdititing();

    void PrepareForRemove() override;
    void Process(float32 timeElapsed) override;
    bool Input(UIEvent* event) override;

    float32 GetLength();
    float32 GetPreviewLength();

    Signal<SceneEditor2*, float64, float64> rulerToolLengthChanged;

protected:
    Vector2 MirrorPoint(const Vector2& point) const;

    uint32 curToolSize;
    Texture* toolImageTexture;

    List<Vector2> linePoints;
    List<float32> lengths;
    Vector2 previewPoint;
    float32 previewLength;
    bool previewEnabled;

    void SetStartPoint(const Vector2& point);
    void AddPoint(const Vector2& point);
    void RemoveLastPoint();
    void CalcPreviewPoint(const Vector2& point, bool force = false);
    float32 GetLength(const Vector2& startPoint, const Vector2& endPoint);
    void DrawPoints();
    void DisablePreview();
    void SendUpdatedLength();

    void ClearInternal();
};
} // namespace DAVA
