#include "Modules/LegacySupportModule/LegacySupportModule.h"
#include "Modules/DocumentsModule/DocumentData.h"
#include "Modules/ProjectModule/ProjectData.h"
#include "Modules/LegacySupportModule/Private/Project.h"

#include "Model/PackageHierarchy/PackageNode.h"

#include "Classes/Application/SettingsConverter.h"
#include "Classes/Application/QEGlobal.h"

#include "UI/mainwindow.h"
#include "UI/ProjectView.h"
#include "UI/Find/Filters/PrototypeUsagesFilter.h"
#include "UI/Preview/PreviewWidgetSettings.h"

#include <TArc/Core/ContextAccessor.h>
#include <TArc/DataProcessing/Common.h>
#include <TArc/Qt/QtIcon.h>
#include <TArc/WindowSubSystem/ActionUtils.h>
#include <TArc/WindowSubSystem/QtAction.h>

#include <Version/Version.h>

DAVA_VIRTUAL_REFLECTION_IMPL(LegacySupportModule)
{
    DAVA::ReflectionRegistrator<LegacySupportModule>::Begin()
    .ConstructorByPointer()
    .End();
}

void LegacySupportModule::PostInit()
{
    using namespace DAVA;

    ContextAccessor* accessor = GetAccessor();
    ConvertSettingsIfNeeded(accessor->GetPropertiesHolder(), accessor);

    projectDataWrapper = accessor->CreateWrapper(DAVA::ReflectedTypeDB::Get<ProjectData>());
    projectDataWrapper.SetListener(this);

    InitMainWindow();
}

void LegacySupportModule::OnWindowClosed(const DAVA::WindowKey& key)
{
    using namespace DAVA;
    ContextAccessor* accessor = GetAccessor();
    DataContext* globalData = accessor->GetGlobalContext();
    projectDataWrapper.SetListener(nullptr);

    //this code is writed to support legacy work with Project
    //when we removing ProjectData inside OnWindowClose we dont receive OnDataChanged
    project = nullptr;
}

void LegacySupportModule::OnDataChanged(const DAVA::DataWrapper& wrapper, const DAVA::Vector<DAVA::Any>& fields)
{
    using namespace DAVA;

    ContextAccessor* accessor = GetAccessor();
    DataContext* globalContext = accessor->GetGlobalContext();
    QWidget* window = GetUI()->GetWindow(DAVA::mainWindowKey);
    MainWindow* mainWindow = qobject_cast<MainWindow*>(window);
    DVASSERT(mainWindow != nullptr);
    MainWindow::ProjectView* projectView = mainWindow->GetProjectView();

    if (wrapper == projectDataWrapper)
    {
        project = nullptr;
        auto found = std::find(fields.begin(), fields.end(), ProjectData::projectPathPropertyName);
        if (found != fields.end() || wrapper.HasData())
        {
            project.reset(new Project(projectView, GetAccessor()));
        }
    }
}

void LegacySupportModule::InitMainWindow()
{
    using namespace DAVA;

    MainWindow* mainWindow = new MainWindow(GetAccessor(), GetUI(), GetInvoker());

    String title = Version::CreateAppVersion("QuickEd");
    mainWindow->SetEditorTitle(QString::fromStdString(title));

    GetUI()->InjectWindow(DAVA::mainWindowKey, mainWindow);

    QString toolbarName = "Main Toolbar";
    ActionPlacementInfo toolbarTogglePlacement(CreateMenuPoint(QList<QString>() << "View"
                                                                                << "Toolbars"));
    GetUI()->DeclareToolbar(DAVA::mainWindowKey, toolbarTogglePlacement, toolbarName);
}
