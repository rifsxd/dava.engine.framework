#include "Classes/Modules/PixelGridModule/PixelGrid.h"

#include "Classes/Modules/CanvasModule/CanvasDataAdapter.h"
#include "Classes/Modules/DocumentsModule/DocumentData.h"
#include "Classes/Modules/DocumentsModule/EditorSystemsData.h"

#include "Classes/Painter/Painter.h"

#include <TArc/Core/ContextAccessor.h>

#include <UI/UIControl.h>
#include <UI/UIControlBackground.h>
#include <UI/UIControlSystem.h>
#include <Reflection/ReflectedTypeDB.h>
#include <Reflection/ReflectionRegistrator.h>
#include <Logger/Logger.h>

DAVA_VIRTUAL_REFLECTION_IMPL(PixelGridPreferences)
{
    DAVA::ReflectionRegistrator<PixelGridPreferences>::Begin()[DAVA::M::DisplayName("Pixel grid"), DAVA::M::SettingsSortKey(80)]
    .ConstructorByPointer()
    .Field("gridColor", &PixelGridPreferences::gridColor)[DAVA::M::DisplayName("Color")]
    .Field("isVisible", &PixelGridPreferences::isVisible)[DAVA::M::DisplayName("Is Visible")]
    .Field("scaleToDisplay", &PixelGridPreferences::scaleToDisplay)[DAVA::M::DisplayName("Display on scale"), DAVA::M::Range(100.0f, DAVA::Any(), 100.0f)]
    .End();
}

PixelGrid::PixelGrid(DAVA::ContextAccessor* accessor)
    : BaseEditorSystem(accessor)
    , canvasDataAdapter(accessor)
{
}

PixelGrid::~PixelGrid() = default;

eSystems PixelGrid::GetOrder() const
{
    return eSystems::PIXEL_GRID;
}

bool PixelGrid::CanShowGrid() const
{
    if (GetSystemsManager()->GetDisplayState() == eDisplayState::Emulation)
    {
        return false;
    }

    PixelGridPreferences* settings = accessor->GetGlobalContext()->GetData<PixelGridPreferences>();

    DAVA::float32 scaleToDisplay = settings->scaleToDisplay / 100.0f;
    if (settings->isVisible == false || scaleToDisplay < 1.0f)
    {
        return false;
    }

    if (canvasDataAdapter.GetScale() < scaleToDisplay)
    {
        return false;
    }

    return true;
}

void PixelGrid::OnUpdate()
{
    using namespace std;
    using namespace DAVA;

    using namespace Painting;

    if (CanShowGrid() == false)
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
        //TODO: replace this line of code with rootControl->GetAbsoluteRect() when it will be calculated correctly
        Rect rootRect(gd.position - rootControl->GetPivotPoint() * gd.scale, rootControl->GetSize());
        Rect drawRect;
        Vector2 topLeft(std::max(rootRect.x, visibleRect.x), std::max(rootRect.y, visibleRect.y));
        Vector2 bottomRight(std::min(rootRect.x + rootRect.dx * scale, visibleRect.x + visibleRect.dx),
                            std::min(rootRect.y + rootRect.dy * scale, visibleRect.y + visibleRect.dy));

        if (topLeft.x < bottomRight.x && topLeft.y < bottomRight.y)
        {
            Vector2 startValue = canvasDataAdapter.MapFromScreenToRoot(Vector2(std::floor(topLeft.x), std::floor(topLeft.y)));
            Vector2 lastValue = canvasDataAdapter.MapFromScreenToRoot(Vector2(std::ceil(bottomRight.x), std::ceil(bottomRight.y)));

            Vector2 startValuePosition = canvasDataAdapter.MapFromRootToScreen(startValue);
            Vector2 lastValuePosition = canvasDataAdapter.MapFromRootToScreen(lastValue);

            PixelGridPreferences* settings = accessor->GetGlobalContext()->GetData<PixelGridPreferences>();
            for (int32 i = 0; i < Vector2::AXIS_COUNT; ++i)
            {
                Vector2::eAxis axis = static_cast<Vector2::eAxis>(i);
                DAVA::Vector2::eAxis oppositeAxis = axis == Vector2::AXIS_X ? Vector2::AXIS_Y : Vector2::AXIS_X;

                for (float32 value = startValue[axis]; value <= lastValue[axis]; ++value)
                {
                    Vector2 relativeStartValue(startValue);
                    relativeStartValue[axis] = value;

                    Vector2 relativeEndValue(lastValue);
                    relativeEndValue[axis] = value;

                    DrawLineParams drawLineParams;
                    drawLineParams.color = settings->gridColor;
                    drawLineParams.startPos = canvasDataAdapter.MapFromRootToScreen(relativeStartValue);
                    drawLineParams.endPos = canvasDataAdapter.MapFromRootToScreen(relativeEndValue);

                    painter->Draw(GetOrder(), drawLineParams);
                }
            }
        }
    }
}
