#include "Modules/FindResultsModule/FindResultsModule.h"
#include "Application/QEGlobal.h"
#include "Modules/DocumentsModule/DocumentData.h"
#include "Modules/FileSystemCacheModule/FileSystemCacheData.h"
#include "Modules/ProjectModule/ProjectData.h"
#include "UI/Find/Widgets/FindResultsWidget.h"

#include <TArc/WindowSubSystem/UI.h>
#include <TArc/Utils/ModuleCollection.h>

#include <Reflection/Reflection.h>
#include <Reflection/ReflectionRegistrator.h>
#include <Base/FastName.h>

namespace FindResultsModuleDetail
{
class DockPanelData : public DAVA::TArcDataNode
{
public:
    static const DAVA::FastName isActiveProperty;
    static const DAVA::FastName titleProperty;

    QString title = "Find Results";
    bool isActive = false;

private:
    DAVA_VIRTUAL_REFLECTION_IN_PLACE(DockPanelData, DAVA::TArcDataNode)
    {
        DAVA::ReflectionRegistrator<DockPanelData>::Begin()
        .Field(isActiveProperty.c_str(), &DockPanelData::isActive)
        .Field(titleProperty.c_str(), &DockPanelData::title)
        .End();
    }
};

const DAVA::FastName DockPanelData::isActiveProperty = DAVA::FastName("isActive");
const DAVA::FastName DockPanelData::titleProperty = DAVA::FastName("title");
}

DAVA_VIRTUAL_REFLECTION_IMPL(FindResultsModule)
{
    DAVA::ReflectionRegistrator<FindResultsModule>::Begin()
    .ConstructorByPointer()
    .End();
}

void FindResultsModule::PostInit()
{
    using namespace DAVA;

    GetAccessor()->GetGlobalContext()->CreateData(std::make_unique<FindResultsModuleDetail::DockPanelData>());

    findResultsWidget = new FindResultsWidget();

    const char* viewName = "Find Results";
    DockPanelInfo panelInfo;
    panelInfo.area = Qt::BottomDockWidgetArea;
    {
        FieldDescriptor& descr = panelInfo.descriptors[DockPanelInfo::Fields::IsActive];
        descr.fieldName = FindResultsModuleDetail::DockPanelData::isActiveProperty;
        descr.type = ReflectedTypeDB::Get<FindResultsModuleDetail::DockPanelData>();
    }
    {
        FieldDescriptor& descr = panelInfo.descriptors[DockPanelInfo::Fields::Title];
        descr.fieldName = FindResultsModuleDetail::DockPanelData::titleProperty;
        descr.type = ReflectedTypeDB::Get<FindResultsModuleDetail::DockPanelData>();
    }
    PanelKey panelKey(viewName, panelInfo);
    GetUI()->AddView(DAVA::mainWindowKey, panelKey, findResultsWidget);

    connections.AddConnection(findResultsWidget, &FindResultsWidget::JumpToControl, MakeFunction(this, &FindResultsModule::JumpToControl));
    connections.AddConnection(findResultsWidget, &FindResultsWidget::JumpToPackage, MakeFunction(this, &FindResultsModule::JumpToPackage));

    RegisterOperations();

    ContextAccessor* accessor = GetAccessor();
    projectDataWrapper = accessor->CreateWrapper(ReflectedTypeDB::Get<ProjectData>());
    projectDataWrapper.SetListener(this);
}

void FindResultsModule::JumpToControl(const DAVA::FilePath& packagePath, const DAVA::String& controlName)
{
    using namespace DAVA;

    const QString& path = QString::fromStdString(packagePath.GetAbsolutePathname());
    const QString& name = QString::fromStdString(controlName);
    InvokeOperation(QEGlobal::SelectControl.ID, path, name);
}

void FindResultsModule::JumpToPackage(const DAVA::FilePath& packagePath)
{
    const QString& path = QString::fromStdString(packagePath.GetAbsolutePathname());
    InvokeOperation(QEGlobal::OpenDocumentByPath.ID, path);
}

void FindResultsModule::RegisterOperations()
{
    RegisterOperation(QEGlobal::FindInProject.ID, this, &FindResultsModule::FindInProject);
    RegisterOperation(QEGlobal::FindInDocument.ID, this, &FindResultsModule::FindInDocument);
}

void FindResultsModule::FindInProject(std::shared_ptr<FindFilter> filter)
{
    using namespace DAVA;

    ContextAccessor* accessor = GetAccessor();
    DataContext* globalContext = accessor->GetGlobalContext();

    ProjectData* projectData = globalContext->GetData<ProjectData>();
    FileSystemCacheData* fileSystemCacheData = globalContext->GetData<FileSystemCacheData>();

    if (projectData != nullptr)
    {
        findResultsWidget->Find(filter, projectData, fileSystemCacheData->GetFiles("yaml"));
        GetAccessor()->GetGlobalContext()->GetData<FindResultsModuleDetail::DockPanelData>()->isActive = true;
    }
}

void FindResultsModule::FindInDocument(std::shared_ptr<FindFilter> filter)
{
    using namespace DAVA;

    ContextAccessor* accessor = GetAccessor();
    DataContext* activeContext = accessor->GetActiveContext();

    if (activeContext != nullptr)
    {
        ProjectData* projectData = activeContext->GetData<ProjectData>();
        DocumentData* documentData = activeContext->GetData<DocumentData>();

        findResultsWidget->Find(filter, projectData, documentData);
        GetAccessor()->GetGlobalContext()->GetData<FindResultsModuleDetail::DockPanelData>()->isActive = true;
    }
}

void FindResultsModule::OnDataChanged(const DAVA::DataWrapper& wrapper, const DAVA::Vector<DAVA::Any>& fields)
{
    findResultsWidget->StopFind();
    findResultsWidget->ClearResults();
}

DECL_TARC_MODULE(FindResultsModule);
