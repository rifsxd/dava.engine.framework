#include "Classes/Modules/DistanceLinesModule/Private/DistanceSystem.h"
#include "Classes/Modules/DistanceLinesModule/Private/DistanceLinesPreferences.h"

#include "Classes/Modules/DocumentsModule/EditorSystemsData.h"
#include "Classes/Modules/DocumentsModule/DocumentData.h"
#include "Classes/Modules/CanvasModule/CanvasData.h"
#include "Classes/Modules/UpdateViewsSystemModule/UpdateViewsSystem.h"
#include "Classes/EditorSystems/UIControlUtils.h"
#include "Classes/Painter/Painter.h"

#include <TArc/Core/ContextAccessor.h>
#include <TArc/Utils/Utils.h>

#include <Math/Vector.h>
#include <UI/UIControl.h>
#include <UI/UIEvent.h>
#include <Render/2D/FTFont.h>

namespace DistanceSystemDetails
{
DAVA::eAlign GetDirection(DAVA::Vector2::eAxis axis, const DAVA::Vector2& startPos, const DAVA::Vector2& endPos)
{
    using namespace DAVA;
    if (axis == Vector2::AXIS_X)
    {
        return startPos.x < endPos.x ? eAlign::ALIGN_RIGHT : eAlign::ALIGN_LEFT;
    }
    else
    {
        return startPos.y < endPos.y ? eAlign::ALIGN_BOTTOM : eAlign::ALIGN_TOP;
    }
}
}

DistanceSystem::DistanceSystem(DAVA::ContextAccessor* accessor)
    : BaseEditorSystem(accessor)
{
}

DistanceSystem::~DistanceSystem()
{
}

eSystems DistanceSystem::GetOrder() const
{
    return eSystems::DISTANCE_LINES;
}

bool DistanceSystem::CanDrawDistances() const
{
    using namespace DAVA;

    if (canDrawDistancesAfterInput == false)
    {
        return false;
    }

    if (IsKeyPressed(eModifierKeys::ALT) == false)
    {
        return false;
    }

    DVASSERT(getHighlight != nullptr);

    ControlNode* highlightedNode = getHighlight();
    if (highlightedNode == nullptr)
    {
        return false;
    }

    DataContext* activeContext = accessor->GetActiveContext();
    DVASSERT(activeContext != nullptr);
    DocumentData* documentData = activeContext->GetData<DocumentData>();
    Set<ControlNode*> selectedControls = documentData->GetSelectedControls();
    if (selectedControls.size() != 1)
    {
        return false;
    }

    ControlNode* selectedControl = *selectedControls.begin();
    if (selectedControl->GetControl()->IsHiddenForDebug())
    {
        return false;
    }

    PackageBaseNode* parent = selectedControl->GetParent();

    if (selectedControls.find(highlightedNode) != selectedControls.end())
    {
        return false;
    }

    return true;
}

void DistanceSystem::OnUpdate()
{
    using namespace DAVA;

    if (CanDrawDistances() == false)
    {
        return;
    }

    //prepare data
    DataContext* activeContext = accessor->GetActiveContext();
    EditorSystemsData* systemsData = activeContext->GetData<EditorSystemsData>();

    DVASSERT(getHighlight != nullptr);
    ControlNode* highlightedNode = getHighlight();
    UIControl* highlightedControl = highlightedNode->GetControl();

    DocumentData* documentData = activeContext->GetData<DocumentData>();
    Set<ControlNode*> selectedControls = documentData->GetSelectedControls();

    ControlNode* selectedNode = *selectedControls.begin();
    UIControl* selectionParent = selectedNode->GetControl()->GetParent();
    UIControl* selectedControl = selectedNode->GetControl();

    Rect selectedRect;
    Rect highlightedRect;

    Matrix3 transformMatrix;

    if (highlightedControl->GetParent() == selectedControl->GetParent())
    {
        selectedRect = selectedControl->GetLocalGeometricData().GetAABBox();
        highlightedRect = highlightedControl->GetLocalGeometricData().GetAABBox();
        highlightedControl->GetParent()->GetGeometricData().BuildTransformMatrix(transformMatrix);
    }
    else if (highlightedControl->GetParent() == selectedControl)
    {
        selectedRect = Rect(Vector2(0.0f, 0.0f), selectedControl->GetSize());
        highlightedRect = highlightedControl->GetLocalGeometricData().GetAABBox();
        selectedControl->GetGeometricData().BuildTransformMatrix(transformMatrix);
    }
    else if (selectedControl->GetParent() == highlightedControl)
    {
        selectedRect = selectedControl->GetLocalGeometricData().GetAABBox();
        highlightedRect = Rect(Vector2(0.0f, 0.0f), highlightedControl->GetSize());
        highlightedControl->GetGeometricData().BuildTransformMatrix(transformMatrix);
    }
    else
    {
        DVASSERT("selected and highlighted nodes must be child and parent or be neighbours");
    }

    DrawLines(selectedRect, highlightedRect, transformMatrix);
}

bool DistanceSystem::CanProcessInput(DAVA::UIEvent* currentInput, eInputSource /*inputSource*/) const
{
    //ignore keyboard events to not enable distances on alt+scroll combinations
    return currentInput->device != DAVA::eInputDevices::KEYBOARD;
}

void DistanceSystem::ProcessInput(DAVA::UIEvent* currentInput, eInputSource /*inputSource*/)
{
    canDrawDistancesAfterInput = (currentInput->phase == DAVA::UIEvent::Phase::MOVE);
}

void DistanceSystem::DrawLines(const DAVA::Rect& selectedRect, const DAVA::Rect& highlightedRect, const DAVA::Matrix3& transformMatrix) const
{
    using namespace DAVA;

    if (highlightedRect.RectIntersects(selectedRect))
    {
        for (int i = 0; i < Vector2::AXIS_COUNT; ++i)
        {
            Vector2::eAxis axis = static_cast<Vector2::eAxis>(i);
            Vector2::eAxis oppositeAxis = axis == Vector2::AXIS_X ? Vector2::AXIS_Y : Vector2::AXIS_X;

            Vector2 startPos1;
            Vector2 startPos2;
            Vector2 endPos1;
            Vector2 endPos2;

            float32 selectedRectMiddle = selectedRect.GetPosition()[oppositeAxis] + selectedRect.GetSize()[oppositeAxis] / 2.0f;
            startPos1[oppositeAxis] = selectedRectMiddle;
            startPos2[oppositeAxis] = selectedRectMiddle;
            endPos1[oppositeAxis] = selectedRectMiddle;
            endPos2[oppositeAxis] = selectedRectMiddle;

            startPos1[axis] = selectedRect.GetPosition()[axis];
            endPos1[axis] = highlightedRect.GetPosition()[axis];
            startPos2[axis] = selectedRect.GetPosition()[axis] + selectedRect.GetSize()[axis];
            endPos2[axis] = highlightedRect.GetPosition()[axis] + highlightedRect.GetSize()[axis];

            if (startPos1 == endPos2 || endPos1 == startPos2)
            {
                continue;
            }

            DrawSolidLine(startPos1, endPos1, transformMatrix, axis);
            DrawSolidLine(startPos2, endPos2, transformMatrix, axis);
        }
    }
    else
    {
        for (int i = 0; i < Vector2::AXIS_COUNT; ++i)
        {
            Vector2::eAxis axis = static_cast<Vector2::eAxis>(i);
            Vector2::eAxis oppositeAxis = axis == Vector2::AXIS_X ? Vector2::AXIS_Y : Vector2::AXIS_X;

            Vector2 startPos;
            Vector2 endPos;
            float32 selectedRectMiddle = selectedRect.GetPosition()[oppositeAxis] + selectedRect.GetSize()[oppositeAxis] / 2.0f;
            startPos[oppositeAxis] = selectedRectMiddle;
            endPos[oppositeAxis] = selectedRectMiddle;

            if (highlightedRect.GetPosition()[axis] + highlightedRect.GetSize()[axis] < selectedRect.GetPosition()[axis])
            { //if highlighted control at the left / top of the selected control

                startPos[axis] = selectedRect.GetPosition()[axis];
                endPos[axis] = highlightedRect.GetPosition()[axis] + highlightedRect.GetSize()[axis];
                DrawSolidLine(startPos, endPos, transformMatrix, axis);
                DrawDotLine(highlightedRect, endPos, transformMatrix, oppositeAxis);
            }
            else if (highlightedRect.GetPosition()[axis] > selectedRect.GetPosition()[axis] + selectedRect.GetSize()[axis])
            {
                startPos[axis] = selectedRect.GetPosition()[axis] + selectedRect.GetSize()[axis];
                endPos[axis] = highlightedRect.GetPosition()[axis];
                DrawSolidLine(startPos, endPos, transformMatrix, axis);
                DrawDotLine(highlightedRect, endPos, transformMatrix, oppositeAxis);
            }
            else
            {
                startPos[axis] = selectedRect.GetPosition()[axis];
                endPos[axis] = highlightedRect.GetPosition()[axis];
                DrawSolidLine(startPos, endPos, transformMatrix, axis);
                DrawDotLine(highlightedRect, endPos, transformMatrix, oppositeAxis);

                startPos[axis] = selectedRect.GetPosition()[axis] + selectedRect.GetSize()[axis];
                endPos[axis] = highlightedRect.GetPosition()[axis] + highlightedRect.GetSize()[axis];
                DrawSolidLine(startPos, endPos, transformMatrix, axis);
                DrawDotLine(highlightedRect, endPos, transformMatrix, oppositeAxis);
            }
        }
    }
}

void DistanceSystem::DrawSolidLine(DAVA::Vector2 startPos, DAVA::Vector2 endPos, const DAVA::Matrix3& transformMatrix, DAVA::Vector2::eAxis axis) const
{
    using namespace DAVA;

    const DAVA::float32 maximumDisplayedLength = 0.1f;

    float32 length = (endPos - startPos).Length();
    if (length < maximumDisplayedLength)
    {
        return;
    }

    Vector2::eAxis oppositeAxis = axis == Vector2::AXIS_X ? Vector2::AXIS_Y : Vector2::AXIS_X;

    Painting::DrawLineParams lineParams;

    DistanceSystemPreferences* preferences = accessor->GetGlobalContext()->GetData<DistanceSystemPreferences>();
    lineParams.color = preferences->linesColor;

    lineParams.startPos = startPos * transformMatrix;
    lineParams.endPos = endPos * transformMatrix;
    GetPainter()->Draw(GetOrder(), lineParams);

    //draw close line
    const float32 endLineLength = 8.0f;

    Vector2 lineVector = (startPos * transformMatrix - endPos * transformMatrix);
    lineVector.Set(lineVector.y, lineVector.x * -1.0f);

    lineVector.Normalize();

    Vector2 closeLineStart = endPos * transformMatrix + lineVector * endLineLength / 2.0f;
    Vector2 closeLineEnd = endPos * transformMatrix - lineVector * endLineLength / 2.0f;

    lineParams.startPos = closeLineStart;
    lineParams.endPos = closeLineEnd;
    GetPainter()->Draw(GetOrder(), lineParams);

    Painting::DrawTextParams textParams;
    textParams.color = preferences->textColor;

    textParams.text = Format("%.1f", length);
    textParams.margin = Vector2(5.0f, 5.0f);

    eAlign direction = DistanceSystemDetails::GetDirection(axis, startPos, endPos);

    //margin around text
    const float32 minLength = 20.0f;
    if (length > minLength)
    {
        if (axis == Vector2::AXIS_X)
        {
            textParams.direction = ALIGN_HCENTER | (direction == ALIGN_RIGHT ? ALIGN_BOTTOM : ALIGN_TOP);
        }
        else
        {
            textParams.direction = ALIGN_VCENTER | (direction == ALIGN_BOTTOM ? ALIGN_LEFT : ALIGN_RIGHT);
        }

        textParams.pos[axis] = (startPos[axis] + endPos[axis]) / 2.0f;
        textParams.pos[oppositeAxis] = endPos[oppositeAxis];
    }
    else
    {
        textParams.direction = direction | (axis == Vector2::AXIS_X ? ALIGN_VCENTER : ALIGN_HCENTER);
        textParams.pos[axis] = endPos[axis];
        textParams.pos[oppositeAxis] = endPos[oppositeAxis];
    }
    textParams.pos = textParams.pos * transformMatrix;

    GetPainter()->Draw(GetOrder(), textParams);
}

void DistanceSystem::DrawDotLine(const DAVA::Rect& rect, DAVA::Vector2 endPos, const DAVA::Matrix3& transformMatrix, DAVA::Vector2::eAxis axis) const
{
    using namespace DAVA;

    Vector2 startPos = endPos;
    if (rect.GetPosition()[axis] + rect.GetSize()[axis] < endPos[axis])
    {
        startPos[axis] = rect.GetPosition()[axis] + rect.GetSize()[axis];
    }
    else if (rect.GetPosition()[axis] > endPos[axis])
    {
        startPos[axis] = rect.GetPosition()[axis];
    }
    else
    {
        return;
    }

    Painting::DrawLineParams lineParams;

    DistanceSystemPreferences* preferences = accessor->GetGlobalContext()->GetData<DistanceSystemPreferences>();
    lineParams.color = preferences->linesColor;
    lineParams.startPos = startPos * transformMatrix;
    lineParams.endPos = endPos * transformMatrix;
    lineParams.type = Painting::DrawLineParams::DOT;

    GetPainter()->Draw(GetOrder(), lineParams);
}
