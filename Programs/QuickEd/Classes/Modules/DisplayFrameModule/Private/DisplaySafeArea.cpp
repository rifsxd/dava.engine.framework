#include "Modules/DisplayFrameModule/DisplaySafeArea.h"

#include "Modules/CanvasModule/CanvasDataAdapter.h"
#include "Modules/DocumentsModule/DocumentData.h"
#include "Modules/DocumentsModule/EditorSystemsData.h"

#include "Painter/Painter.h"

#include <TArc/Core/ContextAccessor.h>

#include <UI/UIControl.h>
#include <UI/UIControlBackground.h>
#include <UI/UIControlSystem.h>
#include <Reflection/ReflectedTypeDB.h>
#include <Reflection/ReflectionRegistrator.h>
#include <Logger/Logger.h>

DAVA_VIRTUAL_REFLECTION_IMPL(DisplaySafeAreaPreferences)
{
    DAVA::ReflectionRegistrator<DisplaySafeAreaPreferences>::Begin()[DAVA::M::DisplayName("Display Safe Area"), DAVA::M::SettingsSortKey(80)]
    .ConstructorByPointer()
    .Field("isVisible", &DisplaySafeAreaPreferences::isVisible)[DAVA::M::DisplayName("Is Visible")]
    .Field("isEnabled", &DisplaySafeAreaPreferences::isEnabled)[DAVA::M::DisplayName("Is Enabled")]
    .Field("linesColor", &DisplaySafeAreaPreferences::linesColor)[DAVA::M::DisplayName("Color")]
    .Field("leftInset", &DisplaySafeAreaPreferences::leftInset)[DAVA::M::DisplayName("Left Inset")]
    .Field("topInset", &DisplaySafeAreaPreferences::topInset)[DAVA::M::DisplayName("Top Inset")]
    .Field("rightInset", &DisplaySafeAreaPreferences::rightInset)[DAVA::M::DisplayName("Right Inset")]
    .Field("bottomInset", &DisplaySafeAreaPreferences::bottomInset)[DAVA::M::DisplayName("Bottom Inset")]
    .Field("leftNotch", &DisplaySafeAreaPreferences::isLeftNotch)[DAVA::M::DisplayName("Left Notch")]
    .Field("rightNotch", &DisplaySafeAreaPreferences::isRightNotch)[DAVA::M::DisplayName("Right Notch")]
    .End();
}

DisplaySafeArea::DisplaySafeArea(DAVA::ContextAccessor* accessor)
    : BaseEditorSystem(accessor)
    , canvasDataAdapter(accessor)
{
}

DisplaySafeArea::~DisplaySafeArea() = default;

eSystems DisplaySafeArea::GetOrder() const
{
    return eSystems::DISPLAY_FRAME;
}

void DisplaySafeArea::OnUpdate()
{
    using namespace std;
    using namespace DAVA;

    using namespace Painting;

    if (accessor->GetActiveContext() == nullptr)
    {
        return;
    }

    if (GetSystemsManager()->GetDisplayState() == eDisplayState::Emulation)
    {
        return;
    }

    DisplaySafeAreaPreferences* prefs = accessor->GetGlobalContext()->GetData<DisplaySafeAreaPreferences>();
    if (!prefs->isVisible || !prefs->isEnabled)
    {
        return;
    }

    DataContext* activeContext = accessor->GetActiveContext();
    DocumentData* documentData = activeContext->GetData<DocumentData>();
    SortedControlNodeSet rootControls = documentData->GetDisplayedRootControls();
    float32 scale = canvasDataAdapter.GetScale();

    Painter* painter = GetPainter();

    Rect visibleRect(Vector2(0.0f, 0.0f), canvasDataAdapter.GetViewSize());

    for (ControlNode* node : rootControls)
    {
        UIControl* rootControl = node->GetControl();
        const UIGeometricData& gd = rootControl->GetGeometricData();

        Rect rootRect(gd.position - rootControl->GetPivotPoint() * gd.scale, rootControl->GetSize());
        Rect drawRect;
        float32 x1 = canvasDataAdapter.MapFromScreenToRoot(std::floor(rootRect.x), Vector2::AXIS_X);
        float32 y1 = canvasDataAdapter.MapFromScreenToRoot(std::floor(rootRect.y), Vector2::AXIS_Y);

        float32 x2 = canvasDataAdapter.MapFromScreenToRoot(std::floor(rootRect.x + rootRect.dx * scale), Vector2::AXIS_X);
        float32 y2 = canvasDataAdapter.MapFromScreenToRoot(std::floor(rootRect.y + rootRect.dy * scale), Vector2::AXIS_Y);

        static const float32 OFFSET = 20.0f;

        if (x1 < x2 && y1 < y2)
        {
            DrawLineParams drawLineParams;
            drawLineParams.color = prefs->linesColor;

            drawLineParams.startPos = canvasDataAdapter.MapFromRootToScreen(Vector2(x1 + prefs->leftInset, y1 - OFFSET));
            drawLineParams.endPos = canvasDataAdapter.MapFromRootToScreen(Vector2(x1 + prefs->leftInset, y2 + OFFSET));
            painter->Draw(GetOrder(), drawLineParams);

            drawLineParams.startPos = canvasDataAdapter.MapFromRootToScreen(Vector2(x2 - prefs->rightInset, y1 - OFFSET));
            drawLineParams.endPos = canvasDataAdapter.MapFromRootToScreen(Vector2(x2 - prefs->rightInset, y2 + OFFSET));
            painter->Draw(GetOrder(), drawLineParams);

            drawLineParams.startPos = canvasDataAdapter.MapFromRootToScreen(Vector2(x1 - OFFSET, y1 + prefs->topInset));
            drawLineParams.endPos = canvasDataAdapter.MapFromRootToScreen(Vector2(x2 + OFFSET, y1 + prefs->topInset));
            painter->Draw(GetOrder(), drawLineParams);

            drawLineParams.startPos = canvasDataAdapter.MapFromRootToScreen(Vector2(x1 - OFFSET, y2 - prefs->bottomInset));
            drawLineParams.endPos = canvasDataAdapter.MapFromRootToScreen(Vector2(x2 + OFFSET, y2 - prefs->bottomInset));
            painter->Draw(GetOrder(), drawLineParams);
        }
    }
}
