#include "Classes/Modules/CanvasModule/CanvasDataAdapter.h"
#include "Classes/Modules/CanvasModule/CanvasData.h"

#include "Classes/Modules/DocumentsModule/DocumentData.h"

#include "Classes/UI/Preview/Data/CentralWidgetData.h"

#include <TArc/Core/ContextAccessor.h>

#include <Reflection/ReflectedTypeDB.h>

DAVA::FastName CanvasDataAdapter::scalePropertyName{ "scale" };
DAVA::FastName CanvasDataAdapter::positionPropertyName{ "position" };
DAVA::FastName CanvasDataAdapter::startValuePropertyName{ "start value" };
DAVA::FastName CanvasDataAdapter::lastValuePropertyName{ "last value" };

DAVA_VIRTUAL_REFLECTION_IMPL(CanvasDataAdapter)
{
    DAVA::ReflectionRegistrator<CanvasDataAdapter>::Begin()
    .Field(scalePropertyName.c_str(), &CanvasDataAdapter::GetScale, static_cast<void (CanvasDataAdapter::*)(DAVA::float32)>(&CanvasDataAdapter::SetScale))
    .Field(positionPropertyName.c_str(), &CanvasDataAdapter::GetPosition, nullptr)
    .Field(startValuePropertyName.c_str(), &CanvasDataAdapter::GetStartValue, nullptr)
    .Field(lastValuePropertyName.c_str(), &CanvasDataAdapter::GetLastValue, nullptr)
    .End();
}

CanvasDataAdapter::CanvasDataAdapter(DAVA::ContextAccessor* accessor_)
    : accessor(accessor_)
{
    DVASSERT(accessor != nullptr);
}

CanvasDataAdapter::~CanvasDataAdapter() = default;

void CanvasDataAdapter::MoveScene(const DAVA::Vector2& delta, bool force)
{
    using namespace DAVA;
    CanvasData* canvasData = GetCanvasData();
    if (canvasData == nullptr)
    {
        return;
    }

    Vector2 newPosition = GetPosition() + delta;
    canvasData->SetPosition(newPosition);
    canvasData->forceCentralize = false;
    if (force == false)
    {
        TryCentralizeScene();
    }
}

void CanvasDataAdapter::TryCentralizeScene()
{
    using namespace DAVA;

    CanvasData* canvasData = GetCanvasData();
    if (canvasData == nullptr)
    {
        return;
    }

    Vector2 position = GetPosition();
    Vector2 minPos = GetMinPos();
    Vector2 maxPos = GetMaxPos();
    Vector2 clampedNewPos(Clamp(position.x, minPos.x, maxPos.x), Clamp(position.y, minPos.y, maxPos.y));
    canvasData->SetPosition(clampedNewPos);
}

DAVA::Vector2 CanvasDataAdapter::GetPosition() const
{
    using namespace DAVA;

    const CanvasData* canvasData = GetCanvasData();
    if (canvasData == nullptr)
    {
        return Vector2(0.0f, 0.0f);
    }
    if (canvasData->forceCentralize)
    {
        return GetCenterPosition();
    }
    else
    {
        return canvasData->GetPosition();
    }
}

DAVA::Vector2 CanvasDataAdapter::GetCenterPosition() const
{
    const CanvasData* canvasData = GetCanvasData();
    if (canvasData == nullptr)
    {
        return DAVA::Vector2(0.0f, 0.0f);
    }
    return (GetViewSize() - canvasData->GetRootControlSize()) / 2.0f;
}

DAVA::Vector2 CanvasDataAdapter::GetStartValue() const
{
    using namespace DAVA;

    const CanvasData* canvasData = GetCanvasData();
    if (canvasData == nullptr)
    {
        return Vector2(0.0f, 0.0f);
    }

    return -1.0f * GetPosition();
}

DAVA::Vector2 CanvasDataAdapter::GetLastValue() const
{
    return GetStartValue() + GetViewSize();
}

DAVA::Vector2 CanvasDataAdapter::GetMaxPos() const
{
    using namespace DAVA;

    const CanvasData* canvasData = GetCanvasData();
    if (canvasData == nullptr)
    {
        return Vector2(0.0f, 0.0f);
    }

    Vector2 margin = canvasData->GetMargin();
    Vector2 topLeftToCenter = canvasData->GetRootControlSize() / 2.0f + canvasData->GetRootPosition();

    Vector2 viewSize = GetViewSize();

    Vector2 sizeDifference = viewSize / 2.0f - topLeftToCenter;

    //if control top left part is greater then half of view size without margin - also add a margin to make a border between document and editor
    if (sizeDifference.x < margin.x)
    {
        sizeDifference.x -= margin.x;
    }
    if (sizeDifference.y < margin.y)
    {
        sizeDifference.y -= margin.y;
    }
    Vector2 invertedSizeDifference = -sizeDifference;
    Vector2 absSizeDifference(std::max(0.0f, invertedSizeDifference.dx), std::max(0.0f, invertedSizeDifference.dy));
    Vector2 centerPosition = GetCenterPosition();

    return centerPosition + absSizeDifference;
}

DAVA::Vector2 CanvasDataAdapter::GetMinPos() const
{
    using namespace DAVA;

    const CanvasData* canvasData = GetCanvasData();
    if (canvasData == nullptr)
    {
        return Vector2(0.0f, 0.0f);
    }

    Vector2 margin = canvasData->GetMargin();
    Vector2 topLeftToCenter = canvasData->GetRootControlSize() / 2.0f + canvasData->GetRootPosition();
    Vector2 centerToBottomRight = canvasData->GetWorkAreaSize() - topLeftToCenter;
    Vector2 viewSize = GetViewSize();

    Vector2 sizeDifference = viewSize / 2.0f - centerToBottomRight;

    //if control top left part is greater then half of view size without margin - also add a margin to make a border between document and editor
    if (sizeDifference.dx < margin.dx)
    {
        sizeDifference.dx -= margin.dx;
    }
    if (sizeDifference.dy < margin.dy)
    {
        sizeDifference.dy -= margin.dy;
    }
    Vector2 invertedSizeDifference = -sizeDifference;
    Vector2 absSizeDifference(std::max(0.0f, invertedSizeDifference.dx), std::max(0.0f, invertedSizeDifference.dy));
    Vector2 centerPosition = GetCenterPosition();
    return centerPosition - absSizeDifference;
}

DAVA::Vector2 CanvasDataAdapter::GetRealMaxPos() const
{
    using namespace DAVA;

    const CanvasData* canvasData = GetCanvasData();
    if (canvasData == nullptr)
    {
        return Vector2(0.0f, 0.0f);
    }

    Vector2 position = GetPosition();
    Vector2 maxPos = GetMaxPos();
    return Vector2(std::max(position.x, maxPos.x), std::max(position.y, maxPos.y));
}

DAVA::Vector2 CanvasDataAdapter::GetRealMinPos() const
{
    using namespace DAVA;

    const CanvasData* canvasData = GetCanvasData();
    if (canvasData == nullptr)
    {
        return Vector2(0.0f, 0.0f);
    }

    Vector2 position = GetPosition();
    Vector2 minPos = GetMinPos();
    return Vector2(std::min(position.x, minPos.x), std::min(position.y, minPos.y));
}

DAVA::float32 CanvasDataAdapter::GetScale() const
{
    const CanvasData* canvasData = GetCanvasData();
    if (canvasData == nullptr)
    {
        return 1.0f;
    }

    return canvasData->GetScale();
}

void CanvasDataAdapter::SetScale(DAVA::float32 scale)
{
    SetScale(scale, GetViewSize() / 2.0f);
}

void CanvasDataAdapter::SetScale(DAVA::float32 scale, const DAVA::Vector2& referencePoint)
{
    using namespace DAVA;

    CanvasData* canvasData = GetCanvasData();
    DVASSERT(canvasData != nullptr);

    const Vector<float32>& predefinedScales = canvasData->GetPredefinedScales();

    scale = Clamp(scale, predefinedScales.front(), predefinedScales.back());
    DVASSERT(scale != 0.0f);
    Vector2 rootPoosition = MapFromScreenToRoot(referencePoint);

    canvasData->SetScale(scale);

    Vector2 screenPos12 = MapFromRootToScreen(rootPoosition);
    MoveScene(referencePoint - screenPos12);
}

DAVA::Vector2 CanvasDataAdapter::GetViewSize() const
{
    DAVA::DataContext* globalContext = accessor->GetGlobalContext();
    CentralWidgetData* centralWidgetData = globalContext->GetData<CentralWidgetData>();
    return centralWidgetData->GetViewSize();
}

CanvasData* CanvasDataAdapter::GetCanvasData() const
{
    using namespace DAVA;

    DataContext* active = accessor->GetActiveContext();
    if (active == nullptr)
    {
        return nullptr;
    }

    return active->GetData<CanvasData>();
}

DAVA::Vector2 CanvasDataAdapter::MapFromRootToScreen(const DAVA::Vector2& absValue) const
{
    return DAVA::Vector2(MapFromRootToScreen(absValue.x, DAVA::Vector2::AXIS_X), MapFromRootToScreen(absValue.y, DAVA::Vector2::AXIS_Y));
}

DAVA::Vector2 CanvasDataAdapter::MapFromScreenToRoot(const DAVA::Vector2& position) const
{
    return DAVA::Vector2(MapFromScreenToRoot(position.x, DAVA::Vector2::AXIS_X), MapFromScreenToRoot(position.y, DAVA::Vector2::AXIS_Y));
}

DAVA::float32 CanvasDataAdapter::MapFromRootToScreen(DAVA::float32 absValue, DAVA::Vector2::eAxis axis) const
{
    using namespace DAVA;
    const CanvasData* canvasData = GetCanvasData();
    if (canvasData == nullptr)
    {
        return absValue;
    }

    float32 scale = canvasData->GetScale();
    Vector2 startValue = GetStartValue();
    return absValue * scale - startValue[axis];
}

DAVA::float32 CanvasDataAdapter::MapFromScreenToRoot(DAVA::float32 position, DAVA::Vector2::eAxis axis) const
{
    using namespace DAVA;
    const CanvasData* canvasData = GetCanvasData();
    if (canvasData == nullptr)
    {
        return position;
    }

    float32 scale = canvasData->GetScale();
    Vector2 startValue = GetStartValue();
    return std::round((startValue[axis] + position) / scale);
}
