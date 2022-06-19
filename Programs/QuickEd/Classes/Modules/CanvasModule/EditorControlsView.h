#pragma once

#include "EditorSystems/BaseEditorSystem.h"
#include "EditorSystems/EditorSystemsManager.h"
#include "Utils/PackageListenerProxy.h"

#include <TArc/DataProcessing/DataWrapper.h>

#include <Reflection/Reflection.h>

#include <Math/Color.h>
#include <Base/BaseTypes.h>

class EditorSystemsManager;
class PackageBaseNode;
class BackgroundController;

namespace DAVA
{
class FieldBinder;
}

class EditorControlsView final : public BaseEditorSystem, PackageListener
{
public:
    EditorControlsView(DAVA::UIControl* canvas, DAVA::ContextAccessor* accessor);
    ~EditorControlsView() override;

    DAVA::uint32 GetIndexByPos(const DAVA::Vector2& pos) const;

    DAVA::Signal<const DAVA::Vector2&> workAreaSizeChanged;
    DAVA::Signal<const DAVA::Vector2&> rootControlSizeChanged;
    DAVA::Signal<const DAVA::Vector2&> rootControlPositionChanged;

private:
    void OnDragStateChanged(eDragState currentState, eDragState previousState) override;

    void Layout();
    void OnRootContolsChanged(const SortedControlNodeSet& rootControls, const SortedControlNodeSet& oldRootControls);
    void ControlWasRemoved(ControlNode* node, ControlsContainerNode* from) override;
    void ControlWasAdded(ControlNode* node, ControlsContainerNode* destination, int index) override;
    void ControlPropertyWasChanged(ControlNode* node, AbstractProperty* property) override;
    void DeleteCanvasControls(const CanvasControls& canvasControls) override;
    eSystems GetOrder() const override;
    void OnUpdate() override;

    void PlaceControlsOnCanvas();

    void OnControlLayouted(DAVA::UIControl* control);

    void RecalculateBackgroundPropertiesForGrids(DAVA::UIControl* control);
    SortedControlNodeSet GetDisplayedControls() const;

    BackgroundController* CreateControlBackground(PackageBaseNode* node);
    void AddBackgroundControllerToCanvas(BackgroundController* backgroundController, size_t pos);

    void OnRootControlPosChanged();

    void OnGameLoopStopped();

    DAVA::RefPtr<DAVA::UIControl> controlsCanvas;
    DAVA::List<std::unique_ptr<BackgroundController>> gridControls;

    std::unique_ptr<DAVA::FieldBinder> fieldBinder;

    PackageListenerProxy packageListenerProxy;

    bool needRecalculateBgrBeforeRender = false;
    DAVA::DataWrapper canvasDataWrapper;
};
