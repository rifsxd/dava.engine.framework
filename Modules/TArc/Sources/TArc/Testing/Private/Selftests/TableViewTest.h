#include "TArc/Testing/TArcTestClass.h"
#include "TArc/Testing/TArcUnitTests.h"
#include "TArc/Testing/MockDefine.h"

#include "TArc/Core/ClientModule.h"
#include "TArc/Controls/TableView.h"
#include "TArc/Controls/QtBoxLayouts.h"
#include "TArc/Controls/CommonStrings.h"
#include "TArc/Utils/QtConnections.h"
#include "TArc/Testing/Private/TestModuleHolder.h"

#include <Base/Any.h>
#include <Base/BaseTypes.h>
#include <Reflection/ReflectionRegistrator.h>
#include <Reflection/Reflection.h>
#include <Reflection/ReflectedMeta.h>
#include <Logger/Logger.h>

#include <QScrollBar>
#include <QtTest>

namespace TableViewTestDetails
{
using namespace DAVA;

WindowKey wndKey("TableViewWindow");

class RowData : public ReflectionBase
{
public:
    RowData(int v, const String& s)
        : value(v)
        , str(s){};

    int32 value;
    String str;

    bool operator==(const RowData& other) const
    {
        return value == other.value && str == other.str;
    }

    bool operator!=(const RowData& other) const
    {
        return !this->operator==(other);
    }

    DAVA_VIRTUAL_REFLECTION_IN_PLACE(RowData)
    {
        ReflectionRegistrator<RowData>::Begin()
        .Field("value", &RowData::value)
        .Field("str", &RowData::str)
        .End();
    }
};

struct HeaderDescription : public ReflectionBase
{
    int value;
    int str;

    DAVA_VIRTUAL_REFLECTION_IN_PLACE(HeaderDescription)
    {
        ReflectionRegistrator<HeaderDescription>::Begin()
        .Field("value", &HeaderDescription::value)
        .Field("str", &HeaderDescription::str)
        .End();
    }
};

class TableData : public ReflectionBase
{
public:
    TableData()
    {
        rows.push_back(RowData(5, "abc"));
        rows.push_back(RowData(10, "qwe"));
    }

    Vector<RowData> rows;
    HeaderDescription header;
    size_t current = 0;

    DAVA::Any GetCurrentRow() const
    {
        return current;
    }

    void SetCurrentRow(const DAVA::Any& currentValue)
    {
        if (currentValue.IsEmpty())
        {
            current = static_cast<size_t>(0);
        }
        else
        {
            current = currentValue.Cast<size_t>();
        }
    }

    DAVA_VIRTUAL_REFLECTION_IN_PLACE(TableData)
    {
        ReflectionRegistrator<TableData>::Begin()
        .Field("rows", &TableData::rows)
        .Field("currentRow", &TableData::GetCurrentRow, &TableData::SetCurrentRow)
        .Field("header", &TableData::header)
        .End();
    }
};

class TableViewTestModule : public ClientModule
{
public:
    TableViewTestModule()
        : holder(this)
    {
    }

    void PostInit() override
    {
        QWidget* w = new QWidget();
        QtVBoxLayout* layout = new QtVBoxLayout(w);

        Reflection ref = Reflection::Create(&model);

        {
            TableView::Params params(GetAccessor(), GetUI(), wndKey);
            params.fields[TableView::Fields::CurrentValue] = "currentRow";
            params.fields[TableView::Fields::Header] = "header";
            params.fields[TableView::Fields::Values] = "rows";
            TableView* table = new TableView(params, GetAccessor(), ref);
            table->SetObjectName("TableView_value");
            layout->AddControl(table);
        }

        GetUI()->AddView(wndKey, PanelKey("TableView", CentralPanelInfo()), w);
    }

    TableData model;

    DAVA_VIRTUAL_REFLECTION_IN_PLACE(TableViewTestModule, ClientModule)
    {
        ReflectionRegistrator<TableViewTestModule>::Begin()
        .ConstructorByPointer()
        .End();
    }

private:
    TestModuleHolder<TableViewTestModule> holder;
};

using Holder = TestModuleHolder<TableViewTestModule>;
}

DAVA_TARC_TESTCLASS(TableViewTests)
{
    DAVA_TEST (TableModelTest)
    {
        using namespace TableViewTestDetails;
        QTableView* tableView = LookupSingleWidget<QTableView>(wndKey, "TableView_value");

        TableViewTestModule* module = Holder::moduleInstance;
        QAbstractItemModel* tableModel = tableView->model();

        TEST_VERIFY(tableModel->data(tableModel->index(0, 0)).toString() == QString::number(module->model.rows[0].value));
        TEST_VERIFY(tableModel->data(tableModel->index(1, 0)).toString() == QString::number(module->model.rows[1].value));

        TEST_VERIFY(tableModel->data(tableModel->index(0, 1)).toString() == QString::fromStdString(module->model.rows[0].str));
        TEST_VERIFY(tableModel->data(tableModel->index(1, 1)).toString() == QString::fromStdString(module->model.rows[1].str));
    }

    void AfterSyncTest()
    {
        using namespace TableViewTestDetails;
        QTableView* tableView = LookupSingleWidget<QTableView>(wndKey, "TableView_value");

        TableViewTestModule* module = Holder::moduleInstance;
        QAbstractItemModel* tableModel = tableView->model();

        TEST_VERIFY(tableModel->data(tableModel->index(0, 0)).toString() == QString("77"));
    }

    DAVA_TEST (ValueSyncTest)
    {
        using namespace ::testing;
        using namespace TableViewTestDetails;
        Holder::moduleInstance->model.rows[0].value = 77;

        EXPECT_CALL(*this, AfterWrappersSync())
        .WillOnce(Invoke(this, &TableViewTests::AfterSyncTest));
    }

    MOCK_METHOD0_VIRTUAL(AfterWrappersSync, void());

    BEGIN_TESTED_MODULES()
    DECLARE_TESTED_MODULE(TableViewTestDetails::TableViewTestModule);
    END_TESTED_MODULES()
};
