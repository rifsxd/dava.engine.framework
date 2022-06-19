#include "Classes/Modules/MouseEditingModule/Private/MouseEditingSystem.h"

#include "Classes/Modules/DocumentsModule/DocumentData.h"

#include <TArc/Core/ContextAccessor.h>
#include <TArc/DataProcessing/DataContext.h>
#include <TArc/Utils/Utils.h>

#include <UI/UIEvent.h>

MouseEditingSystem::MouseEditingSystem(DAVA::ContextAccessor* accessor)
    : BaseEditorSystem(accessor)
{
}

eDragState MouseEditingSystem::RequireNewState(DAVA::UIEvent* currentInput, eInputSource inputSource)
{
    using namespace DAVA;

    DataContext* activeContext = accessor->GetActiveContext();
    DocumentData* documentData = activeContext->GetData<DocumentData>();
    SelectedControls selectedControls = documentData->GetSelectedControls();
    bool hasRootControls = std::find_if(selectedControls.begin(), selectedControls.end(), [](ControlNode* node) {
                               return node->GetParent() != nullptr && node->GetParent()->GetControl() == nullptr;
                           }) != selectedControls.end();

    const EditorSystemsManager* systemsManager = GetSystemsManager();
    HUDAreaInfo areaInfo = GetSystemsManager()->GetCurrentHUDArea();
    if (duplicationState == READY_TO_DUPLICATE
        && IsKeyPressed(eModifierKeys::ALT)
        && hasRootControls == false
        && areaInfo.area != eArea::NO_AREA
        && currentInput->phase == UIEvent::Phase::DRAG
        && currentInput->mouseButton == eMouseButtons::LEFT
        && systemsManager->GetDragState() == eDragState::NoDrag
        && systemsManager->GetCurrentHUDArea().area == eArea::FRAME_AREA)
    {
        return eDragState::DuplicateByAlt;
    }
    return eDragState::NoDrag;
}

eSystems MouseEditingSystem::GetOrder() const
{
    return eSystems::DUPLICATE_BY_ALT;
}

void MouseEditingSystem::OnDragStateChanged(eDragState currentState, eDragState previousState)
{
    if (currentState == eDragState::DuplicateByAlt)
    {
        duplicateRequest.Emit();
        duplicationState = DUPLICATION_DONE;
    }
}

bool MouseEditingSystem::CanProcessInput(DAVA::UIEvent* currentInput, eInputSource inputSource) const
{
    return true;
}

void MouseEditingSystem::ProcessInput(DAVA::UIEvent* currentInput, eInputSource inputSource)
{
    using namespace DAVA;
    if (currentInput->device == eInputDevices::MOUSE &&
        currentInput->phase == UIEvent::Phase::ENDED &&
        currentInput->mouseButton == eMouseButtons::LEFT
        )
    {
        duplicationState = READY_TO_DUPLICATE;
    }
}
