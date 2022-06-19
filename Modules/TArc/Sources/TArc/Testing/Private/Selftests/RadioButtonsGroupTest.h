#include "TArc/Testing/TArcTestClass.h"
#include "TArc/Testing/TArcUnitTests.h"
#include "TArc/Testing/MockDefine.h"

#include "TArc/Core/ClientModule.h"
#include "TArc/Controls/RadioButtonsGroup.h"
#include "TArc/Controls/QtBoxLayouts.h"
#include "TArc/Utils/QtConnections.h"

#include <Base/Any.h>
#include <Base/BaseTypes.h>
#include <Base/FastName.h>
#include <Base/GlobalEnum.h>
#include <Reflection/ReflectedMeta.h>
#include <Reflection/ReflectionRegistrator.h>

#include <QtTest>
#include <QAbstractButton>
#include <QWidget>

namespace DAVA
{
namespace RadioButtonsGroupTestDetails
{
WindowKey wndKey = WindowKey("RadioButtonsGroupWnd");

class RadioButtonsGroupTestModule : public ClientModule
{
public:
    enum eTest
    {
        FIRST = 0,
        SECOND,
        THIRD
    };

    struct TestModel : public ReflectionBase
    {
        //int
        int testValue = eTest::SECOND;
        int GetValue() const
        {
            return testValue;
        }
        void SetValue(int newValue)
        {
            testValue = newValue;
        }

        size_t GetValueSize_t() const
        {
            return testValue;
        }
        void SetValueSize_t(size_t newValue)
        {
            testValue = static_cast<int>(newValue);
        }

        Map<int, String> enumeratorMap = Map<int, String>{
            { FIRST, "FIRST" },
            { SECOND, "SECOND" },
            { THIRD, "THIRD" }
        };

        Vector<String> enumeratorVector = Vector<String>{ "FIRST", "SECOND", "THIRD" };

        const Map<int, QString>& GetEnumeratorMap() const
        {
            static Map<int, QString> iconEnumerator = Map<int, QString>
            {
              { FIRST, { "FIRST" } },
              { SECOND, { "SECOND" } },
              { THIRD, { "THIRD" } }
            };

            return iconEnumerator;
        }

        const Set<String>& GetEnumeratorSet() const
        {
            static Set<String> enumeratorSet = Set<String>{ "1_FIRST", "2_SECOND", "3_THIRD" };
            return enumeratorSet;
        }

        const String GetStringValue() const
        {
            const Set<String>& setEnumerator = GetEnumeratorSet();
            auto it = setEnumerator.begin();

            int i = 0;
            for (int i = 0; i < static_cast<int>(setEnumerator.size()); ++i, ++it)
            {
                if (i == testValue)
                {
                    return *it;
                }
            }

            return String();
        }

        void SetStringValue(const String& str)
        {
            const Set<String>& setEnumerator = GetEnumeratorSet();
            auto it = setEnumerator.begin();

            int i = 0;
            for (int i = 0; i < static_cast<int>(setEnumerator.size()); ++i, ++it)
            {
                if (str == *it)
                {
                    testValue = i;
                    break;
                }
            }
        }

        bool enabled = false;
        Qt::Orientation orientation = Qt::Vertical;

        DAVA_VIRTUAL_REFLECTION_IN_PLACE(TestModel, ReflectionBase)
        {
            ReflectionRegistrator<TestModel>::Begin()
            .Field("enabled", &TestModel::enabled)
            .Field("value", &TestModel::testValue)
            .Field("valueSize_t", &TestModel::GetValueSize_t, &TestModel::SetValueSize_t)
            .Field("valueString", &TestModel::GetStringValue, &TestModel::SetStringValue)
            .Field("method", &TestModel::GetValue, &TestModel::SetValue)

            .Field("enumeratorValueMap", &TestModel::enumeratorMap)
            .Field("enumeratorValueVector", &TestModel::enumeratorVector)
            .Field("enumeratorMethod", &TestModel::GetEnumeratorMap, nullptr)
            .Field("enumeratorMethodSet", &TestModel::GetEnumeratorSet, nullptr)

            .Field("valueMetaReadOnly", &TestModel::testValue)[M::EnumT<eTest>(), M::ReadOnly()]
            .Field("methodMetaOnlyGetter", &TestModel::GetValue, nullptr)[M::EnumT<eTest>()]
            .Field("valueMeta", &TestModel::testValue)[M::EnumT<eTest>()]
            .Field("methodMeta", &TestModel::GetValue, &TestModel::SetValue)[M::EnumT<eTest>()]
            .Field("orientation", &TestModel::orientation)
            .End();
        }
    };

    TestModel model;

    RadioButtonsGroupTestModule()
    {
        instance = this;
    }

    void PostInit() override
    {
        Reflection reflectedModel = Reflection::Create(&model);

        QWidget* w = new QWidget();
        QtVBoxLayout* layout = new QtVBoxLayout(w);

        {
            RadioButtonsGroup::Params params(GetAccessor(), GetUI(), wndKey);
            params.fields[RadioButtonsGroup::Fields::Value] = "valueMeta";
            params.fields[RadioButtonsGroup::Fields::Enabled] = "enabled";
            RadioButtonsGroup* widget = new RadioButtonsGroup(params, reflectedModel);
            widget->SetObjectName("MetaEnabled");
            layout->AddControl(widget);
        }

        {
            RadioButtonsGroup::Params params(GetAccessor(), GetUI(), wndKey);
            params.fields[RadioButtonsGroup::Fields::Value] = "valueMetaReadOnly";
            RadioButtonsGroup* widget = new RadioButtonsGroup(params, reflectedModel);
            widget->SetObjectName("MetaReadOnly");
            layout->AddControl(widget);
        }

        {
            RadioButtonsGroup::Params params(GetAccessor(), GetUI(), wndKey);
            params.fields[RadioButtonsGroup::Fields::Value] = "methodMetaOnlyGetter";
            RadioButtonsGroup* widget = new RadioButtonsGroup(params, reflectedModel);
            widget->SetObjectName("OnlyGetter");
            layout->AddControl(widget);
        }

        {
            RadioButtonsGroup::Params params(GetAccessor(), GetUI(), wndKey);
            params.fields[RadioButtonsGroup::Fields::Value] = "methodMeta";
            RadioButtonsGroup* widget = new RadioButtonsGroup(params, reflectedModel);
            widget->SetObjectName("MethodMeta");
            layout->AddControl(widget);
        }

        {
            RadioButtonsGroup::Params params(GetAccessor(), GetUI(), wndKey);
            params.fields[RadioButtonsGroup::Fields::Value] = "value";
            params.fields[RadioButtonsGroup::Fields::Enumerator] = "enumeratorValueMap";
            RadioButtonsGroup* widget = new RadioButtonsGroup(params, reflectedModel);
            widget->SetObjectName("EnumeratorMap");
            layout->AddControl(widget);
        }

        {
            RadioButtonsGroup::Params params(GetAccessor(), GetUI(), wndKey);
            params.fields[RadioButtonsGroup::Fields::Value] = "value";
            params.fields[RadioButtonsGroup::Fields::Enumerator] = "enumeratorMethod";
            RadioButtonsGroup* widget = new RadioButtonsGroup(params, reflectedModel);
            widget->SetObjectName("EnumeratorMethod");
            layout->AddControl(widget);
        }

        {
            RadioButtonsGroup::Params params(GetAccessor(), GetUI(), wndKey);
            params.fields[RadioButtonsGroup::Fields::Value] = "valueString";
            params.fields[RadioButtonsGroup::Fields::Enumerator] = "enumeratorMethodSet";
            RadioButtonsGroup* widget = new RadioButtonsGroup(params, reflectedModel);
            widget->SetObjectName("EnumeratorMethodSet");
            layout->AddControl(widget);
        }

        {
            RadioButtonsGroup::Params params(GetAccessor(), GetUI(), wndKey);
            params.fields[RadioButtonsGroup::Fields::Value] = "valueString";
            params.fields[RadioButtonsGroup::Fields::Enumerator] = "enumeratorMethodSet";
            params.fields[RadioButtonsGroup::Fields::Orientation] = "orientation";
            RadioButtonsGroup* widget = new RadioButtonsGroup(params, reflectedModel);
            widget->SetObjectName("EnumeratorMethodSetVertical");
            layout->AddControl(widget);
        }

        PanelKey panelKey("RadioButtonsTest", CentralPanelInfo());
        GetUI()->AddView(wndKey, panelKey, w);
    }

    static RadioButtonsGroupTestModule* instance;

    DAVA_VIRTUAL_REFLECTION_IN_PLACE(RadioButtonsGroupTestModule, ClientModule)
    {
        ReflectionRegistrator<RadioButtonsGroupTestModule>::Begin()
        .ConstructorByPointer()
        .End();
    }
};

RadioButtonsGroupTestModule* RadioButtonsGroupTestModule::instance = nullptr;
}

DAVA_TARC_TESTCLASS(RadioButtonsGroupTest)
{
    Vector<std::pair<QString, Vector<int>>> data;

    QWidget* GetWidget(const QString& name)
    {
        using namespace RadioButtonsGroupTestDetails;

        QList<QWidget*> widgets = LookupWidget(wndKey, name);
        TEST_VERIFY(widgets.size() == 1);
        QWidget* widget = widgets.front();

        return widget;
    }

    int GetSelectedItemIndex(QWidget * widget)
    {
        QList<QAbstractButton*> children = widget->findChildren<QAbstractButton*>();
        for (int i = 0, count = children.size(); i < count; ++i)
        {
            if (children.at(i)->isChecked())
            {
                return i;
            }
        }
        return -1;
    }

    DAVA_TEST (DisplayTest)
    {
        auto testFn = [this](const QString& name) {
            QWidget* widget = GetWidget(name);
            TEST_VERIFY(widget != nullptr);
            int index = GetSelectedItemIndex(widget);
            TEST_VERIFY(index == RadioButtonsGroupTestDetails::RadioButtonsGroupTestModule::instance->model.testValue);
        };

        testFn("MetaEnabled");
        testFn("MetaReadOnly");
        testFn("OnlyGetter");
        testFn("MethodMeta");
        testFn("EnumeratorMap");
        testFn("EnumeratorMethod");
        testFn("EnumeratorMethodSet");
        testFn("EnumeratorMethodSetVertical");
    }

    MOCK_METHOD0_VIRTUAL(AfterWrappersSync, void());

    BEGIN_TESTED_MODULES()
    DECLARE_TESTED_MODULE(RadioButtonsGroupTestDetails::RadioButtonsGroupTestModule);
    END_TESTED_MODULES()

    QtConnections connections;
};
} //namespace DAVA

ENUM_DECLARE(DAVA::RadioButtonsGroupTestDetails::RadioButtonsGroupTestModule::eTest)
{
    ENUM_ADD_DESCR(static_cast<int>(DAVA::RadioButtonsGroupTestDetails::RadioButtonsGroupTestModule::eTest::FIRST), "1st");
    ENUM_ADD_DESCR(static_cast<int>(DAVA::RadioButtonsGroupTestDetails::RadioButtonsGroupTestModule::eTest::SECOND), "2nd");
    ENUM_ADD_DESCR(static_cast<int>(DAVA::RadioButtonsGroupTestDetails::RadioButtonsGroupTestModule::eTest::THIRD), "3rd");
}