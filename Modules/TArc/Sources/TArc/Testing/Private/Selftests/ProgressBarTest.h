#include "TArc/Testing/TArcTestClass.h"
#include "TArc/Testing/TArcUnitTests.h"
#include "TArc/Testing/MockDefine.h"

#include "TArc/Core/ClientModule.h"
#include "TArc/Controls/ProgressBar.h"
#include "TArc/Controls/QtBoxLayouts.h"
#include "TArc/Controls/CommonStrings.h"
#include "TArc/Utils/QtConnections.h"
#include "TArc/Testing/Private/TestModuleHolder.h"

#include <Base/Any.h>
#include <Base/BaseTypes.h>
#include <Reflection/ReflectionRegistrator.h>
#include <Reflection/Reflection.h>
#include <Reflection/ReflectedMeta.h>

#include <QtTest>

namespace ProgressBarTestDetails
{
using namespace DAVA;

WindowKey wndKey = WindowKey("ProgressBarTestWindow");

class ProgressBarTestData : public ReflectionBase
{
public:
    ProgressBarTestData()
    {
        rangeMeta.reset(new M::Range(0, 10, 1));
    }

    int32 value = 5;

    int32 GetValue() const
    {
        return value;
    }

    void SetValue(int32 v)
    {
        value = v;
    }

    Any GetNoValue() const
    {
        return Any();
    }

    const M::Range* GetRangeMeta() const
    {
        return rangeMeta.get();
    }

    QString GetValueFormat() const
    {
        return "format %v";
    }

    QString GetPercentageFormat() const
    {
        return "format %p%";
    }

    DAVA_VIRTUAL_REFLECTION_IN_PLACE(ProgressBarTestData)
    {
        ReflectionRegistrator<ProgressBarTestData>::Begin()
        .Field("value", &ProgressBarTestData::GetValue, &ProgressBarTestData::SetValue)[M::Range(0, 10, 1)]
        .Field("noRangeValue", &ProgressBarTestData::GetValue, &ProgressBarTestData::SetValue)
        .Field("readOnlyValue", &ProgressBarTestData::GetValue, nullptr)
        .Field("noValue", &ProgressBarTestData::GetNoValue, nullptr)
        .Field("rangeMeta", &ProgressBarTestData::GetRangeMeta, nullptr)
        .Field("valueFormat", &ProgressBarTestData::GetValueFormat, nullptr)
        .Field("percentageFormat", &ProgressBarTestData::GetPercentageFormat, nullptr)
        .End();
    }

    std::shared_ptr<M::Range> rangeMeta;
};

class ProgressBarTestModule : public ClientModule
{
public:
    ProgressBarTestModule()
        : holder(this)
    {
    }

    void PostInit() override
    {
        QWidget* w = new QWidget();
        QtVBoxLayout* layout = new QtVBoxLayout(w);

        Reflection ref = Reflection::Create(&model);

        {
            ProgressBar::Params params(GetAccessor(), GetUI(), wndKey);
            params.fields[ProgressBar::Fields::Value] = "value";
            ProgressBar* edit = new ProgressBar(params, ref);
            edit->SetObjectName("ProgressBar_value");
            layout->AddControl(edit);
        }

        {
            ProgressBar::Params params(GetAccessor(), GetUI(), wndKey);
            params.fields[ProgressBar::Fields::Value] = "noRangeValue";
            params.fields[ProgressBar::Fields::Range] = "rangeMeta";
            ProgressBar* edit = new ProgressBar(params, ref);
            edit->SetObjectName("ProgressBar_metaRangeValue");
            layout->AddControl(edit);
        }

        {
            ProgressBar::Params params(GetAccessor(), GetUI(), wndKey);
            params.fields[ProgressBar::Fields::Value] = "readOnlyValue";
            params.fields[ProgressBar::Fields::Range] = "rangeMeta";
            ProgressBar* edit = new ProgressBar(params, ref);
            edit->SetObjectName("ProgressBar_readOnlyValue");
            layout->AddControl(edit);
        }

        {
            ProgressBar::Params params(GetAccessor(), GetUI(), wndKey);
            params.fields[ProgressBar::Fields::Value] = "noValue";
            ProgressBar* edit = new ProgressBar(params, ref);
            edit->SetObjectName("ProgressBar_noValue");
            layout->AddControl(edit);
        }

        {
            ProgressBar::Params params(GetAccessor(), GetUI(), wndKey);
            params.fields[ProgressBar::Fields::Value] = "value";
            params.fields[ProgressBar::Fields::Format] = "percentageFormat";
            ProgressBar* edit = new ProgressBar(params, ref);
            edit->SetObjectName("ProgressBar_percentageFormat");
            layout->AddControl(edit);
        }

        {
            ProgressBar::Params params(GetAccessor(), GetUI(), wndKey);
            params.fields[ProgressBar::Fields::Value] = "value";
            params.fields[ProgressBar::Fields::Format] = "valueFormat";
            ProgressBar* edit = new ProgressBar(params, ref);
            edit->SetObjectName("ProgressBar_valueFormat");
            layout->AddControl(edit);
        }

        GetUI()->AddView(wndKey, PanelKey("ProgressBarView", CentralPanelInfo()), w);
    }

    ProgressBarTestData model;

    DAVA_VIRTUAL_REFLECTION_IN_PLACE(ProgressBarTestModule, ClientModule)
    {
        ReflectionRegistrator<ProgressBarTestModule>::Begin()
        .ConstructorByPointer()
        .End();
    }

private:
    TestModuleHolder<ProgressBarTestModule> holder;
};

using Holder = TestModuleHolder<ProgressBarTestModule>;
}

DAVA_TARC_TESTCLASS(ProgressBarTests)
{
    void RangedValueChangeTest(const QString& name)
    {
        using namespace ProgressBarTestDetails;
        QProgressBar* bar = LookupSingleWidget<QProgressBar>(wndKey, name);
        ProgressBarTestModule* module = Holder::moduleInstance;
        TEST_VERIFY(bar->value() == module->model.value);
        TEST_VERIFY(bar->minimum() == 0);
        TEST_VERIFY(bar->maximum() == 10);
    }

    DAVA_TEST (RangedValueFieldTest)
    {
        RangedValueChangeTest("ProgressBar_value");
    }

    DAVA_TEST (RangedMetaValueFieldTest)
    {
        RangedValueChangeTest("ProgressBar_metaRangeValue");
    }

    DAVA_TEST (ReadOnlySetterTest)
    {
        RangedValueChangeTest("ProgressBar_readOnlyValue");
    }

    DAVA_TEST (NoValueTest)
    {
        using namespace ProgressBarTestDetails;
        QProgressBar* bar = LookupSingleWidget<QProgressBar>(wndKey, "ProgressBar_noValue");

        ProgressBarTestModule* module = Holder::moduleInstance;
        TEST_VERIFY(bar->value() == bar->minimum());
    }

    DAVA_TEST (ValueFormatTest)
    {
        using namespace ProgressBarTestDetails;
        QProgressBar* bar = LookupSingleWidget<QProgressBar>(wndKey, "ProgressBar_valueFormat");

        ProgressBarTestModule* module = Holder::moduleInstance;

        TEST_VERIFY(bar->text() == "format 5");
    }

    DAVA_TEST (PercentageFormatTest)
    {
        using namespace ProgressBarTestDetails;
        QProgressBar* bar = LookupSingleWidget<QProgressBar>(wndKey, "ProgressBar_percentageFormat");

        ProgressBarTestModule* module = Holder::moduleInstance;

        TEST_VERIFY(bar->text() == "format 50%");
    }

    void AfterSyncTest()
    {
        using namespace ProgressBarTestDetails;
        QProgressBar* valueBar = LookupSingleWidget<QProgressBar>(wndKey, "ProgressBar_value");
        QProgressBar* metaRangeBar = LookupSingleWidget<QProgressBar>(wndKey, "ProgressBar_metaRangeValue");

        TEST_VERIFY(valueBar->value() == 7);
        TEST_VERIFY(valueBar->minimum() == 0);
        TEST_VERIFY(valueBar->maximum() == 10);

        TEST_VERIFY(metaRangeBar->value() == 7);
        TEST_VERIFY(metaRangeBar->minimum() == 5);
        TEST_VERIFY(metaRangeBar->maximum() == 100);
    }

    DAVA_TEST (ValueSyncTest)
    {
        using namespace ::testing;
        using namespace ProgressBarTestDetails;
        Holder::moduleInstance->model.value = 7;
        Holder::moduleInstance->model.rangeMeta.reset(new M::Range(5, 100, 4));

        EXPECT_CALL(*this, AfterWrappersSync())
        .WillOnce(Invoke(this, &ProgressBarTests::AfterSyncTest));
    }

    MOCK_METHOD0_VIRTUAL(AfterWrappersSync, void());

    BEGIN_TESTED_MODULES()
    DECLARE_TESTED_MODULE(ProgressBarTestDetails::ProgressBarTestModule);
    END_TESTED_MODULES()
};