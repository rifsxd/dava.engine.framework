#pragma once

#include "Classes/EditorSystems/EditorSystemsManager.h"

#include <UI/UIControl.h>
#include <Functional/TrackedObject.h>
#include <Math/Color.h>

#include <memory>

class VisibleValueProperty;

class ControlContainer
{
public:
    explicit ControlContainer(const eArea area);
    virtual ~ControlContainer();

    ControlContainer(const ControlContainer&) = delete;
    ControlContainer(ControlContainer&&) = delete;

    ControlContainer& operator=(const ControlContainer&) = delete;
    ControlContainer& operator==(ControlContainer&&) = delete;

    eArea GetArea() const;
    virtual void InitFromGD(const DAVA::UIGeometricData& gd_) = 0;

    //this methods are used to temporally show/hide some HUD controls, like pivot or rotate
    //we can not use control visibility flag, because visibility state will be overwritten by show/hide whole HUD
    void SetSystemVisible(bool visible);
    bool GetSystemVisible() const;

    void AddChild(std::unique_ptr<ControlContainer>&& child);

    void AddToParent(DAVA::UIControl* parent);
    void RemoveFromParent(DAVA::UIControl* parent);

    void SetVisible(bool visible);
    void SetName(const DAVA::String& name);

    bool IsPointInside(const DAVA::Vector2& point) const;
    bool IsHiddenForDebug() const;
    bool GetVisibilityFlag() const;
    bool IsDrawableControl(DAVA::UIControl* control) const;

protected:
    const eArea area = eArea::NO_AREA;
    bool systemVisible = true;

    DAVA::RefPtr<DAVA::UIControl> drawable;
    DAVA::Vector<std::unique_ptr<ControlContainer>> children;
};

class HUDContainer : public ControlContainer, public DAVA::TrackedObject
{
public:
    explicit HUDContainer(const ControlNode* node);
    void InitFromGD(const DAVA::UIGeometricData& geometricData) override;

private:
    ~HUDContainer() override = default;
    const ControlNode* node = nullptr;
    VisibleValueProperty* visibleProperty = nullptr;
    //weak pointer to control to wrap around
    DAVA::UIControl* control = nullptr;
};

class FrameControl : public ControlContainer
{
public:
    enum eBorder
    {
        TOP,
        BOTTOM,
        LEFT,
        RIGHT,
        COUNT
    };

    enum eType
    {
        SELECTION,
        HIGHLIGHT,
        SELECTION_RECT
    };
    explicit FrameControl(eType type, DAVA::ContextAccessor* accessor);
    void SetRect(const DAVA::Rect& rect);
    DAVA::Rect GetAbsoluteRect() const;

protected:
    void InitFromGD(const DAVA::UIGeometricData& geometricData) override;
    DAVA::Rect GetSubControlRect(const DAVA::Rect& rect, eBorder border) const;

    eType type = SELECTION;
    DAVA::float32 lineThickness = 1.0f;
};

class FrameRectControl : public ControlContainer
{
public:
    explicit FrameRectControl(const eArea area_, DAVA::ContextAccessor* accessor);

private:
    ~FrameRectControl() = default;
    void InitFromGD(const DAVA::UIGeometricData& geometricData) override;
    DAVA::Vector2 GetPos(const DAVA::Rect& rect) const;

    DAVA::Vector2 rectSize;
};

class PivotPointControl : public ControlContainer
{
public:
    explicit PivotPointControl(DAVA::ContextAccessor* accessor);

private:
    ~PivotPointControl() = default;
    void InitFromGD(const DAVA::UIGeometricData& geometricData) override;
};

class RotateControl : public ControlContainer
{
public:
    explicit RotateControl(DAVA::ContextAccessor* accessor);

private:
    ~RotateControl() = default;
    void InitFromGD(const DAVA::UIGeometricData& geometricData) override;
};

void SetupHUDMagnetLineControl(DAVA::UIControl* control, DAVA::ContextAccessor* accessor);
void SetupHUDMagnetRectControl(DAVA::UIControl* control, DAVA::ContextAccessor* accessor);

std::unique_ptr<ControlContainer> CreateHighlightRect(const ControlNode* node, DAVA::ContextAccessor* accessor);
