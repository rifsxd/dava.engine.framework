#include "TArc/Testing/TArcTestClass.h"
#include "TArc/Testing/TArcUnitTests.h"
#include "TArc/Testing/MockDefine.h"

#include "TArc/Controls/CommonStrings.h"
#include "TArc/Controls/QtBoxLayouts.h"
#include "TArc/Controls/Slider.h"
#include "TArc/Core/ClientModule.h"
#include "TArc/Testing/Private/TestModuleHolder.h"
#include "TArc/Utils/QtConnections.h"

#include <Base/Any.h>
#include <Base/BaseTypes.h>
#include <Reflection/ReflectedMeta.h>
#include <Reflection/Reflection.h>
#include <Reflection/ReflectionRegistrator.h>

#include <QSlider>
#include <QtTest>
#include <QStyleOption>

namespace SliderTestDetails
{
using namespace DAVA;

WindowKey wndKey = DAVA::mainWindowKey;

class SliderData : public ReflectionBase
{
public:
    SliderData()
    {
        range = new DAVA::M::Range(-10, 10, 1);
    }

    ~SliderData()
    {
        delete range;
    }

    int32 value = 3;
    bool isEnabled = true;
    const DAVA::M::Range* range = nullptr;

    int32 GetValue() const
    {
        return value;
    }

    void SetValue(int32 value_)
    {
        value = value_;
    }

    DAVA_VIRTUAL_REFLECTION_IN_PLACE(SliderData)
    {
        ReflectionRegistrator<SliderData>::Begin()
        .Field("value", &SliderData::GetValue, &SliderData::SetValue)
        .Field("enabled", &SliderData::isEnabled)
        .End();
    }
};

class SliderTestModule : public ClientModule
{
public:
    SliderTestModule()
        : holder(this)
    {
    }

    void PostInit() override
    {
        QWidget* w = new QWidget();
        QtVBoxLayout* layout = new QtVBoxLayout(w);

        Reflection ref = Reflection::Create(&model);

        {
            Slider::Params params(GetAccessor(), GetUI(), wndKey);
            params.fields[Slider::Fields::Value] = "value";
            params.fields[Slider::Fields::Enabled] = "enabled";
            params.fields[Slider::Fields::Range].BindConstValue(model.range);
            params.fields[Slider::Fields::Orientation].BindConstValue(Qt::Horizontal);
            Slider* edit = new Slider(params, ref);
            edit->ForceUpdate();
            edit->SetObjectName("Slider");
            layout->AddControl(edit);
        }

        GetUI()->AddView(wndKey, PanelKey("SliderView", CentralPanelInfo()), w);
    }

    SliderData model;

    DAVA_VIRTUAL_REFLECTION_IN_PLACE(SliderTestModule, ClientModule)
    {
        ReflectionRegistrator<SliderTestModule>::Begin()
        .ConstructorByPointer()
        .End();
    }

private:
    TestModuleHolder<SliderTestModule> holder;
};

using Holder = TestModuleHolder<SliderTestModule>;
}

DAVA_TARC_TESTCLASS(SliderTests)
{
    void AfterSyncTest()
    {
        using namespace SliderTestDetails;
        QSlider* slider = LookupSingleWidget<QSlider>(wndKey, "Slider");

        SliderTestModule* module = Holder::moduleInstance;
        TEST_VERIFY(slider->value() == 7);
    }

    DAVA_TEST (ValueSyncTest)
    {
        using namespace ::testing;
        using namespace SliderTestDetails;
        Holder::moduleInstance->model.value = 7;

        EXPECT_CALL(*this, AfterWrappersSync())
        .WillOnce(Invoke(this, &SliderTests::AfterSyncTest));
    }

    MOCK_METHOD0_VIRTUAL(AfterWrappersSync, void());

    BEGIN_TESTED_MODULES()
    DECLARE_TESTED_MODULE(SliderTestDetails::SliderTestModule);
    END_TESTED_MODULES()
};
