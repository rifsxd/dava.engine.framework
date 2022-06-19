#include "Classes/Modules/CanvasModule/EditorControlsView.h"

#include "Classes/EditorSystems/EditorSystemsManager.h"
#include "Classes/EditorSystems/MovableInEditorComponent.h"
#include "Classes/EditorSystems/CounterpoiseComponent.h"

#include "Classes/Model/PackageHierarchy/PackageBaseNode.h"
#include "Classes/Model/PackageHierarchy/ControlNode.h"
#include "Classes/Model/PackageHierarchy/PackageNode.h"
#include "Classes/Model/ControlProperties/RootProperty.h"

#include "Classes/Modules/DocumentsModule/DocumentData.h"
#include "Classes/UI/Preview/PreviewWidgetSettings.h"

#include <TArc/Core/FieldBinder.h>
#include <TArc/DataProcessing/DataListener.h>

#include <Engine/Engine.h>
#include <Engine/EngineContext.h>
#include <Render/2D/Sprite.h>
#include <Render/2D/Systems/RenderSystem2D.h>
#include <UI/UIControl.h>
#include <UI/UIControlBackground.h>
#include <UI/Layouts/UILayoutIsolationComponent.h>
#include <UI/UIControlSystem.h>
#include <UI/Layouts/UILayoutSystem.h>
#include <Reflection/ReflectionRegistrator.h>
#include <Base/BaseTypes.h>

namespace EditorControlsViewDetails
{
using namespace DAVA;

class GridControl : public UIControl, DAVA::DataListener
{
public:
    GridControl(DAVA::ContextAccessor* accessor);
    ~GridControl() override;
    void SetSize(const Vector2& size) override;

protected:
    void OnDataChanged(const DAVA::DataWrapper& wrapper, const Vector<Any>& fields) override;

private:
    void Draw(const UIGeometricData& geometricData) override;
    void UpdateColorControlBackground();

    ScopedPtr<UIControl> colorControl;
    DAVA::ContextAccessor* accessor = nullptr;
    DAVA::DataWrapper wrapper;
};

GridControl::GridControl(DAVA::ContextAccessor* accessor_)
    : colorControl(new UIControl)
    , accessor(accessor_)
{
    {
        UIControlBackground* background = GetOrCreateComponent<UIControlBackground>();
        background->SetDrawType(UIControlBackground::DRAW_TILED);
        ScopedPtr<Sprite> sprite(Sprite::CreateFromSourceFile("~res:/QuickEd/UI/GrayGrid.png"));
        background->SetSprite(sprite, 0);
        colorControl->SetName("Color_control");
    }

    {
        UIControlBackground* background = colorControl->GetOrCreateComponent<UIControlBackground>();
        background->SetDrawType(UIControlBackground::DRAW_FILL);
        UIControl::AddControl(colorControl);
    }

    UpdateColorControlBackground();
    wrapper = accessor->CreateWrapper(DAVA::ReflectedTypeDB::Get<PreviewWidgetSettings>());
    wrapper.SetListener(this);
}

GridControl::~GridControl()
{
    wrapper.SetListener(nullptr);
}

void GridControl::SetSize(const Vector2& size)
{
    colorControl->SetSize(size);
    UIControl::SetSize(size);
}

void GridControl::Draw(const UIGeometricData& geometricData)
{
    if (0.0f != geometricData.scale.x)
    {
        float32 invScale = 1.0f / geometricData.scale.x;
        UIGeometricData unscaledGd;
        unscaledGd.scale = Vector2(invScale, invScale);
        unscaledGd.size = geometricData.size * geometricData.scale.x;
        unscaledGd.AddGeometricData(geometricData);
        UIControl::Draw(unscaledGd);
    }
}

void GridControl::OnDataChanged(const DAVA::DataWrapper& wrapper, const Vector<Any>& fields)
{
    DVASSERT(wrapper == this->wrapper);
    UpdateColorControlBackground();
}

void GridControl::UpdateColorControlBackground()
{
    PreviewWidgetSettings* settings = accessor->GetGlobalContext()->GetData<PreviewWidgetSettings>();
    DAVA::Color color = settings->backgroundColors[settings->backgroundColorIndex];
    colorControl->GetComponent<UIControlBackground>()->SetColor(color);
}

void CalculateTotalRectImpl(UIControl* control, Rect& totalRect, Vector2& rootControlPosition, const UIGeometricData& gd)
{
    if (!control->GetVisibilityFlag() || control->IsHiddenForDebug())
    {
        return;
    }
    UIGeometricData tempGeometricData = control->GetLocalGeometricData();
    tempGeometricData.AddGeometricData(gd);
    Rect box = tempGeometricData.GetAABBox();

    if (totalRect.x > box.x)
    {
        float32 delta = totalRect.x - box.x;
        rootControlPosition.x += delta;
        totalRect.dx += delta;
        totalRect.x = box.x;
    }
    if (totalRect.y > box.y)
    {
        float32 delta = totalRect.y - box.y;
        rootControlPosition.y += delta;
        totalRect.dy += delta;
        totalRect.y = box.y;
    }
    if (totalRect.x + totalRect.dx < box.x + box.dx)
    {
        float32 nextRight = box.x + box.dx;
        totalRect.dx = nextRight - totalRect.x;
    }
    if (totalRect.y + totalRect.dy < box.y + box.dy)
    {
        float32 nextBottom = box.y + box.dy;
        totalRect.dy = nextBottom - totalRect.y;
    }

    for (const auto& child : control->GetChildren())
    {
        CalculateTotalRectImpl(child.Get(), totalRect, rootControlPosition, tempGeometricData);
    }
}

bool IsPropertyAffectBackground(AbstractProperty* property)
{
    DVASSERT(nullptr != property);
    FastName name(property->GetName());
    static FastName matchedNames[] = { FastName("angle"), FastName("size"), FastName("scale"), FastName("position"), FastName("pivot"), FastName("visible") };
    return std::find(std::begin(matchedNames), std::end(matchedNames), name) != std::end(matchedNames);
}
} //EditorControlsViewDetails

class BackgroundController final
{
public:
    BackgroundController(DAVA::UIControl* nestedControl, DAVA::ContextAccessor* accessor);
    ~BackgroundController();
    DAVA::UIControl* GetControl() const;
    bool IsNestedControl(const DAVA::UIControl* control) const;
    DAVA::Vector2 GetRootControlPos() const;
    DAVA::Vector2 GetWorkAreaSize() const;

    void RecalculateBackgroundProperties(DAVA::UIControl* control);
    void ControlWasRemoved(ControlNode* node, ControlsContainerNode* from);
    void ControlWasAdded(ControlNode* node, ControlsContainerNode* /*destination*/, int /*index*/);
    void UpdateCounterpoise();
    void AdjustToNestedControl();

    DAVA::Signal<> contentSizeChanged;
    DAVA::Signal<> rootControlPosChanged;

private:
    void CalculateTotalRect(DAVA::Rect& totalRect, DAVA::Vector2& rootControlPosition) const;
    void FitGridIfParentIsNested(DAVA::UIControl* control);
    //colored grid under root control
    DAVA::RefPtr<DAVA::UIControl> gridControl;
    //this one make root control looks unscaled, unrotated and unmoved
    DAVA::RefPtr<DAVA::UIControl> counterpoiseControl;
    //after all - sometimes we need to add additional position to a root control
    DAVA::RefPtr<DAVA::UIControl> positionHolderControl;

    DAVA::Vector2 workAreaSize = DAVA::Vector2(0.0f, 0.0f);
    DAVA::Vector2 rootControlPos = DAVA::Vector2(0.0f, 0.0f);
    DAVA::UIControl* nestedControl = nullptr;
};

BackgroundController::BackgroundController(DAVA::UIControl* nestedControl_, DAVA::ContextAccessor* accessor)
    : gridControl(new EditorControlsViewDetails::GridControl(accessor))
    , counterpoiseControl(new DAVA::UIControl())
    , positionHolderControl(new DAVA::UIControl())
    , nestedControl(nestedControl_)
{
    using namespace DAVA;

    DVASSERT(nullptr != nestedControl);
    String name = nestedControl->GetName().c_str();
    name = name.empty() ? "unnamed" : name;
    gridControl->SetName(Format("Grid_control_of_%s", name.c_str()));
    gridControl->GetOrCreateComponent<MovableInEditorComponent>();
    counterpoiseControl->SetName(Format("counterpoise_of_%s", name.c_str()));
    counterpoiseControl->GetOrCreateComponent<CounterpoiseComponent>();
    positionHolderControl->SetName(Format("Position_holder_of_%s", name.c_str()));
    gridControl->AddControl(positionHolderControl.Get());
    positionHolderControl->AddControl(counterpoiseControl.Get());
    counterpoiseControl->AddControl(nestedControl);
    nestedControl->GetOrCreateComponent<UILayoutIsolationComponent>();
}

BackgroundController::~BackgroundController()
{
    nestedControl->RemoveComponent<DAVA::UILayoutIsolationComponent>();
}

DAVA::UIControl* BackgroundController::GetControl() const
{
    return gridControl.Get();
}

bool BackgroundController::IsNestedControl(const DAVA::UIControl* control) const
{
    return control == nestedControl;
}

DAVA::Vector2 BackgroundController::GetRootControlPos() const
{
    return rootControlPos;
}

DAVA::Vector2 BackgroundController::GetWorkAreaSize() const
{
    return workAreaSize;
}

void BackgroundController::RecalculateBackgroundProperties(DAVA::UIControl* control)
{
    if (control == nestedControl)
    {
        UpdateCounterpoise();
    }
    FitGridIfParentIsNested(control);
}

void BackgroundController::CalculateTotalRect(DAVA::Rect& totalRect, DAVA::Vector2& rootControlPosition) const
{
    using namespace DAVA;

    rootControlPosition.SetZero();
    UIGeometricData gd = nestedControl->GetGeometricData();

    gd.position.SetZero();
    UIControl* scalableControl = gridControl->GetParent()->GetParent();
    DVASSERT(nullptr != scalableControl, "grid update without being attached to screen");
    gd.scale /= scalableControl->GetScale(); //grid->controlCanvas->scalableControl
    if (gd.scale.x != 0.0f || gd.scale.y != 0.0f)
    {
        totalRect = gd.GetAABBox();

        for (const auto& child : nestedControl->GetChildren())
        {
            EditorControlsViewDetails::CalculateTotalRectImpl(child.Get(), totalRect, rootControlPosition, gd);
        }
    }
}

void BackgroundController::AdjustToNestedControl()
{
    using namespace DAVA;

    Rect rect;
    Vector2 pos;
    CalculateTotalRect(rect, pos);
    workAreaSize = rect.GetSize();
    gridControl->SetSize(nestedControl->GetSize());

    rootControlPos = pos;
    rootControlPosChanged.Emit();
}

void BackgroundController::ControlWasRemoved(ControlNode* node, ControlsContainerNode* from)
{
    //check that we adjust removed node
    if (node->GetControl() == nestedControl)
    {
        return;
    }
    FitGridIfParentIsNested(from->GetControl());
}

void BackgroundController::ControlWasAdded(ControlNode* /*node*/, ControlsContainerNode* destination, int /*index*/)
{
    FitGridIfParentIsNested(destination->GetControl());
}

void BackgroundController::UpdateCounterpoise()
{
    using namespace DAVA;

    UIGeometricData gd = nestedControl->GetLocalGeometricData();
    Vector2 invScale;
    invScale.x = gd.scale.x != 0.0f ? 1.0f / gd.scale.x : 0.0f;
    invScale.y = gd.scale.y != 0.0f ? 1.0f / gd.scale.y : 0.0f;
    counterpoiseControl->SetScale(invScale);
    counterpoiseControl->SetSize(gd.size);
    gd.cosA = cosf(gd.angle);
    gd.sinA = sinf(gd.angle);
    counterpoiseControl->SetAngle(-gd.angle);
    Vector2 pos(gd.position * invScale);
    Vector2 angeledPosition(pos.x * gd.cosA + pos.y * gd.sinA,
                            pos.x * -gd.sinA + pos.y * gd.cosA);

    counterpoiseControl->SetPosition(-angeledPosition + gd.pivotPoint);
}

void BackgroundController::FitGridIfParentIsNested(DAVA::UIControl* control)
{
    DAVA::UIControl* parent = control;
    while (parent != nullptr)
    {
        if (parent == nestedControl) //we change child in the nested control
        {
            AdjustToNestedControl();
            contentSizeChanged.Emit();
            return;
        }
        parent = parent->GetParent();
    }
}

EditorControlsView::EditorControlsView(DAVA::UIControl* canvas, DAVA::ContextAccessor* accessor)
    : BaseEditorSystem(accessor)
    , controlsCanvas(new DAVA::UIControl())
    , packageListenerProxy(this, accessor)
{
    using namespace DAVA;

    canvas->AddControl(controlsCanvas.Get());
    controlsCanvas->SetName(FastName("controls_canvas"));

    GetEngineContext()->uiControlSystem->GetLayoutSystem()->controlLayouted.Connect(this, &EditorControlsView::OnControlLayouted);

    Engine::Instance()->beginFrame.Connect(this, &EditorControlsView::PlaceControlsOnCanvas);
    Engine::Instance()->gameLoopStopped.Connect(this, &EditorControlsView::OnGameLoopStopped);
}

EditorControlsView::~EditorControlsView()
{
}

void EditorControlsView::DeleteCanvasControls(const CanvasControls& canvasControls)
{
}

void EditorControlsView::OnDragStateChanged(eDragState /*currentState*/, eDragState previousState)
{
    if (previousState == eDragState::Transform)
    {
        needRecalculateBgrBeforeRender = true;
        OnUpdate();
    }
}

void EditorControlsView::ControlWasRemoved(ControlNode* node, ControlsContainerNode* from)
{
    if (nullptr == controlsCanvas->GetParent())
    {
        return;
    }
    for (auto& iter : gridControls)
    {
        iter->ControlWasRemoved(node, from);
    }
}

void EditorControlsView::ControlWasAdded(ControlNode* node, ControlsContainerNode* destination, int index)
{
    if (nullptr == controlsCanvas->GetParent())
    {
        return;
    }
    for (auto& iter : gridControls)
    {
        iter->ControlWasAdded(node, destination, index);
    }
}

void EditorControlsView::ControlPropertyWasChanged(ControlNode* node, AbstractProperty* property)
{
    DVASSERT(nullptr != node);
    DVASSERT(nullptr != property);

    if (nullptr == controlsCanvas->GetParent()) //detached canvas
    {
        DVASSERT(false);
        return;
    }

    if (GetSystemsManager()->GetDragState() != eDragState::Transform)
    {
        if (EditorControlsViewDetails::IsPropertyAffectBackground(property))
        {
            RecalculateBackgroundPropertiesForGrids(node->GetControl());
        }
    }
}

void EditorControlsView::OnControlLayouted(DAVA::UIControl* control)
{
    needRecalculateBgrBeforeRender = true;
}

void EditorControlsView::RecalculateBackgroundPropertiesForGrids(DAVA::UIControl* control)
{
    for (auto& iter : gridControls)
    {
        iter->RecalculateBackgroundProperties(control);
    }
}

eSystems EditorControlsView::GetOrder() const
{
    return eSystems::CONTROLS_VIEW;
}

void EditorControlsView::OnUpdate()
{
    if (needRecalculateBgrBeforeRender)
    {
        if (GetSystemsManager()->GetDragState() == eDragState::Transform)
        { // do not recalculate while control is dragged
            return;
        }

        needRecalculateBgrBeforeRender = false;

        for (auto& iter : gridControls)
        {
            iter->UpdateCounterpoise();
            iter->AdjustToNestedControl();
        }
        Layout();
    }
}

void EditorControlsView::PlaceControlsOnCanvas()
{
    static SortedControlNodeSet displayedControls;
    SortedControlNodeSet newDisplayedControls = GetDisplayedControls();
    if (displayedControls != newDisplayedControls)
    {
        OnRootContolsChanged(newDisplayedControls, displayedControls);

        //we need to retain cached nodes between frames
        //this is the only way to have no conflicts with the other systems
        for (ControlNode* node : displayedControls)
        {
            node->Release();
        }
        displayedControls = newDisplayedControls;
        for (ControlNode* node : displayedControls)
        {
            node->Retain();
        }
    }
}

BackgroundController* EditorControlsView::CreateControlBackground(PackageBaseNode* node)
{
    BackgroundController* backgroundController(new BackgroundController(node->GetControl(), accessor));
    backgroundController->contentSizeChanged.Connect(this, &EditorControlsView::Layout);
    backgroundController->rootControlPosChanged.Connect(this, &EditorControlsView::OnRootControlPosChanged);
    return backgroundController;
}

void EditorControlsView::AddBackgroundControllerToCanvas(BackgroundController* backgroundController, size_t pos)
{
    DAVA::UIControl* grid = backgroundController->GetControl();
    if (pos >= controlsCanvas->GetChildren().size())
    {
        controlsCanvas->AddControl(grid);
        gridControls.emplace_back(backgroundController);
    }
    else
    {
        auto iterToInsertControl = controlsCanvas->GetChildren().begin();
        std::advance(iterToInsertControl, pos);
        controlsCanvas->InsertChildBelow(grid, iterToInsertControl->Get());

        auto iterToInsertController = gridControls.begin();
        std::advance(iterToInsertController, pos);
        gridControls.emplace(iterToInsertController, backgroundController);
    }
    backgroundController->UpdateCounterpoise();
    backgroundController->AdjustToNestedControl();
}

DAVA::uint32 EditorControlsView::GetIndexByPos(const DAVA::Vector2& pos) const
{
    DAVA::uint32 index = 0;
    for (auto& iter : gridControls)
    {
        auto grid = iter->GetControl();

        if (pos.y < (grid->GetPosition().y + grid->GetSize().y / 2.0f))
        {
            return index;
        }
        index++;
    }
    return index;
}

void EditorControlsView::Layout()
{
    using namespace DAVA;

    float32 maxWidth = 0.0f;
    float32 totalHeight = 0.0f;
    const int spacing = 5;

    size_t childrenCount = gridControls.size();
    if (childrenCount > 1)
    {
        totalHeight += spacing * (childrenCount - 1);
    }

    //collect current geometry
    for (const std::unique_ptr<BackgroundController>& controller : gridControls)
    {
        Vector2 size = controller->GetWorkAreaSize();
        maxWidth = Max(maxWidth, size.dx);
        totalHeight += size.dy;
    }

    //place all grids in a column
    float32 curY = 0.0f;
    for (std::unique_ptr<BackgroundController>& controller : gridControls)
    {
        Vector2 size = controller->GetWorkAreaSize();
        Vector2 pos((maxWidth - size.dx) / 2.0f, curY);

        //TODO: remove this trick when package will contain only one root control
        if (childrenCount > 1)
        {
            pos += controller->GetRootControlPos();
        }
        curY += size.dy + spacing;

        controller->GetControl()->SetPosition(pos);
    }

    Vector2 workAreaSize(maxWidth, totalHeight);
    workAreaSizeChanged.Emit(workAreaSize);

    OnRootControlPosChanged();

    if (gridControls.size() == 1)
    {
        const std::unique_ptr<BackgroundController>& grid = gridControls.front();
        rootControlSizeChanged.Emit(grid->GetControl()->GetSize());
    }
    else
    {
        rootControlSizeChanged.Emit(workAreaSize);
    }
}

void EditorControlsView::OnRootContolsChanged(const SortedControlNodeSet& newRootControls, const SortedControlNodeSet& oldRootControls)
{
    using namespace DAVA;

    //set_difference requires sorted Set without custom comparator
    Set<ControlNode*> sortedNewControls(newRootControls.begin(), newRootControls.end());
    Set<ControlNode*> sortedOldControls(oldRootControls.begin(), oldRootControls.end());

    Set<ControlNode*> newNodes;
    Set<ControlNode*> deletedNodes;
    if (!oldRootControls.empty())
    {
        std::set_difference(sortedOldControls.begin(),
                            sortedOldControls.end(),
                            sortedNewControls.begin(),
                            sortedNewControls.end(),
                            std::inserter(deletedNodes, deletedNodes.end()));
    }
    if (!newRootControls.empty())
    {
        std::set_difference(sortedNewControls.begin(),
                            sortedNewControls.end(),
                            sortedOldControls.begin(),
                            sortedOldControls.end(),
                            std::inserter(newNodes, newNodes.end()));
    }

    for (auto iter = deletedNodes.begin(); iter != deletedNodes.end(); ++iter)
    {
        PackageBaseNode* node = *iter;
        UIControl* control = node->GetControl();
        auto findIt = std::find_if(gridControls.begin(), gridControls.end(), [control](std::unique_ptr<BackgroundController>& gridIter) {
            return gridIter->IsNestedControl(control);
        });
        DVASSERT(findIt != gridControls.end());
        controlsCanvas->RemoveControl(findIt->get()->GetControl());
        gridControls.erase(findIt);
    }

    for (auto iter = newRootControls.begin(); iter != newRootControls.end(); ++iter)
    {
        ControlNode* node = *iter;
        if (newNodes.find(node) == newNodes.end())
        {
            continue;
        }
        UIControl* control = node->GetControl();
        DVASSERT(std::find_if(gridControls.begin(), gridControls.end(), [control](std::unique_ptr<BackgroundController>& gridIter) {
                     return gridIter->IsNestedControl(control);
                 }) == gridControls.end());
        BackgroundController* backgroundController = CreateControlBackground(node);
        AddBackgroundControllerToCanvas(backgroundController, std::distance(newRootControls.begin(), iter));
    }
    needRecalculateBgrBeforeRender = true;
    OnUpdate();
}

//later background controls must be a part of data and rootControlPos must be simple getter
void EditorControlsView::OnRootControlPosChanged()
{
    if (gridControls.size() == 1)
    {
        const std::unique_ptr<BackgroundController>& grid = gridControls.front();
        rootControlPositionChanged.Emit(grid->GetRootControlPos());
    }
    else
    {
        //force show 0, 0 at top left corner if many root controls displayed
        rootControlPositionChanged.Emit(DAVA::Vector2(0.0f, 0.0f));
    }
}

SortedControlNodeSet EditorControlsView::GetDisplayedControls() const
{
    using namespace DAVA;

    DataContext* activeContext = accessor->GetActiveContext();
    if (nullptr == activeContext)
    {
        return SortedControlNodeSet(CompareByLCA);
    }

    DocumentData* documentData = activeContext->GetData<DocumentData>();
    return documentData->GetDisplayedRootControls();
}

void EditorControlsView::OnGameLoopStopped()
{
    using namespace DAVA;
    GetEngineContext()->uiControlSystem->GetLayoutSystem()->controlLayouted.Disconnect(this);
    Engine::Instance()->beginFrame.Disconnect(this);
}
