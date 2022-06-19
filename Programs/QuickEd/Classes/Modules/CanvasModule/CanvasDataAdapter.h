#pragma once

#include <Math/Vector.h>
#include <Reflection/ReflectionRegistrator.h>

namespace DAVA
{
class ContextAccessor;
}

class CanvasData;
class CentralWidgetData;

class CanvasDataAdapter : public DAVA::ReflectionBase
{
public:
    explicit CanvasDataAdapter(DAVA::ContextAccessor* accessor);
    ~CanvasDataAdapter() override;

    static DAVA::FastName scalePropertyName;

    static DAVA::FastName positionPropertyName;

    //values displayed on screen. Used by rulers, guides and grid
    static DAVA::FastName startValuePropertyName;
    static DAVA::FastName lastValuePropertyName;

    void MoveScene(const DAVA::Vector2& delta, bool force = false);
    void TryCentralizeScene();

    DAVA::Vector2 GetPosition() const;
    DAVA::Vector2 GetCenterPosition() const;

    DAVA::Vector2 GetStartValue() const;
    DAVA::Vector2 GetLastValue() const;

    DAVA::Vector2 GetMaxPos() const;
    DAVA::Vector2 GetMinPos() const;

    //canvas minimum position includes extra position
    //as an example if min pos is Vector2(-100, -100), but canvas moved to Vector2(-200, -200) - negative overflow will be Vector2(-200, -200)
    DAVA::Vector2 GetRealMaxPos() const;

    //canvas maximum position includes extra position
    //as an example if min pos is Vector2(-100, -100), but canvas moved to Vector2(-200, -200) - negative overflow will be Vector2(-200, -200)
    DAVA::Vector2 GetRealMinPos() const;

    DAVA::float32 GetScale() const;
    void SetScale(DAVA::float32 scale);
    void SetScale(DAVA::float32 scale, const DAVA::Vector2& referencePoint);

    DAVA::Vector2 GetViewSize() const;

    DAVA::Vector2 MapFromRootToScreen(const DAVA::Vector2& absValue) const;
    DAVA::Vector2 MapFromScreenToRoot(const DAVA::Vector2& position) const;

    DAVA::float32 MapFromRootToScreen(DAVA::float32 absValue, DAVA::Vector2::eAxis axis) const;
    DAVA::float32 MapFromScreenToRoot(DAVA::float32 position, DAVA::Vector2::eAxis axis) const;

private:
    CanvasData* GetCanvasData() const;

    DAVA::ContextAccessor* accessor = nullptr;

    DAVA_VIRTUAL_REFLECTION(CanvasDataAdapter, DAVA::ReflectionBase);
};
