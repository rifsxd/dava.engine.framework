#include "Classes/Modules/HUDModule/Private/HUDSystem.h"
#include "Classes/Modules/DocumentsModule/EditorSystemsData.h"

#include "Classes/Modules/HUDModule/Private/HUDControls.h"

#include "Modules/DocumentsModule/DocumentData.h"

#include "Model/PackageHierarchy/ControlNode.h"
#include "Model/PackageHierarchy/PackageNode.h"
#include "Model/PackageHierarchy/PackageControlsNode.h"
#include "Model/ControlProperties/RootProperty.h"
#include "Model/ControlProperties/VisibleValueProperty.h"

#include "Classes/EditorSystems/ControlTransformationSettings.h"

#include <TArc/Core/ContextAccessor.h>
#include <TArc/Core/FieldBinder.h>
#include <TArc/DataProcessing/DataContext.h>
#include <TArc/Utils/Utils.h>

#include <Base/BaseTypes.h>
#include <UI/UIControl.h>
#include <UI/UIEvent.h>

using namespace DAVA;

namespace
{
const Array<eArea, 2> AreasToHide = { { eArea::PIVOT_POINT_AREA, eArea::ROTATE_AREA } };
}

std::unique_ptr<ControlContainer> CreateControlContainer(eArea area, DAVA::ContextAccessor* accessor)
{
    switch (area)
    {
    case eArea::PIVOT_POINT_AREA:
        return std::unique_ptr<ControlContainer>(new PivotPointControl(accessor));
    case eArea::ROTATE_AREA:
        return std::unique_ptr<ControlContainer>(new RotateControl(accessor));
    case eArea::TOP_LEFT_AREA:
    case eArea::TOP_CENTER_AREA:
    case eArea::TOP_RIGHT_AREA:
    case eArea::CENTER_LEFT_AREA:
    case eArea::CENTER_RIGHT_AREA:
    case eArea::BOTTOM_LEFT_AREA:
    case eArea::BOTTOM_CENTER_AREA:
    case eArea::BOTTOM_RIGHT_AREA:
        return std::unique_ptr<ControlContainer>(new FrameRectControl(area, accessor));
    case eArea::FRAME_AREA:
        return std::unique_ptr<ControlContainer>(new FrameControl(FrameControl::SELECTION, accessor));
    default:
        DVASSERT(!"unacceptable value of area");
        return std::unique_ptr<ControlContainer>(nullptr);
    }
}

struct HUDSystem::HUD
{
    HUD(ControlNode* node, HUDSystem* hudSystem, UIControl* hudControl);
    ~HUD();
    ControlNode* node = nullptr;
    UIControl* control = nullptr;
    UIControl* hudControl = nullptr;
    std::unique_ptr<ControlContainer> container;
    Map<eArea, ControlContainer*> hudControls;
};

HUDSystem::HUD::HUD(ControlNode* node_, HUDSystem* hudSystem, UIControl* hudControl_)
    : node(node_)
    , control(node_->GetControl())
    , hudControl(hudControl_)
    , container(new HUDContainer(node_))
{
    container->SetName(String("Container_for_HUD_controls_of_node"));
    DAVA::Vector<eArea> areas;
    if (node->GetParent() != nullptr && node->GetParent()->GetControl() != nullptr)
    {
        areas.reserve(eArea::AREAS_COUNT);
        for (int area = eArea::AREAS_COUNT - 1; area >= eArea::AREAS_BEGIN; --area)
        {
            ControlTransformationSettings* settings = hudSystem->GetSettings();
            if ((settings->showPivot == false && area == eArea::PIVOT_POINT_AREA) ||
                (settings->showRotate == false && area == eArea::ROTATE_AREA))
            {
                continue;
            }
            areas.push_back(static_cast<eArea>(area));
        }
    }
    else
    {
        //custom areas for root control
        areas = {
            eArea::TOP_LEFT_AREA,
            eArea::TOP_CENTER_AREA,
            eArea::TOP_RIGHT_AREA,
            eArea::CENTER_LEFT_AREA,
            eArea::CENTER_RIGHT_AREA,
            eArea::BOTTOM_LEFT_AREA,
            eArea::BOTTOM_CENTER_AREA,
            eArea::BOTTOM_RIGHT_AREA
        };
    }
    for (eArea area : areas)
    {
        std::unique_ptr<ControlContainer> controlContainer = CreateControlContainer(area, hudSystem->GetAccessor());
        hudControls[area] = controlContainer.get();
        container->AddChild(std::move(controlContainer));
    }
    container->AddToParent(hudControl);
    container->InitFromGD(control->GetGeometricData());
}

HUDSystem::HUD::~HUD()
{
    container->RemoveFromParent(hudControl);
}

class HUDControl : public UIControl
{
    void Draw(const UIGeometricData& geometricData) override
    {
        UpdateLayout();
        UIControl::Draw(geometricData);
    }
};

HUDSystem::HUDSystem(DAVA::ContextAccessor* accessor)
    : BaseEditorSystem(accessor)
    , hudMap(CompareByLCA)
{
    systemsDataWrapper = accessor->CreateWrapper(DAVA::ReflectedTypeDB::Get<EditorSystemsData>());
    GetSystemsManager()->magnetLinesChanged.Connect(this, &HUDSystem::OnMagnetLinesChanged);
}

HUDSystem::~HUDSystem() = default;

eSystems HUDSystem::GetOrder() const
{
    return eSystems::HUD;
}

void HUDSystem::OnUpdate()
{
    using namespace DAVA;

    DataContext* activeContext = accessor->GetActiveContext();
    if (activeContext == nullptr)
    {
        hudMap.clear();
        SetNewArea(HUDAreaInfo());
        SetHighlight(nullptr);
        return;
    }

    SyncHudWithSelection();

    //show Rotate and Pivot only if only one control is selected
    bool showAreas = hudMap.size() == 1;

    for (const auto& iter : hudMap)
    {
        for (eArea area : AreasToHide)
        {
            auto hudControlsIter = iter.second->hudControls.find(area);
            if (hudControlsIter != iter.second->hudControls.end())
            {
                ControlContainer* controlContainer = hudControlsIter->second;
                controlContainer->SetSystemVisible(showAreas);
            }
        }
    }

    for (const auto& hudPair : hudMap)
    {
        const UIGeometricData& controlGD = hudPair.first->GetControl()->GetGeometricData();
        hudPair.second->container->InitFromGD(controlGD);
    }

    EditorSystemsData* systemsData = accessor->GetGlobalContext()->GetData<EditorSystemsData>();
    const EditorSystemsManager* systemsManager = GetSystemsManager();
    if (systemsManager->GetDragState() == eDragState::NoDrag &&
        systemsManager->GetDisplayState() != eDisplayState::Emulation &&
        systemsData->IsHighlightDisabled() == false)
    {
        ControlNode* node = systemsManager->GetControlNodeAtPoint(systemsManager->GetLastMousePos());
        SetHighlight(node);
    }
    else
    {
        SetHighlight(nullptr);
    }

    if (GetSystemsManager()->GetDragState() == eDragState::NoDrag)
    {
        UpdateHUDArea();
    }
}

void HUDSystem::Invalidate()
{
    SetHighlight(nullptr);
}

void HUDSystem::ProcessInput(UIEvent* currentInput, eInputSource inputSource)
{
    const EditorSystemsManager* systemsManager = GetSystemsManager();

    UIEvent::Phase phase = currentInput->phase;
    if (inputSource == eInputSource::SYSTEM)
    {
        hoveredPoint = currentInput->point;
    }
    else
    {
        pressedPoint -= systemsManager->GetMouseDelta();
    }

    switch (phase)
    {
    case UIEvent::Phase::DRAG:
        if (systemsManager->GetDragState() == eDragState::SelectByRect)
        {
            Vector2 point(pressedPoint);
            Vector2 size(hoveredPoint - pressedPoint);
            if (size.x < 0.0f)
            {
                point.x += size.x;
                size.x *= -1.0f;
            }
            if (size.y <= 0.0f)
            {
                point.y += size.y;
                size.y *= -1.0f;
            }
            selectionRectControl->SetRect(Rect(point, size));
            selectionRectChanged.Emit(selectionRectControl->GetAbsoluteRect());
        }
        break;
    default:
        break;
    }
}

void HUDSystem::SetHighlight(ControlNode* node)
{
    highlightChanged.Emit(node);
    HighlightNode(node);
}

void HUDSystem::HighlightNode(ControlNode* node)
{
    if (hoveredNodeControl != nullptr && node != nullptr && hoveredNodeControl->IsDrawableControl(node->GetControl()))
    {
        return;
    }
    if (hoveredNodeControl != nullptr)
    {
        hoveredNodeControl->RemoveFromParent(hudControl.Get());
        hoveredNodeControl = nullptr;
    }

    if (node != nullptr)
    {
        DataContext* activeContext = accessor->GetActiveContext();
        DVASSERT(activeContext != nullptr);
        DocumentData* documentData = activeContext->GetData<DocumentData>();
        const SelectedNodes& selectedNodes = documentData->GetSelectedNodes();

        if (selectedNodes.find(node) != selectedNodes.end())
        {
            return;
        }

        UIControl* targetControl = node->GetControl();
        DVASSERT(hoveredNodeControl == nullptr);
        hoveredNodeControl = CreateHighlightRect(node, GetAccessor());
        hoveredNodeControl->AddToParent(hudControl.Get());
    }
}

void HUDSystem::OnMagnetLinesChanged(const Vector<MagnetLineInfo>& magnetLines)
{
    static const float32 extraSizeValue = 50.0f;
    DVASSERT(magnetControls.size() == magnetTargetControls.size());

    const size_type magnetsSize = magnetControls.size();
    const size_type newMagnetsSize = magnetLines.size();
    if (newMagnetsSize < magnetsSize)
    {
        auto linesRIter = magnetControls.rbegin();
        auto rectsRIter = magnetTargetControls.rbegin();
        size_type count = magnetsSize - newMagnetsSize;
        for (size_type i = 0; i < count; ++i)
        {
            UIControl* lineControl = (*linesRIter++).Get();
            UIControl* targetRectControl = (*rectsRIter++).Get();
            hudControl->RemoveControl(lineControl);
            hudControl->RemoveControl(targetRectControl);
        }
        const auto& linesEnd = magnetControls.end();
        const auto& targetRectsEnd = magnetTargetControls.end();
        magnetControls.erase(linesEnd - count, linesEnd);
        magnetTargetControls.erase(targetRectsEnd - count, targetRectsEnd);
    }
    else if (newMagnetsSize > magnetsSize)
    {
        size_type count = newMagnetsSize - magnetsSize;

        magnetControls.reserve(count);
        magnetTargetControls.reserve(count);
        for (size_type i = 0; i < count; ++i)
        {
            RefPtr<UIControl> lineControl(new UIControl());
            lineControl->SetName(FastName("magnet_line_control"));
            ::SetupHUDMagnetLineControl(lineControl.Get(), accessor);
            hudControl->AddControl(lineControl.Get());
            magnetControls.emplace_back(lineControl);

            RefPtr<UIControl> rectControl(new UIControl());
            rectControl->SetName(FastName("rect_of_target_control_which_we_magnet_to"));
            ::SetupHUDMagnetRectControl(rectControl.Get(), accessor);
            hudControl->AddControl(rectControl.Get());
            magnetTargetControls.emplace_back(rectControl);
        }
    }
    DVASSERT(magnetLines.size() == magnetControls.size() && magnetControls.size() == magnetTargetControls.size());
    for (size_t i = 0, size = magnetLines.size(); i < size; ++i)
    {
        const MagnetLineInfo& line = magnetLines.at(i);
        const auto& gd = line.gd;
        auto linePos = line.rect.GetPosition();
        auto lineSize = line.rect.GetSize();

        linePos = ::Rotate(linePos, gd->angle);
        linePos *= gd->scale;
        lineSize[line.axis] *= gd->scale[line.axis];
        Vector2 gdPos = gd->position - ::Rotate(gd->pivotPoint * gd->scale, gd->angle);

        UIControl* lineControl = magnetControls.at(i).Get();
        float32 angle = line.gd->angle;
        Vector2 extraSize(line.axis == Vector2::AXIS_X ? extraSizeValue : 0.0f, line.axis == Vector2::AXIS_Y ? extraSizeValue : 0.0f);
        Vector2 extraPos = ::Rotate(extraSize, angle) / 2.0f;
        Rect lineRect(Vector2(linePos + gdPos) - extraPos, lineSize + extraSize);
        lineControl->SetRect(lineRect);
        lineControl->SetAngle(angle);

        linePos = line.targetRect.GetPosition();
        lineSize = line.targetRect.GetSize();

        linePos = ::Rotate(linePos, gd->angle);
        linePos *= gd->scale;
        lineSize *= gd->scale;

        UIControl* rectControl = magnetTargetControls.at(i).Get();
        rectControl->SetRect(Rect(linePos + gdPos, lineSize));
        rectControl->SetAngle(line.gd->angle);
    }
}

HUDAreaInfo HUDSystem::GetControlArea(const Vector2& pos, eSearchOrder searchOrder) const
{
    uint32 end = eArea::AREAS_BEGIN;
    int sign = 1;
    if (searchOrder == SEARCH_BACKWARD)
    {
        if (eArea::AREAS_BEGIN != eArea::AREAS_COUNT)
        {
            end = eArea::AREAS_COUNT - 1;
        }
        sign = -1;
    }
    for (uint32 i = eArea::AREAS_BEGIN; i < eArea::AREAS_COUNT; ++i)
    {
        for (auto iter = hudMap.rbegin(); iter != hudMap.rend(); ++iter)
        {
            const std::unique_ptr<HUD>& hud = iter->second;
            if (hud->container->GetVisibilityFlag() && !hud->container->IsHiddenForDebug())
            {
                eArea area = static_cast<eArea>(end + sign * i);
                auto hudControlsIter = hud->hudControls.find(area);
                if (hudControlsIter != hud->hudControls.end())
                {
                    const auto& controlContainer = hudControlsIter->second;
                    if (controlContainer->GetVisibilityFlag() && !controlContainer->IsHiddenForDebug() && controlContainer->IsPointInside(pos))
                    {
                        return HUDAreaInfo(hud->node, area);
                    }
                }
            }
        }
    }
    return HUDAreaInfo();
}

void HUDSystem::SetNewArea(const HUDAreaInfo& areaInfo)
{
    EditorSystemsManager* systemsManager = GetSystemsManager();
    HUDAreaInfo currentHUDArea = systemsManager->GetCurrentHUDArea();
    if (currentHUDArea.area != areaInfo.area || currentHUDArea.owner != areaInfo.owner)
    {
        systemsManager->SetActiveHUDArea(areaInfo);
    }
}

void HUDSystem::UpdateHUDArea()
{
    using namespace DAVA;

    bool findPivot = hudMap.size() == 1 && IsKeyPressed(eModifierKeys::CONTROL) && IsKeyPressed(eModifierKeys::ALT);
    eSearchOrder searchOrder = findPivot ? SEARCH_BACKWARD : SEARCH_FORWARD;
    SetNewArea(GetControlArea(GetSystemsManager()->GetLastMousePos(), searchOrder));
}

void HUDSystem::OnDragStateChanged(eDragState currentState, eDragState previousState)
{
    switch (currentState)
    {
    case eDragState::SelectByRect:
        DVASSERT(selectionRectControl == nullptr);
        selectionRectControl.reset(new FrameControl(FrameControl::SELECTION_RECT, accessor));
        selectionRectControl->AddToParent(hudControl.Get());
        selectionByRectStarted.Emit();
        break;
    case eDragState::DragScreen:
        UpdateHUDEnabled();
        break;
    case eDragState::DuplicateByAlt:
        SyncHudWithSelection();
        UpdateHUDArea();
        break;
    default:
        break;
    }

    switch (previousState)
    {
    case eDragState::SelectByRect:
        DVASSERT(selectionRectControl != nullptr);
        selectionRectControl->RemoveFromParent(hudControl.Get());
        selectionRectControl = nullptr;
        selectionByRectFinished.Emit();
        break;
    case eDragState::Transform:
        ClearMagnetLines();
        break;
    case eDragState::DragScreen:
        UpdateHUDEnabled();
        break;
    default:
        break;
    }
}

void HUDSystem::OnDisplayStateChanged(eDisplayState, eDisplayState)
{
    UpdateHUDEnabled();
}

CanvasControls HUDSystem::CreateCanvasControls()
{
    DVASSERT(hudControl.Valid() == false);

    hudControl.Set(new DAVA::UIControl());
    hudControl->SetName("hud_control");

    return { { hudControl } };
}

void HUDSystem::DeleteCanvasControls(const CanvasControls& canvasControls)
{
    hudMap.clear();
    hudControl = nullptr;
}

bool HUDSystem::CanProcessInput(DAVA::UIEvent* currentInput, eInputSource /*inputSource*/) const
{
    if (hudControl->IsVisible() == false)
    {
        return false;
    }
    eDisplayState displayState = GetSystemsManager()->GetDisplayState();
    eDragState dragState = GetSystemsManager()->GetDragState();
    return (displayState == eDisplayState::Edit || displayState == eDisplayState::Preview)
    && dragState != eDragState::Transform;
}

eDragState HUDSystem::RequireNewState(DAVA::UIEvent* currentInput, eInputSource /*inputSource*/)
{
    eDragState dragState = GetSystemsManager()->GetDragState();
    //ignore all input devices except mouse while selecting by rect
    if (dragState == eDragState::SelectByRect && currentInput->device != eInputDevices::MOUSE)
    {
        return eDragState::SelectByRect;
    }
    if (dragState == eDragState::Transform || dragState == eDragState::DragScreen)
    {
        return eDragState::NoDrag;
    }

    Vector2 point = currentInput->point;
    if (currentInput->phase == UIEvent::Phase::BEGAN
        && dragState != eDragState::SelectByRect)
    {
        pressedPoint = point;
    }
    if (currentInput->phase == UIEvent::Phase::DRAG)
    {
        //if we in selectByRect and still drag mouse - continue this state
        if (dragState == eDragState::SelectByRect)
        {
            return eDragState::SelectByRect;
        }
        //check that we can draw rect
        Vector<ControlNode*> nodesUnderPoint;
        auto predicate = [point](const ControlNode* node) -> bool {
            const auto visibleProp = node->GetRootProperty()->GetVisibleProperty();
            DVASSERT(node->GetControl() != nullptr);
            return visibleProp->GetVisibleInEditor() && node->GetControl()->IsPointInside(point);
        };
        GetSystemsManager()->CollectControlNodes(std::back_inserter(nodesUnderPoint), predicate);
        bool noHudableControls = nodesUnderPoint.empty() || (nodesUnderPoint.size() == 1 && nodesUnderPoint.front()->GetParent()->GetControl() == nullptr);

        if (noHudableControls)
        {
            //distinguish between mouse click and mouse drag sometimes is less than few pixels
            //so lets select by rect only if we sure that is not mouse click
            Vector2 rectSize(pressedPoint - point);
            ControlTransformationSettings* settings = GetSettings();
            if (fabs(rectSize.dx) >= settings->minimumSelectionRectSize.dx || fabs(rectSize.dy) >= settings->minimumSelectionRectSize.dy)
            {
                return eDragState::SelectByRect;
            }
        }
    }
    return eDragState::NoDrag;
}

void HUDSystem::ClearMagnetLines()
{
    static const Vector<MagnetLineInfo> emptyVector;
    OnMagnetLinesChanged(emptyVector);
}

void HUDSystem::SyncHudWithSelection()
{
    using namespace DAVA;

    DataContext* activeContext = accessor->GetActiveContext();
    DVASSERT(activeContext != nullptr);
    DocumentData* documentData = activeContext->GetData<DocumentData>();
    SelectedControls selection = documentData->GetSelectedControls();
    hudMap.clear();
    for (ControlNode* node : selection)
    {
        hudMap[node] = std::make_unique<HUD>(node, this, hudControl.Get());
    }
}

void HUDSystem::UpdateHUDEnabled()
{
    bool enabled = GetSystemsManager()->GetDragState() != eDragState::DragScreen
    && GetSystemsManager()->GetDisplayState() == eDisplayState::Edit;
    hudControl->SetVisibilityFlag(enabled);
}

ControlTransformationSettings* HUDSystem::GetSettings()
{
    return accessor->GetGlobalContext()->GetData<ControlTransformationSettings>();
}

DAVA::ContextAccessor* HUDSystem::GetAccessor()
{
    return accessor;
}
