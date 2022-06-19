#include "Modules/DataBindingInspectorModule/DataBindingInspectorModule.h"

#include "Modules/DataBindingInspectorModule/DataBindingInspectorModel.h"
#include "Application/QEGlobal.h"

#include <Engine/Engine.h>
#include <TArc/WindowSubSystem/UI.h>
#include <TArc/Utils/ModuleCollection.h>

#include "Modules/DocumentsModule/DocumentData.h"
#include "Modules/DataBindingInspectorModule/DataBindingInspectorModel.h"
#include "UI/Formula/FormulaContext.h"

#include <UI/UIControl.h>
#include <UI/UIControlSystem.h>
#include <UI/DataBinding/UIDataBindingSystem.h>
#include <UI/DataBinding/Private/UIDataModel.h>

#include <Reflection/Reflection.h>

#include <TArc/Controls/QtBoxLayouts.h>
#include <TArc/Controls/ComboBox.h>
#include <QHBoxLayout>

#include <QWidget>
#include <QTreeView>
#include <QTimer>

DAVA_VIRTUAL_REFLECTION_IMPL(DataBindingInspectorModule)
{
    DAVA::ReflectionRegistrator<DataBindingInspectorModule>::Begin()
    .ConstructorByPointer()
    .End();
}

class DataBindingInspectorData : public DAVA::TArcDataNode
{
public:
    DataBindingInspectorData()
    {
        model = new DataBindingInspectorModel(false);
    }

    DAVA::RefPtr<DAVA::UIControl> currentControl;
    DataBindingInspectorModel* model = nullptr;

    DAVA_VIRTUAL_REFLECTION_IN_PLACE(DataBindingInspectorData, DAVA::TArcDataNode)
    {
        DAVA::ReflectionRegistrator<DataBindingInspectorData>::Begin()
        .ConstructorByPointer()
        .Field("currentControl", &DataBindingInspectorData::currentControl)
        .End();
    }
};

void DataBindingInspectorModule::PostInit()
{
    using namespace DAVA;

    const char* title = "Data Binding Inspector";
    DockPanelInfo panelInfo;
    panelInfo.title = title;
    panelInfo.area = Qt::RightDockWidgetArea;
    PanelKey panelKey(title, panelInfo);

    GetAccessor()->GetGlobalContext()->CreateData(std::make_unique<DataBindingInspectorData>());

    QWidget* widget = InitUI();

    GetUI()->AddView(DAVA::mainWindowKey, panelKey, widget);

    UIDataBindingSystem* dataBindingSystem = GetAccessor()->GetEngineContext()->uiControlSystem->GetSystem<UIDataBindingSystem>();
    dataBindingSystem->onDataModelProcessed.Connect(this, &DataBindingInspectorModule::OnDataModelProcessed);
    dataBindingSystem->onValueWrittenToModel.Connect(this, &DataBindingInspectorModule::OnDataModelProcessed);
}

void DataBindingInspectorModule::OnWindowClosed(const DAVA::WindowKey& key)
{
    using namespace DAVA;
    DVASSERT(key == DAVA::mainWindowKey);

    UIDataBindingSystem* dataBindingSystem = GetAccessor()->GetEngineContext()->uiControlSystem->GetSystem<UIDataBindingSystem>();
    dataBindingSystem->onDataModelProcessed.Disconnect(this);
    dataBindingSystem->onValueWrittenToModel.Disconnect(this);
}

QWidget* DataBindingInspectorModule::InitUI()
{
    using namespace DAVA;

    ContextAccessor* accessor = GetAccessor();
    DataBindingInspectorData* data = accessor->GetGlobalContext()->GetData<DataBindingInspectorData>();
    QWidget* widget = new QWidget();
    QTreeView* treeView = new QTreeView();
    treeView->setModel(data->model);

    QtVBoxLayout* layout = new QtVBoxLayout();
    layout->setContentsMargins(0, 4, 0, 0);
    layout->setSpacing(4);
    Reflection ref = DAVA::Reflection::Create(GetAccessor()->GetGlobalContext()->GetData<DataBindingInspectorData>());
    layout->addWidget(treeView);
    widget->setLayout(layout);

    selectionFieldBinder.reset(new FieldBinder(GetAccessor()));

    FieldDescriptor fieldDescr;
    fieldDescr.type = ReflectedTypeDB::Get<DocumentData>();
    fieldDescr.fieldName = FastName(DocumentData::selectionPropertyName);
    selectionFieldBinder->BindField(fieldDescr, MakeFunction(this, &DataBindingInspectorModule::OnSelectionChanged));

    FieldDescriptor currentControlDescr(ReflectedTypeDB::Get<DataBindingInspectorData>(), FastName("currentControl"));
    selectionFieldBinder->BindField(currentControlDescr, MakeFunction(this, &DataBindingInspectorModule::OnCurrentControlChanged));

    return widget;
}

void DataBindingInspectorModule::OnDataModelProcessed(DAVA::UIControl* control, DAVA::UIComponent* component)
{
    RefreshData();
}

void DataBindingInspectorModule::OnCurrentControlChanged(const DAVA::Any& val)
{
    RefreshData();
}

void DataBindingInspectorModule::OnSelectionChanged(const DAVA::Any& selectionValue)
{
    DataBindingInspectorData* data = GetAccessor()->GetGlobalContext()->GetData<DataBindingInspectorData>();
    data->currentControl = nullptr;

    SelectedNodes selection = selectionValue.Cast<SelectedNodes>(SelectedNodes());
    for (const PackageBaseNode* node : selection)
    {
        const ControlNode* controlNode = dynamic_cast<const ControlNode*>(node);
        if (nullptr != controlNode && nullptr != controlNode->GetControl())
        {
            data->currentControl = controlNode->GetControl();
            break;
        }
    }
}

void DataBindingInspectorModule::RefreshData()
{
    DataBindingInspectorData* data = GetAccessor()->GetGlobalContext()->GetData<DataBindingInspectorData>();
    if (data->currentControl)
    {
        std::shared_ptr<DAVA::FormulaContext> context = DAVA::GetEngineContext()->uiControlSystem->GetSystem<DAVA::UIDataBindingSystem>()->GetFormulaContext(data->currentControl.Get());
        data->model->UpdateModel(context.get());
    }
    else
    {
        data->model->ClearModel();
    }
}

DECL_TARC_MODULE(DataBindingInspectorModule);
