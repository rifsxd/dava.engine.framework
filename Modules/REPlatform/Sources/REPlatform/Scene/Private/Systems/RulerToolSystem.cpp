#include "REPlatform/Scene/Systems/RulerToolSystem.h"
#include "REPlatform/Scene/Private/Systems/LandscapeEditorDrawSystem/HeightmapProxy.h"
#include "REPlatform/Scene/Private/Systems/LandscapeEditorDrawSystem/RulerToolProxy.h"
#include "REPlatform/Scene/SceneEditor2.h"
#include "REPlatform/Scene/Systems/CollisionSystem.h"
#include "REPlatform/Scene/Systems/LandscapeProxy.h"
#include "REPlatform/Scene/Systems/ModifSystem.h"
#include "REPlatform/Scene/Systems/SelectionSystem.h"
#include "REPlatform/Scene/Utils/Utils.h"

namespace DAVA
{
RulerToolSystem::RulerToolSystem(Scene* scene)
    : LandscapeEditorSystem(scene, DefaultCursorPath())
    , curToolSize(0)
    , previewEnabled(true)
{
}

RulerToolSystem::~RulerToolSystem()
{
}

LandscapeEditorDrawSystem::eErrorType RulerToolSystem::EnableLandscapeEditing()
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
    DVASSERT(inputLocked);

    Scene* scene = GetScene();
    scene->GetSystem<SelectionSystem>()->SetLocked(true);

    Texture* rulerToolTexture = drawSystem->GetRulerToolProxy()->GetTexture();
    drawSystem->GetLandscapeProxy()->SetToolTexture(rulerToolTexture, false);
    landscapeSize = drawSystem->GetHeightmapProxy()->Size();

    previewLength = -1.f;
    previewEnabled = true;

    ClearInternal();
    DrawPoints();

    SendUpdatedLength();

    enabled = true;
    return LandscapeEditorDrawSystem::LANDSCAPE_EDITOR_SYSTEM_NO_ERRORS;
}

bool RulerToolSystem::DisableLandscapeEdititing()
{
    if (!enabled)
    {
        return true;
    }

    ReleaseInputLock(GetScene());
    Scene* scene = GetScene();
    scene->GetSystem<SelectionSystem>()->SetLocked(false);
    scene->GetSystem<EntityModificationSystem>()->SetLocked(false);
    drawSystem->DisableCustomDraw();
    drawSystem->GetLandscapeProxy()->SetToolTexture(nullptr, false);

    ClearInternal();
    previewLength = -1.f;
    SendUpdatedLength();

    enabled = false;
    return !enabled;
}

void RulerToolSystem::PrepareForRemove()
{
    ClearInternal();
}

void RulerToolSystem::Process(float32 timeElapsed)
{
    if (!IsLandscapeEditingEnabled())
    {
        return;
    }
}

bool RulerToolSystem::Input(UIEvent* event)
{
    if (!IsLandscapeEditingEnabled())
    {
        return false;
    }

    UpdateCursorPosition();

    Vector3 point3;
    GetScene()->GetSystem<SceneCollisionSystem>()->LandRayTestFromCamera(point3);
    Vector2 point(point3.x, point3.y);

    switch (event->phase)
    {
    case UIEvent::Phase::KEY_DOWN:
    case UIEvent::Phase::KEY_DOWN_REPEAT:
        if (eInputElements::KB_BACKSPACE == event->key)
        {
            RemoveLastPoint();
            previewEnabled = true;
            CalcPreviewPoint(point, true);
        }
        else if (eInputElements::KB_ESCAPE == event->key)
        {
            previewEnabled = false;
        }
        DrawPoints();
        break;

    case UIEvent::Phase::MOVE:
        if (previewEnabled)
        {
            CalcPreviewPoint(point);
            DrawPoints();
        }
        break;

    case UIEvent::Phase::ENDED:
        if (event->mouseButton == eMouseButtons::LEFT && isIntersectsLandscape)
        {
            if (IsKeyModificatorPressed(eInputElements::KB_LSHIFT))
            {
                SetStartPoint(point);
            }
            else
            {
                if (previewEnabled)
                {
                    AddPoint(point);
                }
            }

            previewEnabled = true;
            CalcPreviewPoint(point);
            DrawPoints();
        }
        break;

    default:
        break;
    }
    return false;
}

void RulerToolSystem::SetStartPoint(const Vector2& point)
{
    ClearInternal();

    previewPoint = point;
    linePoints.push_back(point);
    lengths.push_back(0.f);
    SendUpdatedLength();
}

void RulerToolSystem::AddPoint(const Vector2& point)
{
    if (0 < linePoints.size())
    {
        Vector2 prevPoint = *(linePoints.rbegin());
        float32 l = lengths.back();
        l += GetLength(prevPoint, point);

        linePoints.push_back(point);
        lengths.push_back(l);

        SendUpdatedLength();
    }
}

void RulerToolSystem::RemoveLastPoint()
{
    //remove points except start point
    if (linePoints.size() > 1)
    {
        List<Vector2>::iterator pointsIter = linePoints.end();
        --pointsIter;
        linePoints.erase(pointsIter);

        List<float32>::iterator lengthsIter = lengths.end();
        --lengthsIter;
        lengths.erase(lengthsIter);

        SendUpdatedLength();
    }
}

void RulerToolSystem::CalcPreviewPoint(const Vector2& point, bool force)
{
    if (!previewEnabled)
    {
        return;
    }

    if ((isIntersectsLandscape && linePoints.size() > 0) && (force || previewPoint != point))
    {
        Vector2 lastPoint = linePoints.back();
        float32 previewLen = GetLength(lastPoint, point);

        previewPoint = point;
        previewLength = lengths.back() + previewLen;
    }
    else if (!isIntersectsLandscape)
    {
        previewLength = -1.f;
    }
    SendUpdatedLength();
}

float32 RulerToolSystem::GetLength(const Vector2& startPoint, const Vector2& endPoint)
{
    float32 lineSize = 0.f;

    Vector3 prevPoint = Vector3(startPoint);
    Vector3 prevLandscapePoint = drawSystem->GetLandscapeProxy()->PlacePoint(prevPoint); //

    for (int32 i = 1; i <= APPROXIMATION_COUNT; ++i)
    {
        Vector3 point = Vector3(startPoint + (endPoint - startPoint) * i / static_cast<float32>(APPROXIMATION_COUNT));
        Vector3 landscapePoint = drawSystem->GetLandscapeProxy()->PlacePoint(point); //

        lineSize += (landscapePoint - prevLandscapePoint).Length();

        prevPoint = point;
        prevLandscapePoint = landscapePoint;
    }

    return lineSize;
}

void RulerToolSystem::DrawPoints()
{
    if (!drawSystem->GetRulerToolProxy())
    {
        return;
    }

    Texture* targetTexture = drawSystem->GetRulerToolProxy()->GetTexture();

    RenderSystem2D::RenderTargetPassDescriptor desc;
    desc.colorAttachment = targetTexture->handle;
    desc.depthAttachment = targetTexture->handleDepthStencil;
    desc.width = targetTexture->GetWidth();
    desc.height = targetTexture->GetHeight();
    desc.transformVirtualToPhysical = false;
    RenderSystem2D::Instance()->BeginRenderTargetPass(desc);

    Vector<Vector2> points;
    points.reserve(linePoints.size() + 1);
    std::copy(linePoints.begin(), linePoints.end(), std::back_inserter(points));

    if (previewEnabled && isIntersectsLandscape)
    {
        points.push_back(previewPoint);
    }

    const uint32 pointsCount = static_cast<uint32>(points.size());
    if (pointsCount > 1)
    {
        for (uint32 i = 0; i < pointsCount; ++i)
        {
            points[i] = MirrorPoint(points[i]);
        }

        Color red(1.0f, 0.0f, 0.0f, 1.0f);
        Color blue(0.f, 0.f, 1.f, 1.f);

        const AABBox3& boundingBox = drawSystem->GetLandscapeProxy()->GetLandscapeBoundingBox();
        const Vector3 landSize = boundingBox.max - boundingBox.min;
        Vector2 offsetPoint = Vector2(boundingBox.min.x, boundingBox.min.y);

        float32 koef = static_cast<float32>(targetTexture->GetWidth()) / landSize.x;

        Vector2 startPoint = points[0];
        for (uint32 i = 1; i < pointsCount; ++i)
        {
            Vector2 endPoint = points[i];

            Vector2 startPosition = (startPoint - offsetPoint) * koef;
            Vector2 endPosition = (endPoint - offsetPoint) * koef;

            if (previewEnabled && isIntersectsLandscape && i == (points.size() - 1))
            {
                RenderSystem2D::Instance()->DrawLine(startPosition, endPosition, blue);
            }
            else
            {
                RenderSystem2D::Instance()->DrawLine(startPosition, endPosition, red);
            }

            startPoint = endPoint;
        }
    }

    RenderSystem2D::Instance()->EndRenderTargetPass();
}

void RulerToolSystem::ClearInternal()
{
    linePoints.clear();
    lengths.clear();
}

void RulerToolSystem::DisablePreview()
{
    previewEnabled = false;
    previewLength = -1.f;

    SendUpdatedLength();
}

void RulerToolSystem::SendUpdatedLength()
{
    float32 length = GetLength();
    float32 previewLength = GetPreviewLength();

    rulerToolLengthChanged.Emit(static_cast<SceneEditor2*>(GetScene()), length, previewLength);
}

float32 RulerToolSystem::GetLength()
{
    float32 length = -1.f;
    if (lengths.size() > 0)
    {
        length = lengths.back();
    }

    return length;
}

float32 RulerToolSystem::GetPreviewLength()
{
    float32 previewLength = -1.f;
    if (previewEnabled)
    {
        previewLength = this->previewLength;
    }

    return previewLength;
}

Vector2 RulerToolSystem::MirrorPoint(const Vector2& point) const
{
    const AABBox3& boundingBox = drawSystem->GetLandscapeProxy()->GetLandscapeBoundingBox();

    Vector2 newPoint = point;
    newPoint.y = (boundingBox.max.y - point.y) + boundingBox.min.y;

    return newPoint;
}
} // namespace DAVA
