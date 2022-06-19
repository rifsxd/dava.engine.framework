#include "TArc/Testing/TArcTestClass.h"
#include "TArc/Testing/TArcUnitTests.h"
#include "TArc/Testing/MockDefine.h"

#include "TArc/Core/ClientModule.h"
#include "TArc/Controls/ScrollBar.h"
#include "TArc/Controls/QtBoxLayouts.h"
#include "TArc/Controls/CommonStrings.h"
#include "TArc/Utils/QtConnections.h"
#include "TArc/Testing/Private/TestModuleHolder.h"

#include <Base/Any.h>
#include <Base/BaseTypes.h>
#include <Reflection/ReflectionRegistrator.h>
#include <Reflection/Reflection.h>
#include <Reflection/ReflectedMeta.h>

#include <QScrollBar>
#include <QtTest>

namespace ScrollBarTestDetails
{
using namespace DAVA;
using namespace DAVA;

WindowKey wndKey = WindowKey("ScrollBarWindow");

class ScrollBarData : public ReflectionBase
{
public:
    ScrollBarData()
    {
    }

    int32 minimum = 1;
    int32 maximum = 10;
    int32 value = 3;
    int32 pageStep = 2;
    bool isEnabled = true;
    Qt::Orientation orientation = Qt::Horizontal;

    int32 GetValue() const
    {
        return value;
    }

    void SetValue(int32 value_)
    {
        value = value_;
    }

    DAVA_VIRTUAL_REFLECTION_IN_PLACE(ScrollBarData)
    {
        ReflectionRegistrator<ScrollBarData>::Begin()
        .Field("value", &ScrollBarData::GetValue, &ScrollBarData::SetValue)
        .Field("minimum", &ScrollBarData::minimum)
        .Field("maximum", &ScrollBarData::maximum)
        .Field("pageStep", &ScrollBarData::pageStep)
        .Field("isEnabled", &ScrollBarData::isEnabled)
        .Field("orientation", &ScrollBarData::orientation)
        .End();
    }
};

class ScrollBarTestModule : public ClientModule
{
public:
    ScrollBarTestModule()
        : holder(this)
    {
    }

    void PostInit() override
    {
        QWidget* w = new QWidget();
        QtVBoxLayout* layout = new QtVBoxLayout(w);

        Reflection ref = Reflection::Create(&model);

        {
            ScrollBar::Params params(GetAccessor(), GetUI(), wndKey);
            params.fields[ScrollBar::Fields::Value] = "value";
            params.fields[ScrollBar::Fields::Minimum] = "minimum";
            params.fields[ScrollBar::Fields::Maximum] = "maximum";
            params.fields[ScrollBar::Fields::PageStep] = "pageStep";
            params.fields[ScrollBar::Fields::Orientation] = "orientation";
            ScrollBar* edit = new ScrollBar(params, GetAccessor(), ref);
            edit->SetObjectName("ScrollBar_value");
            layout->AddControl(edit);
        }

        {
            ScrollBar::Params params(GetAccessor(), GetUI(), wndKey);
            params.fields[ScrollBar::Fields::Value] = "value";
            params.fields[ScrollBar::Fields::Enabled] = "isEnabled";
            params.fields[ScrollBar::Fields::Orientation] = "orientation";

            ScrollBar* edit = new ScrollBar(params, GetAccessor(), ref);
            edit->SetObjectName("ScrollBar_enable");
            layout->AddControl(edit);
        }

        GetUI()->AddView(wndKey, PanelKey("ScrollBarView", CentralPanelInfo()), w);
    }

    ScrollBarData model;

    DAVA_VIRTUAL_REFLECTION_IN_PLACE(ScrollBarTestModule, ClientModule)
    {
        ReflectionRegistrator<ScrollBarTestModule>::Begin()
        .ConstructorByPointer()
        .End();
    }

private:
    TestModuleHolder<ScrollBarTestModule> holder;
};

using Holder = TestModuleHolder<ScrollBarTestModule>;
}

DAVA_TARC_TESTCLASS(ScrollBarTests)
{
    void SetFocus(QScrollBar * scrollBar)
    {
        scrollBar->clearFocus();
        if (scrollBar->isActiveWindow() == false)
        {
            QWidget* topLevel = scrollBar;
            while (topLevel->parent() != nullptr)
                topLevel = topLevel->parentWidget();

            qApp->setActiveWindow(topLevel);
        }
        scrollBar->setFocus(Qt::MouseFocusReason);
    }

    void ButtonClick(QTestEventList & list, bool isUp, QScrollBar* scrollBar)
    {
        QStyleOptionSlider opt;
        opt.initFrom(scrollBar);

        QStyle* style = scrollBar->style();
        QRect r = style->subControlRect(QStyle::CC_ScrollBar, &opt, (isUp == true) ? QStyle::SC_ScrollBarSubLine : QStyle::SC_ScrollBarAddLine, scrollBar);
        r.adjust(1, 1, -1, -1);

        QPoint pt = (isUp == true) ? r.topLeft() : r.bottomRight();
        list.addMouseClick(Qt::LeftButton, Qt::KeyboardModifier(), pt);
    }

    DAVA_TEST (RangedValueFieldTest)
    {
        using namespace ScrollBarTestDetails;
        QScrollBar* scrollBar = LookupSingleWidget<QScrollBar>(wndKey, "ScrollBar_value");

        ScrollBarTestModule* module = Holder::moduleInstance;
        TEST_VERIFY(scrollBar->value() == module->model.value);
        TEST_VERIFY(scrollBar->minimum() == module->model.minimum);
        TEST_VERIFY(scrollBar->maximum() == module->model.maximum);
        TEST_VERIFY(scrollBar->pageStep() == module->model.pageStep);

        // simulate scrollBar value lower than minimum
        QTestEventList events;
        SetFocus(scrollBar);
        for (int i = 0; i < 4; ++i) // 3 (current value) - 4 (single step value) == 1
            ButtonClick(events, true, scrollBar);
        events.simulate(scrollBar);

        TEST_VERIFY(scrollBar->value() == scrollBar->minimum());
        TEST_VERIFY(scrollBar->value() == module->model.value);
        TEST_VERIFY(scrollBar->value() == module->model.minimum);

        events.clear();
        ButtonClick(events, false, scrollBar);
        events.simulate(scrollBar);
        TEST_VERIFY(scrollBar->value() == module->model.minimum + 1);
        TEST_VERIFY(scrollBar->value() == module->model.value);

        ScrollBarTestDetails::Holder::moduleInstance->model.isEnabled = false;
    }

    void ReadOnlyTest(const QString& name)
    {
        using namespace ScrollBarTestDetails;
        ScrollBarTestModule* module = Holder::moduleInstance;
        int32 readOnlyValue = module->model.value;

        QScrollBar* scrollBar = LookupSingleWidget<QScrollBar>(wndKey, name);

        QTestEventList events;
        SetFocus(scrollBar);
        ButtonClick(events, true, scrollBar);
        events.simulate(scrollBar);
        TEST_VERIFY(scrollBar->value() == readOnlyValue);
        TEST_VERIFY(scrollBar->value() == module->model.value);

        events.clear();
        SetFocus(scrollBar);
        ButtonClick(events, false, scrollBar);
        events.simulate(scrollBar);
        TEST_VERIFY(scrollBar->value() == readOnlyValue);
        TEST_VERIFY(scrollBar->value() == module->model.value);
    }

    DAVA_TEST (ReadOnlyTest)
    {
        ReadOnlyTest("ScrollBar_enable");
        ScrollBarTestDetails::Holder::moduleInstance->model.isEnabled = true;
    }

    void AfterSyncTest()
    {
        using namespace ScrollBarTestDetails;
        QScrollBar* valueScrollBar = LookupSingleWidget<QScrollBar>(wndKey, "ScrollBar_value");

        ScrollBarTestModule* module = Holder::moduleInstance;
        TEST_VERIFY(valueScrollBar->value() == 7);
        TEST_VERIFY(valueScrollBar->minimum() == module->model.minimum);
        TEST_VERIFY(valueScrollBar->maximum() == module->model.maximum);
    }

    DAVA_TEST (ValueSyncTest)
    {
        using namespace ::testing;
        using namespace ScrollBarTestDetails;
        Holder::moduleInstance->model.value = 7;

        EXPECT_CALL(*this, AfterWrappersSync())
        .WillOnce(Invoke(this, &ScrollBarTests::AfterSyncTest));
    }

    MOCK_METHOD0_VIRTUAL(AfterWrappersSync, void());

    BEGIN_TESTED_MODULES()
    DECLARE_TESTED_MODULE(ScrollBarTestDetails::ScrollBarTestModule);
    END_TESTED_MODULES()
};
