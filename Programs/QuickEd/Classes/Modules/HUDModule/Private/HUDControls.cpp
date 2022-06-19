#include "Classes/Modules/HUDModule/Private/HUDControls.h"

#include "Model/ControlProperties/RootProperty.h"
#include "Model/ControlProperties/VisibleValueProperty.h"
#include "Classes/EditorSystems/EditorTransformSystem.h"
#include "Classes/EditorSystems/UserAssetsSettings.h"

#include <TArc/Core/ContextAccessor.h>
#include <TArc/DataProcessing/DataContext.h>

#include <UI/UIControlBackground.h>
#include <UI/UIControlSystem.h>
#include <UI/Layouts/UIAnchorComponent.h>
#include <Render/2D/Sprite.h>
#include <Render/2D/Systems/RenderSystem2D.h>

using namespace DAVA;

namespace HUDControlsDetails
{
RefPtr<UIControl> CreateFrameBorderControl(FrameControl::eBorder border)
{
    RefPtr<UIControl> control(new UIControl(Rect(1.0f, 1.0f, 1.0f, 1.0f)));
    UIAnchorComponent* anchor = control->GetOrCreateComponent<UIAnchorComponent>();
    anchor->SetLeftAnchorEnabled(true);
    anchor->SetRightAnchorEnabled(true);
    anchor->SetTopAnchorEnabled(true);
    anchor->SetBottomAnchorEnabled(true);
    switch (border)
    {
    case FrameControl::TOP:
        anchor->SetBottomAnchorEnabled(false);
        break;
    case FrameControl::BOTTOM:
        anchor->SetTopAnchorEnabled(false);
        break;
    case FrameControl::LEFT:
        anchor->SetRightAnchorEnabled(false);
        break;
    case FrameControl::RIGHT:
        anchor->SetLeftAnchorEnabled(false);
        break;
    default:
        DVASSERT("!impossible value for frame control position");
    }
    return control;
}
}

ControlContainer::ControlContainer(const eArea area_)
    : area(area_)
    , drawable(new UIControl())
{
}

ControlContainer::~ControlContainer() = default;

eArea ControlContainer::GetArea() const
{
    return area;
}

void ControlContainer::SetSystemVisible(bool visible)
{
    systemVisible = visible;
}

bool ControlContainer::GetSystemVisible() const
{
    return systemVisible;
}

void ControlContainer::AddChild(std::unique_ptr<ControlContainer>&& child)
{
    drawable->AddControl(child->drawable.Get());
    children.emplace_back(std::move(child));
}

void ControlContainer::AddToParent(DAVA::UIControl* parent)
{
    parent->AddControl(drawable.Get());
}

void ControlContainer::RemoveFromParent(DAVA::UIControl* parent)
{
    parent->RemoveControl(drawable.Get());
}

void ControlContainer::SetVisible(bool visible)
{
    drawable->SetVisibilityFlag(visible);
}

void ControlContainer::SetName(const DAVA::String& name)
{
    drawable->SetName(name);
}

bool ControlContainer::IsPointInside(const Vector2& point) const
{
    return drawable->IsPointInside(point);
}

bool ControlContainer::IsHiddenForDebug() const
{
    return drawable->IsHiddenForDebug();
}

bool ControlContainer::GetVisibilityFlag() const
{
    return drawable->GetVisibilityFlag();
}

bool ControlContainer::IsDrawableControl(DAVA::UIControl* control) const
{
    return drawable == control;
}

HUDContainer::HUDContainer(const ControlNode* node_)
    : ControlContainer(eArea::NO_AREA)
    , node(node_)
{
    DVASSERT(nullptr != node);
    drawable->SetName(FastName("HudContainer_of_control"));
    control = node->GetControl();
    visibleProperty = node->GetRootProperty()->GetVisibleProperty();
    DVASSERT(nullptr != control && nullptr != visibleProperty);
}

void HUDContainer::InitFromGD(const UIGeometricData& gd)
{
    bool contolIsInValidState = systemVisible && gd.size.dx >= 0.0f && gd.size.dy >= 0.0f && gd.scale.dx > 0.0f && gd.scale.dy > 0.0f;
    bool containerVisible = contolIsInValidState && visibleProperty->GetVisibleInEditor();
    if (containerVisible)
    {
        PackageBaseNode* parent = node->GetParent();
        while (containerVisible && nullptr != parent)
        {
            ControlNode* parentControlNode = dynamic_cast<ControlNode*>(parent);
            if (parentControlNode == nullptr)
            {
                break;
            }
            containerVisible &= parentControlNode->GetRootProperty()->GetVisibleProperty()->GetVisibleInEditor();
            parent = parent->GetParent();
        }
    }
    drawable->SetVisibilityFlag(containerVisible);
    if (containerVisible)
    {
        const DAVA::Vector2& minimumSize = EditorTransformSystem::GetMinimumSize();
        DAVA::Vector2 actualSize(gd.size * gd.scale);
        DAVA::UIGeometricData changedGD = gd;
        bool controlIsMoveOnly = actualSize.dx < minimumSize.dx && actualSize.dy < minimumSize.dy;
        if (controlIsMoveOnly)
        {
            changedGD.position -= ::Rotate((minimumSize - actualSize) / 2.0f, changedGD.angle);
            changedGD.size = minimumSize / gd.scale;
        }

        Rect ur(changedGD.position - ::Rotate(changedGD.pivotPoint, changedGD.angle) * changedGD.scale, changedGD.size * changedGD.scale);
        drawable->SetRect(ur);

        drawable->SetAngle(changedGD.angle);

        for (const std::unique_ptr<ControlContainer>& child : children)
        {
            eArea area = child->GetArea();
            bool childVisible = child->GetSystemVisible() && changedGD.scale.x > 0.0f && changedGD.scale.y > 0.0f;
            if (area != eArea::FRAME_AREA)
            {
                childVisible &= !controlIsMoveOnly;
            }
            child->SetVisible(childVisible);
            if (childVisible)
            {
                child->InitFromGD(changedGD);
            }
        }
    }
}

FrameControl::FrameControl(eType type_, DAVA::ContextAccessor* accessor)
    : ControlContainer(eArea::FRAME_AREA)
    , type(type_)
{
    drawable->SetName(FastName("Frame_Control"));

    switch (type)
    {
    case SELECTION:
    case SELECTION_RECT:
        lineThickness = 1.0f;
        break;
    case HIGHLIGHT:
        lineThickness = 2.0f;
        break;
    default:
        DVASSERT(false, "set line thickness please");
        break;
    }

    UserAssetsSettings* settings = accessor->GetGlobalContext()->GetData<UserAssetsSettings>();
    for (uint32 i = 0; i < eBorder::COUNT; ++i)
    {
        FrameControl::eBorder border = static_cast<FrameControl::eBorder>(i);

        RefPtr<UIControl> control(new UIControl());
        control->SetName(FastName(String("border_of_") + drawable->GetName().c_str()));
        UIControlBackground* background = control->GetOrCreateComponent<UIControlBackground>();
        background->SetPerPixelAccuracyType(UIControlBackground::PER_PIXEL_ACCURACY_ENABLED);
        background->SetDrawType(UIControlBackground::DRAW_FILL);
        switch (type)
        {
        case SELECTION:
            background->SetColor(settings->hudRectColor);
            break;
        case SELECTION_RECT:
            background->SetColor(settings->selectionRectColor);
            break;
        case HIGHLIGHT:
            background->SetColor(settings->highlightColor);
            break;
        default:
            break;
        }
        drawable->AddControl(control.Get());
    }
}

void FrameControl::InitFromGD(const UIGeometricData& gd)
{
    Rect currentRect(Vector2(0.0f, 0.0f), gd.size * gd.scale);
    SetRect(currentRect);
}

void FrameControl::SetRect(const DAVA::Rect& rect)
{
    drawable->SetRect(rect);
    const auto& children = drawable->GetChildren();
    auto iter = children.begin();
    for (uint32 i = 0; i < eBorder::COUNT; ++i, ++iter)
    {
        FrameControl::eBorder border = static_cast<FrameControl::eBorder>(i);
        (*iter)->SetRect(GetSubControlRect(rect, border));
    }
}

Rect FrameControl::GetAbsoluteRect() const
{
    return drawable->GetAbsoluteRect();
}

Rect FrameControl::GetSubControlRect(const DAVA::Rect& rect, eBorder border) const
{
    switch (border)
    {
    case TOP:
        return Rect(0.0f, 0.0f, rect.dx, lineThickness);
    case BOTTOM:
        return Rect(0.0f, rect.dy - lineThickness, rect.dx, lineThickness);
    case LEFT:
        return Rect(0.0f, 0.0f, lineThickness, rect.dy);
    case RIGHT:
        return Rect(rect.dx - lineThickness, 0.0f, lineThickness, rect.dy);
    default:
        DVASSERT(!"wrong border passed to frame control");
        return Rect(0.0f, 0.0f, 0.0f, 0.0f);
    }
}

FrameRectControl::FrameRectControl(const eArea area_, DAVA::ContextAccessor* accessor)
    : ControlContainer(area_)
{
    UserAssetsSettings* settings = accessor->GetGlobalContext()->GetData<UserAssetsSettings>();

    drawable->SetName(FastName("Frame_Rect_Control"));
    UIControlBackground* background = drawable->GetOrCreateComponent<UIControlBackground>();
    FilePath spritePath;
    switch (area)
    {
    case eArea::TOP_LEFT_AREA:
    case eArea::TOP_RIGHT_AREA:
    case eArea::BOTTOM_LEFT_AREA:
    case eArea::BOTTOM_RIGHT_AREA:
        spritePath = settings->cornerRectPath2;
        rectSize = Vector2(8.0f, 8.0f);
        break;
    case eArea::TOP_CENTER_AREA:
    case eArea::BOTTOM_CENTER_AREA:
    case eArea::CENTER_LEFT_AREA:
    case eArea::CENTER_RIGHT_AREA:
        spritePath = settings->borderRectPath2;
        rectSize = Vector2(8.0f, 8.0f);
        break;
    default:
        DVASSERT(false, "invalid area passed to FrameRectControl");
        break;
    }
    ScopedPtr<Sprite> sprite(Sprite::CreateFromSourceFile(spritePath, true, false));
    background->SetSprite(sprite, 0);
    background->SetDrawType(UIControlBackground::DRAW_SCALE_TO_RECT);
    background->SetPerPixelAccuracyType(UIControlBackground::PER_PIXEL_ACCURACY_ENABLED);
}

void FrameRectControl::InitFromGD(const UIGeometricData& gd)
{
    Rect subRect(Vector2(), rectSize);
    Rect parentRect(Vector2(0.0f, 0.0f), gd.size * gd.scale);
    subRect.SetCenter(GetPos(parentRect));
    drawable->SetRect(subRect);
}

Vector2 FrameRectControl::GetPos(const DAVA::Rect& rect) const
{
    switch (area)
    {
    case eArea::TOP_LEFT_AREA:
        return Vector2(0.0f, 0.0f);
    case eArea::TOP_CENTER_AREA:
        return Vector2(rect.dx / 2.0f, 0.0f);
    case eArea::TOP_RIGHT_AREA:
        return Vector2(rect.dx, 0.0f);
    case eArea::CENTER_LEFT_AREA:
        return Vector2(0, rect.dy / 2.0f);
    case eArea::CENTER_RIGHT_AREA:
        return Vector2(rect.dx, rect.dy / 2.0f);
    case eArea::BOTTOM_LEFT_AREA:
        return Vector2(0, rect.dy);
    case eArea::BOTTOM_CENTER_AREA:
        return Vector2(rect.dx / 2.0f, rect.dy);
    case eArea::BOTTOM_RIGHT_AREA:
        return Vector2(rect.dx, rect.dy);
    default:
        DVASSERT(!"wrong area passed to hud control");
        return Vector2(0.0f, 0.0f);
    }
}

PivotPointControl::PivotPointControl(DAVA::ContextAccessor* accessor)
    : ControlContainer(eArea::PIVOT_POINT_AREA)
{
    UserAssetsSettings* settings = accessor->GetGlobalContext()->GetData<UserAssetsSettings>();
    drawable->SetName(FastName("pivot_point_control"));
    UIControlBackground* background = drawable->GetOrCreateComponent<UIControlBackground>();
    ScopedPtr<Sprite> sprite(Sprite::CreateFromSourceFile(settings->pivotPointPath2, true, false));
    background->SetSprite(sprite, 0);
    background->SetDrawType(UIControlBackground::DRAW_SCALE_TO_RECT);
    background->SetPerPixelAccuracyType(UIControlBackground::PER_PIXEL_ACCURACY_ENABLED);
}

void PivotPointControl::InitFromGD(const UIGeometricData& gd)
{
    Rect rect(Vector2(), Vector2(15.0f, 15.0f));
    rect.SetCenter(gd.pivotPoint * gd.scale);
    drawable->SetRect(rect);
}

RotateControl::RotateControl(DAVA::ContextAccessor* accessor)
    : ControlContainer(eArea::ROTATE_AREA)
{
    UserAssetsSettings* settings = accessor->GetGlobalContext()->GetData<UserAssetsSettings>();
    drawable->SetName(FastName("rotate_control"));
    UIControlBackground* background = drawable->GetOrCreateComponent<UIControlBackground>();
    ScopedPtr<Sprite> sprite(Sprite::CreateFromSourceFile(settings->rotatePath2, true, false));
    background->SetSprite(sprite, 0);
    background->SetDrawType(UIControlBackground::DRAW_SCALE_TO_RECT);
    background->SetPerPixelAccuracyType(UIControlBackground::PER_PIXEL_ACCURACY_ENABLED);
}

void RotateControl::InitFromGD(const UIGeometricData& gd)
{
    const DAVA::Vector2 rectSize(15.0f, 15.0f);
    Rect rect(Vector2(0.0f, 0.0f), rectSize);
    Rect controlRect(Vector2(0.0f, 0.0f), gd.size * gd.scale);

    const int margin = 5;
    rect.SetCenter(Vector2(controlRect.dx / 2.0f, controlRect.GetPosition().y - rectSize.y - margin));

    drawable->SetRect(rect);
}

void SetupHUDMagnetLineControl(UIControl* control, DAVA::ContextAccessor* accessor)
{
    UserAssetsSettings* settings = accessor->GetGlobalContext()->GetData<UserAssetsSettings>();

    UIControlBackground* background = control->GetOrCreateComponent<UIControlBackground>();
    background->SetPerPixelAccuracyType(UIControlBackground::PER_PIXEL_ACCURACY_ENABLED);
    ScopedPtr<Sprite> sprite(Sprite::CreateFromSourceFile(settings->magnetLinePath2));
    background->SetSprite(sprite, 0);
    background->SetDrawType(UIControlBackground::DRAW_TILED);
    control->SetName("Magnet_line");
}

void SetupHUDMagnetRectControl(UIControl* parentControl, DAVA::ContextAccessor* accessor)
{
    UserAssetsSettings* settings = accessor->GetGlobalContext()->GetData<UserAssetsSettings>();
    for (int i = 0; i < FrameControl::eBorder::COUNT; ++i)
    {
        FrameControl::eBorder border = static_cast<FrameControl::eBorder>(i);
        RefPtr<UIControl> control(HUDControlsDetails::CreateFrameBorderControl(border));

        UIControlBackground* background = control->GetOrCreateComponent<UIControlBackground>();
        background->SetPerPixelAccuracyType(UIControlBackground::PER_PIXEL_ACCURACY_ENABLED);
        ScopedPtr<Sprite> sprite(Sprite::CreateFromSourceFile(settings->magnetRectPath2));
        background->SetSprite(sprite, 0);
        background->SetDrawType(UIControlBackground::DRAW_TILED);
        control->SetName("magnet_rect_border");

        control->SetName(FastName(String("border_of_magnet_rect")));
        parentControl->AddControl(control.Get());
    }
}

std::unique_ptr<ControlContainer> CreateHighlightRect(const ControlNode* node, DAVA::ContextAccessor* accessor)
{
    std::unique_ptr<ControlContainer> container(new HUDContainer(node));
    container->SetName("HUD_rect_container");
    std::unique_ptr<ControlContainer> frame(new FrameControl(FrameControl::HIGHLIGHT, accessor));
    frame->SetName("HUD_rect_frame_control");
    container->AddChild(std::move(frame));

    const UIGeometricData& gd = node->GetControl()->GetGeometricData();
    container->InitFromGD(gd);
    return container;
}
