#pragma once

#include "Classes/EditorSystems/BaseEditorSystem.h"

#include <Functional/Signal.h>

class MouseEditingSystem : public BaseEditorSystem
{
public:
    MouseEditingSystem(DAVA::ContextAccessor* accessor);
    DAVA::Signal<> duplicateRequest;

private:
    enum eDuplicationState
    {
        READY_TO_DUPLICATE,
        DUPLICATION_DONE
    };

    eDragState RequireNewState(DAVA::UIEvent* currentInput, eInputSource inputSource) override;
    eSystems GetOrder() const override;

    void OnDragStateChanged(eDragState currentState, eDragState previousState) override;

    bool CanProcessInput(DAVA::UIEvent* currentInput, eInputSource inputSource) const override;
    void ProcessInput(DAVA::UIEvent* currentInput, eInputSource inputSource) override;

    eDuplicationState duplicationState = READY_TO_DUPLICATE;
};
