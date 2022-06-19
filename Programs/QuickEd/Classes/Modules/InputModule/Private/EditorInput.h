#pragma once

#include "Classes/EditorSystems/BaseEditorSystem.h"

class EditorInput : public BaseEditorSystem
{
public:
    EditorInput(DAVA::ContextAccessor* accessor);
    ~EditorInput() override = default;

private:
    CanvasControls CreateCanvasControls() override;
    void DeleteCanvasControls(const CanvasControls& canvasControls) override;
    eSystems GetOrder() const override;
    void OnDisplayStateChanged(eDisplayState currentState, eDisplayState previousState) override;

    DAVA::RefPtr<DAVA::UIControl> control;
};
