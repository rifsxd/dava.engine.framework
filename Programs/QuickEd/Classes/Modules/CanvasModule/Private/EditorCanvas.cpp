#include "Modules/CanvasModule/EditorCanvas.h"
#include "Modules/CanvasModule/CanvasModuleData.h"
#include "Modules/CanvasModule/CanvasData.h"

#include "UI/Preview/Data/CentralWidgetData.h"

#include <TArc/Core/ContextAccessor.h>
#include <TArc/DataProcessing/DataContext.h>

#include <Engine/Engine.h>
#include <Time/SystemTimer.h>
#include <UI/UIControlSystem.h>
#include <UI/UIEvent.h>
#include <UI/UIControl.h>

EditorCanvas::EditorCanvas(DAVA::ContextAccessor* accessor)
    : BaseEditorSystem(accessor)
    , canvasDataAdapter(accessor)
{
    using namespace DAVA;

    canvasDataAdapterWrapper = accessor->CreateWrapper([this](const DataContext*) { return Reflection::Create(&canvasDataAdapter); });
}

bool EditorCanvas::CanProcessInput(DAVA::UIEvent* currentInput, eInputSource /*inputSource*/) const
{
    using namespace DAVA;
    if ((currentInput->device & eInputDevices::CLASS_POINTER) == eInputDevices::UNKNOWN)
    {
        return false;
    }
    if (currentInput->phase == UIEvent::Phase::WHEEL || currentInput->phase == UIEvent::Phase::GESTURE)
    {
        return true;
    }
    return (GetSystemsManager()->GetDragState() != eDragState::NoDrag &&
            (currentInput->mouseButton == eMouseButtons::LEFT || currentInput->mouseButton == eMouseButtons::MIDDLE));
}

void EditorCanvas::ProcessInput(DAVA::UIEvent* currentInput, eInputSource inputSource)
{
    using namespace DAVA;

    const EditorSystemsManager* systemsManager = GetSystemsManager();

#if defined __DAVAENGINE_MACOS__
    const float32 direction = -1.0f;
#elif defined __DAVAENGINE_WINDOWS__
    const float32 direction = 1.0f;
#else
#error "unsupported platform"
#endif

    Vector2 pos = currentInput->point;
    Vector2 mouseDelta = systemsManager->GetMouseDelta();

    if (currentInput->device == eInputDevices::TOUCH_PAD)
    {
        if (currentInput->phase == UIEvent::Phase::GESTURE)
        {
            const UIEvent::Gesture& gesture = currentInput->gesture;
            Vector2 gestureDelta(gesture.dx, gesture.dy);
            if (gesture.dx != 0.0f || gesture.dy != 0.0f)
            {
                if ((currentInput->modifiers & (eModifierKeys::CONTROL | eModifierKeys::COMMAND)) != eModifierKeys::NONE)
                {
                    float32 scale = canvasDataAdapter.GetScale();

                    //magic value to convert gestureDelta to visible scale changing
                    //later we can move it to preferences
                    const float32 scaleDelta = 0.003f;

                    float32 newScale = scale * (1.0f + (scaleDelta * gestureDelta.dy * direction));

                    canvasDataAdapter.SetScale(newScale, pos);
                }
                else
                {
                    canvasDataAdapter.MoveScene(gestureDelta);
                }
            }
            else if (gesture.magnification != 0.0f)
            {
                float32 newScale = canvasDataAdapter.GetScale() + gesture.magnification;
                canvasDataAdapter.SetScale(newScale, pos);
            }
        }
    }
    else if (currentInput->device == eInputDevices::MOUSE)
    {
        eDragState dragState = systemsManager->GetDragState();
        if (currentInput->phase == UIEvent::Phase::WHEEL)
        {
            if ((currentInput->modifiers & (eModifierKeys::CONTROL | eModifierKeys::COMMAND)) != eModifierKeys::NONE)
            {
                int32 ticksCount = static_cast<int32>(currentInput->wheelDelta.y);
                float32 scale = canvasDataAdapter.GetScale();

                //magic value to convert ticsCount to visible scale changing
                //later we can move it to preferences
                const float32 scaleDelta = 0.07f;
                float32 newScale = scale * (1.0f + (scaleDelta * ticksCount * direction));
                canvasDataAdapter.SetScale(newScale, pos);
            }
            else
            {
                Vector2 additionalPos(currentInput->wheelDelta.x, currentInput->wheelDelta.y);

                additionalPos *= canvasDataAdapter.GetViewSize();
                //custom delimiter to scroll widget by little chunks of visible area
                static const float wheelDelta = 0.05f;
                canvasDataAdapter.MoveScene(additionalPos * wheelDelta);
            }
        }
        else if (dragState == eDragState::DragScreen)
        {
            canvasDataAdapter.MoveScene(mouseDelta);
        }
        else if (inputSource == eInputSource::CANVAS)
        {
            Vector2 lastPos = pos - systemsManager->GetMouseDelta();

            Vector2 delta(0.0f, 0.0f);

            DataContext* activeContext = accessor->GetActiveContext();
            DVASSERT(activeContext != nullptr);
            CanvasData* canvasData = activeContext->GetData<CanvasData>();
            Vector2 viewSize = canvasDataAdapter.GetViewSize();
            Vector2 start(0.0f, 0.0f);
            Vector2 end = viewSize;

            for (int32 i = Vector2::AXIS_X; i < Vector2::AXIS_COUNT; ++i)
            {
                Vector2::eAxis axis = static_cast<Vector2::eAxis>(i);

                if (pos[axis] < start[axis])
                {
                    delta[axis] = std::min(start[axis] - pos[axis], lastPos[axis] - pos[axis]);
                }
                else if (pos[axis] > end[axis])
                {
                    delta[axis] = -std::min(pos[axis] - end[axis], pos[axis] - lastPos[axis]);
                }
            }
            if (delta.IsZero() == false)
            {
                canvasDataAdapter.MoveScene(delta, true);
            }
        }
    }
}

CanvasControls EditorCanvas::CreateCanvasControls()
{
    CanvasModuleData* canvasModuleData = accessor->GetGlobalContext()->GetData<CanvasModuleData>();
    return { { canvasModuleData->canvas } };
}

void EditorCanvas::DeleteCanvasControls(const CanvasControls& canvasControls)
{
    CanvasModuleData* canvasModuleData = accessor->GetGlobalContext()->GetData<CanvasModuleData>();
    canvasModuleData->canvas = nullptr;
}

eSystems EditorCanvas::GetOrder() const
{
    return eSystems::CANVAS;
}

void EditorCanvas::OnUpdate()
{
    MoveSceneByUpdate();
    OnPositionChanged(canvasDataAdapterWrapper.GetFieldValue(CanvasDataAdapter::positionPropertyName));
    OnScaleChanged(canvasDataAdapterWrapper.GetFieldValue(CanvasDataAdapter::scalePropertyName));
}

eDragState EditorCanvas::RequireNewState(DAVA::UIEvent* currentInput, eInputSource /*inputSource*/)
{
    using namespace DAVA;

    Function<void(UIEvent*, bool)> setMouseButtonOnInput = [this](const UIEvent* currentInput, bool value) {
        if (currentInput->mouseButton == eMouseButtons::MIDDLE)
        {
            isMouseMidButtonPressed = value;
        }
    };
    if (currentInput->device == eInputDevices::MOUSE)
    {
        if (currentInput->phase == UIEvent::Phase::BEGAN)
        {
            setMouseButtonOnInput(currentInput, true);
        }
        else if (currentInput->phase == UIEvent::Phase::ENDED)
        {
            setMouseButtonOnInput(currentInput, false);
        }
    }
    else if (currentInput->device == eInputDevices::KEYBOARD && currentInput->key == eInputElements::KB_SPACE)
    {
        //we cant use isKeyPressed here, because DAVA update keyboard state after sending Key_Up event
        //if we will press key on dock widget and hold it -> DAVA will receive only KEY_REPEAT event without KEY_DOWN
        if (currentInput->phase == UIEvent::Phase::KEY_DOWN || currentInput->phase == UIEvent::Phase::KEY_DOWN_REPEAT)
        {
            isSpacePressed = true;
        }
        else if (currentInput->phase == UIEvent::Phase::KEY_UP)
        {
            isSpacePressed = false;
        }
    }
    bool inDragScreenState = isMouseMidButtonPressed || isSpacePressed;
    return inDragScreenState ? eDragState::DragScreen : eDragState::NoDrag;
}

void EditorCanvas::OnPositionChanged(const DAVA::Any& positionValue)
{
    using namespace DAVA;
    CanvasModuleData* canvasModuleData = accessor->GetGlobalContext()->GetData<CanvasModuleData>();
    UIControl* canvas = canvasModuleData->canvas.Get();
    //right now we scale and move the same UIControl
    //because there is no reason to have another UIControl for moving only
    if (positionValue.CanGet<Vector2>())
    {
        Vector2 position = positionValue.Get<Vector2>();
        canvas->SetPosition(position);
    }
    else
    {
        canvas->SetPosition(Vector2(0.0f, 0.0f));
    }
}

void EditorCanvas::OnScaleChanged(const DAVA::Any& scaleValue)
{
    using namespace DAVA;

    CanvasModuleData* canvasModuleData = accessor->GetGlobalContext()->GetData<CanvasModuleData>();
    UIControl* canvas = canvasModuleData->canvas.Get();
    if (scaleValue.CanGet<float32>())
    {
        float32 scale = scaleValue.Get<float32>();
        canvas->SetScale(Vector2(scale, scale));
    }
    else
    {
        canvas->SetScale(Vector2(1.0f, 1.0f));
    }
}

void EditorCanvas::MoveSceneByUpdate()
{
    using namespace DAVA;

    EditorSystemsManager* systemsManager = GetSystemsManager();
    Vector2 mousePos = systemsManager->GetLastMousePos();
    eDragState dragState = systemsManager->GetDragState();
    DataContext* activeContext = accessor->GetActiveContext();

    if (activeContext == nullptr || dragState != eDragState::Transform)
    {
        return;
    }

    Vector2 viewSize = canvasDataAdapter.GetViewSize();

    Vector2 start(0.0f, 0.0f);
    Vector2 end = viewSize;
    Vector2 mouseDelta(0.0f, 0.0f);
    float32 maxOverflow = 400.0f;
    //constant to convert mouse overflow to mouse delta
    float32 overflowToSpeedDelimiter = 10.0f;
    for (int32 i = Vector2::AXIS_X; i < Vector2::AXIS_COUNT; ++i)
    {
        Vector2::eAxis axis = static_cast<Vector2::eAxis>(i);
        if (mousePos[axis] < start[axis])
        {
            mouseDelta[axis] -= std::min(maxOverflow, start[axis] - mousePos[axis]) / overflowToSpeedDelimiter;
        }
        else if (mousePos[axis] > end[axis])
        {
            mouseDelta[axis] += std::min(maxOverflow, mousePos[axis] - end[axis]) / overflowToSpeedDelimiter;
        }
    }

    if (mouseDelta.IsZero() == false)
    {
        Vector2 positionDelta = -mouseDelta;
        Vector2 clampedPositionDelta = positionDelta;
        mouseDelta = -clampedPositionDelta;

        UIEvent event;
        event.phase = UIEvent::Phase::DRAG;
        event.device = eInputDevices::MOUSE;
        event.mouseButton = eMouseButtons::LEFT;
        event.point = mousePos + mouseDelta;
        systemsManager->OnInput(&event, eInputSource::CANVAS);
    }
}
