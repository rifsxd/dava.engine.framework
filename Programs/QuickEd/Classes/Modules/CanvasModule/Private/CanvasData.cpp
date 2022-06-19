#include "Modules/CanvasModule/CanvasData.h"

#include <Base/FastName.h>
#include <Math/Vector.h>
#include <Logger/Logger.h>

DAVA::FastName CanvasData::positionPropertyName{ "displacement" };
DAVA::FastName CanvasData::workAreaSizePropertyName{ "work area size" };
DAVA::FastName CanvasData::rootPositionPropertyName{ "root control position" };
DAVA::FastName CanvasData::scalePropertyName{ "scale" };
DAVA::FastName CanvasData::predefinedScalesPropertyName{ "predefined scales" };
DAVA::FastName CanvasData::referencePointPropertyName{ "reference point" };

DAVA_VIRTUAL_REFLECTION_IMPL(CanvasData)
{
    DAVA::ReflectionRegistrator<CanvasData>::Begin()
    .Field(workAreaSizePropertyName.c_str(), &CanvasData::GetWorkAreaSize, &CanvasData::SetWorkAreaSize)
    .Field(positionPropertyName.c_str(), &CanvasData::GetPosition, &CanvasData::SetPosition)
    .Field(rootPositionPropertyName.c_str(), &CanvasData::GetRootPosition, &CanvasData::SetRootPosition)
    .Field(scalePropertyName.c_str(), &CanvasData::GetScale, &CanvasData::SetScale)
    .Field(predefinedScalesPropertyName.c_str(), &CanvasData::GetPredefinedScales, nullptr)
    .End();
}

CanvasData::CanvasData()
{
    predefinedScales = { 0.25f, 0.50f, 1.0f, 2.00f, 4.00f,
                         8.00f, 16.0f, 24.0f, 32.0f };
}

DAVA::Vector2 CanvasData::GetWorkAreaSize() const
{
    return workAreaSize * scale;
}

DAVA::Vector2 CanvasData::GetRootControlSize() const
{
    return rootControlSize * scale;
}

void CanvasData::SetWorkAreaSize(const DAVA::Vector2& size)
{
    DVASSERT(size.dx >= 0.0f && size.dy >= 0.0f);
    workAreaSize = size;
    if (workAreaSize.IsZero())
    {
        position.SetZero();
    }
}

DAVA::Vector2 CanvasData::GetPosition() const
{
    return position;
}

void CanvasData::SetPosition(const DAVA::Vector2& position_)
{
    position = position_;
}

DAVA::Vector2 CanvasData::GetRootPosition() const
{
    return rootRelativePosition * scale;
}

void CanvasData::SetRootPosition(const DAVA::Vector2& rootPosition_)
{
    DVASSERT(rootPosition_.dx >= 0.0f && rootPosition_.dy >= 0.0f);
    rootRelativePosition = rootPosition_;
}

DAVA::float32 CanvasData::GetScale() const
{
    return scale;
}

void CanvasData::SetScale(DAVA::float32 scale_)
{
    using namespace DAVA;
    scale = Clamp(scale_, predefinedScales.front(), predefinedScales.back());
    DVASSERT(scale != 0.0f);
}

const DAVA::Vector<DAVA::float32>& CanvasData::GetPredefinedScales() const
{
    return predefinedScales;
}

DAVA::float32 CanvasData::GetNextScale(DAVA::int32 ticksCount) const
{
    using namespace DAVA;
    DVASSERT(ticksCount > 0);
    auto iter = std::upper_bound(predefinedScales.begin(), predefinedScales.end(), scale);
    if (iter == predefinedScales.end())
    {
        return scale;
    }
    ticksCount--;
    int32 distance = static_cast<int32>(std::distance(iter, predefinedScales.end())) - 1;
    ticksCount = std::min(distance, ticksCount);
    std::advance(iter, ticksCount);
    return *iter;
}

DAVA::float32 CanvasData::GetPreviousScale(DAVA::int32 ticksCount) const
{
    using namespace DAVA;
    DVASSERT(ticksCount < 0);
    auto iter = std::lower_bound(predefinedScales.begin(), predefinedScales.end(), scale);
    if (iter == predefinedScales.end())
    {
        return scale;
    }
    int32 distance = static_cast<int32>(std::distance(iter, predefinedScales.begin()));
    ticksCount = std::max(ticksCount, distance);
    std::advance(iter, ticksCount);
    return *iter;
}

DAVA::Vector2 CanvasData::GetMargin() const
{
    return margin;
}
