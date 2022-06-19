#include "Modules/FileSystemModule/FileSystemModule.h"
#include "Modules/FileSystemModule/FileSystemWidget.h"
#include "Modules/ProjectModule/ProjectData.h"
#include "Application/QEGlobal.h"

#include <TArc/WindowSubSystem/UI.h>
#include <TArc/WindowSubSystem/QtAction.h>
#include <TArc/WindowSubSystem/ActionUtils.h>
#include <TArc/Utils/ModuleCollection.h>

#include <QTreeView>

DAVA_VIRTUAL_REFLECTION_IMPL(FileSystemModule)
{
    DAVA::ReflectionRegistrator<FileSystemModule>::Begin()
    .ConstructorByPointer()
    .End();
}

void FileSystemModule::PostInit()
{
    InitUI();
    RegisterOperations();
    CreateActions();
}

void FileSystemModule::InitUI()
{
    using namespace DAVA;

    const char* title = "File System";
    DockPanelInfo panelInfo;
    panelInfo.title = title;
    panelInfo.area = Qt::LeftDockWidgetArea;
    PanelKey panelKey(title, panelInfo);

    widget = new FileSystemWidget(GetAccessor());
    widget->openFile.Connect(this, &FileSystemModule::OnOpenFile);
    GetUI()->AddView(DAVA::mainWindowKey, panelKey, widget);
}

void FileSystemModule::RegisterOperations()
{
    RegisterOperation(QEGlobal::SelectFile.ID, widget, &FileSystemWidget::SelectFile);
}

void FileSystemModule::CreateActions()
{
    using namespace DAVA;

    {
        QtAction* action = new QtAction(GetAccessor(), QObject::tr("Collapse all"), widget);

        KeyBindableActionInfo info;
        info.blockName = "File System";
        info.context = Qt::WidgetWithChildrenShortcut;
        MakeActionKeyBindable(action, info);

        QTreeView* treeView = widget->GetTreeView();
        QObject::connect(action, &QAction::triggered, treeView, &QTreeView::collapseAll);

        FieldDescriptor fieldDescr;
        fieldDescr.type = ReflectedTypeDB::Get<ProjectData>();
        fieldDescr.fieldName = FastName(ProjectData::projectPathPropertyName);
        action->SetStateUpdationFunction(QtAction::Enabled, fieldDescr, [](const Any& fieldValue) -> Any {
            return fieldValue.Cast<FilePath>(FilePath()).IsEmpty() == false;
        });
        action->SetStateUpdationFunction(QtAction::Visible, fieldDescr, [](const Any& fieldValue) -> Any {
            return fieldValue.Cast<FilePath>(FilePath()).IsEmpty() == false;
        });
        treeView->addAction(action);
        GetUI()->AddAction(DAVA::mainWindowKey, ActionPlacementInfo(CreateInvisiblePoint()), action);
    }
}

void FileSystemModule::OnOpenFile(const QString& filePath)
{
    InvokeOperation(QEGlobal::OpenDocumentByPath.ID, filePath);
}

DECL_TARC_MODULE(FileSystemModule);
