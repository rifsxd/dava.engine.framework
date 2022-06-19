#include "Classes/Modules/InputModule/Private/EditorInput.h"

#include <TArc/Core/ContextAccessor.h>

#include <UI/UIControl.h>
#include <UI/UIControlSystem.h>
#include <UI/Input/UIModalInputComponent.h>
#include <UI/Input/UIInputSystem.h>
#include <Engine/Engine.h>

class InputLayerControl : public DAVA::UIControl
{
public:
    InputLayerControl(EditorSystemsManager* systemManager_)
        : DAVA::UIControl()
        , systemManager(systemManager_)
    {
        GetOrCreateComponent<DAVA::UIModalInputComponent>();
    }

    bool SystemProcessInput(DAVA::UIEvent* currentInput) override
    {
        //redirect input from the framework to the editor
        systemManager->OnInput(currentInput, eInputSource::SYSTEM);
        return false;
    }

private:
    EditorSystemsManager* systemManager = nullptr;
};

EditorInput::EditorInput(DAVA::ContextAccessor* accessor)
    : BaseEditorSystem(accessor)
{
}

CanvasControls EditorInput::CreateCanvasControls()
{
    control.Set(new InputLayerControl(GetSystemsManager()));
    control->SetName("input_layer_control");
    return { control };
}

void EditorInput::DeleteCanvasControls(const CanvasControls& canvasControls)
{
    control = nullptr;
}

eSystems EditorInput::GetOrder() const
{
    return eSystems::INPUT;
}

void EditorInput::OnDisplayStateChanged(eDisplayState currentState, eDisplayState previousState)
{
    using namespace DAVA;

    DVASSERT(currentState != previousState);
    if (currentState == eDisplayState::Emulation)
    {
        control->RemoveComponent<UIModalInputComponent>();
    }
    if (previousState == eDisplayState::Emulation)
    {
        control->GetOrCreateComponent<UIModalInputComponent>();
    }

    //apply new modal control
    GetEngineContext()->uiControlSystem->GetInputSystem()->SetCurrentScreen(GetEngineContext()->uiControlSystem->GetScreen());
}
